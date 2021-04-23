#include "bin_addr.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fstream>
#include <iostream>

#define DEBUG 1
#define SHM_KEY 0x1234
#define ARR_SIZE 1024
using namespace std;

struct cli_struct{
    bool s, t;
    int x, time;
    char* datafile;
};

//https://www.tutorialspoint.com/inter_process_communication/inter_process_communication_shared_memory.htm 
struct shmSegment {
    int counter;
    int complete;
    int intArr[ARR_SIZE];
};

cli_struct parse_args(int argc,char* argv[]);
int parentAction();
int childAction();
int func(int *arr, int *size, char* filename);

int main(int argc, char* argv[]) {
    
    cli_struct args = parse_args(argc, argv);   //catching the parsed args
    struct shmSegment *shmPtr;
    int actualSize = ARR_SIZE;

    //allocate shared memory shmget()
    int shmID = shmget(SHM_KEY, sizeof(struct shmSegment), 0644|IPC_CREAT);
    if (shmID == -1) {  //error check to see if shared memeory was allocated correctly
      perror("Shared memory\n");
      return 1;
    }

    //use shared memory; put inputs from datafile into shared memory shmat()
    shmPtr = (struct shmSegment *)shmat(shmID, NULL, 0);            //if the second argument of shmat() is NULL, then the system picks the address; third argument is shmflg
    if (shmPtr == (void *) -1) {
      perror("Shared memory attach\n");
      return 1;
    }

    if (!func(shmPtr->intArr, &actualSize, args.datafile)) { //func woked properly
        printf("func worked!\n");
        printf("actualSize = %d\n", actualSize);

        for(int i = 0; i < actualSize; i++) {
            printf("  %d  ", shmPtr->intArr[i]);
        }
        printf("\n");
    }
    else {  //func failed
        printf("func didn't work!\n");
        exit(-1);
    }

    //deallocate shared memory shmdt()

    pid_t pid = fork();

    if(pid == -1) {
        printf("Fork call failed\n");
        exit(-1);
    }
    else if(pid == 0) {
        printf("This process is the child %d\n", childAction());
    }
    else {
        printf("This is the parent process %d\n", parentAction());
    }

    return 0;
}

cli_struct parse_args(int argc,char* argv[]) {
    cli_struct cli_args;
    int option;
    cli_args.s = false;
    cli_args.t = false;
    cli_args.x = 20;
    cli_args.time = 100;
    //cli_args.datafile = "";

    while((option = getopt(argc, argv, "hs:t:")) != -1) {
        switch(option) {
            case 'h':   //help message
                printf("master [-h] [-s x] [-t time] datafile\n\t-s: maximum number of child processes\n\t-t: maximum allowed second before process terminates");
                exit(0);

            case 's':   //maximum number of child processes 
                cli_args.s = true;
                if(optarg != 0) {
                    printf("%s\n", optarg);
                }
                cli_args.x = atoi(optarg); 

                break;

            case 't':   //maximum allowed second before process terminates
                cli_args.t = true;
                cli_args.time = atoi(optarg);
                break;

            case '?':
                printf("unknown option: %s\n", optarg);
                exit(-1);
        }
    }

    //this protects against operator error
    for(; optind < argc; optind++) {
        cli_args.datafile = argv[optind];
        //printf("Extra arguments: %s\n", argv[optind]);  //this line was copied from somwhere else look at later
    }

    /* Haven't figured out that the number of file arguments are correct
    printf("optind = %d\targc = %d\n", optind, argc);

    if(optind > argc) {
        printf("Operator Error: Too many files\n");
        exit(-1);
    }
    if(optind == argc) {
        printf("Operator Error: No file given\n");
        exit(-1);
    }
    */

    return cli_args;
}

/*What a parent process does: 
            1. parent is going to allocate some memory
            2. sleep to verify that the child is going to stop and wait
            3. share the memory 
            4. wait for the child to complete this is for testing purposes
            5. write into memory what the child has to do
            6. get the results from the child
            7. print the answer
            8. wait for all the children to die (why we need the child PID)
            9. this parent process exits with return status 0
*/
int parentAction() {
    printf("I'm in parent action\n");

    return 0;
}

/*What a child process does:
            1. wait for the parent to allocate some memory
            2. go process whatever it is the parent gives us
            3. return it to parent 
            4. exit normally
*/
int childAction() {
    printf("I'm in child action\n");
    return 0;
}

/*
    This function fills the array of integers
    On input *size is how big the array can be
    On output *size is how big the array actually is

    Things that can go wrong:
        -array isn't big enough
        -there could be a problem opening up the file
        -there could be a problem converting the string to an integer
*/
int func(int *arr, int *size, char* filename) {

    //declare variables
    string line;
    //opens the file
    //FILE *fh = fopen(filename, "r"); take out later
    ifstream myfile(filename);

    //counts how many lines are in the file; it puts the number of lines *size
    int counter = 0;

    while (getline(myfile, line)) {
        counter++;
    }

    /* take out later
    while (!feof(fh)) {
        getline(buffer, &bufferSize, fh);
        counter++;
    }
    */
    //rewinds the file
    //rewind();take out later
    myfile.clear();
    myfile.seekg(0);

    //reads integers from file into the array; it reads the integers into *arr
    for (int i = 0; i < counter; i++) {
        //getline(buffer, &bufferSize, fh); take out later
        getline(myfile, line);
        arr[i] = stoi(line);
    }

    *size = counter;

    //closes the file
    myfile.close();

    return 0;
}