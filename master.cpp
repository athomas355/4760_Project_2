#include "bin_addr.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <fstream>
#include <cmath>
#include <limits.h>
#include <assert.h>
#include <iostream>

#define DEBUG 1
#define SHM_KEY 0x1234
#define ARR_SIZE 1024
using namespace std;

//command argument struct
struct cli_struct{
    bool s, t;
    int x, time;
    char* datafile;
};

//https://www.tutorialspoint.com/inter_process_communication/inter_process_communication_shared_memory.htm 
//shared memory struct
struct shmSegment {
    int intArr[ARR_SIZE];
    int stage;
    int memoryIndex;    //holds index of shared memory; based on what stage we are in it get incremented differently; stage x = incremented 2^stage x
    //if actualSize = 8, then memoryIndex = 0, 2, 4, 6; if actualSize = even number, then memoryIndex = every even number until 1  even number below the actualSize (comment: I don't know if this is right)
};

//fucntions declarations
cli_struct parse_args(int argc,char* argv[]);
int parentAction();
int childAction();
int readDatafile(int *arr, int *size, char* filename);
static unsigned int mylog2 (unsigned int val);
static unsigned int ipow(unsigned int val, unsigned int exp);

//main function
int main(int argc, char* argv[]) {
    
    cli_struct args = parse_args(argc, argv);   //catching the parsed args
    struct shmSegment *shmPtr;
    int actualSize = ARR_SIZE;
    int sum;
    int pidCounter = 0;
    int pidArr[1024];

    //allocate shared memory shmget()
    int shmID = shmget(SHM_KEY, sizeof(struct shmSegment), 0644|IPC_CREAT);
    if (shmID == -1) {  //error check to see if shared memeory was allocated correctly
      perror("Shared memory\n");
      exit(-1);
    }

    //use shared memory; put inputs from datafile into shared memory shmat()
    shmPtr = (struct shmSegment *)shmat(shmID, NULL, 0);            //if the second argument of shmat() is NULL, then the system picks the address; third argument is shmflg
    if (shmPtr == (void *) -1) {
      perror("Shared memory attach\n");
      exit(-1);
    }

    readDatafile(shmPtr->intArr, &actualSize, args.datafile);
    if(actualSize > 0) {    //readDatafile worked properly
        printf("readDatafile worked!\n");
        printf("actualSize = %d\n", actualSize);

        //prints the shared memory array
        for(int i = 0; i < actualSize; i++) {
            printf("  %d  ", shmPtr->intArr[i]);
        }
        printf("\n");
    }
    else {  //readDatafile failed
        printf("readDatafile didn't work!\n");
        exit(-1);
    }
    
    int actualSizeLog2 = mylog2(actualSize);
    

    //offset = stage 1 = 1, stage 2 = 2, stage 3 = 4, offset = 2 ^ (stage - 1)
    for(int stage = 1; stage <= actualSizeLog2; stage++) { //for loop for looping through each stage of processes calculation
        int offset = ipow(2, (stage - 1));
        for(int memoryIndex = 0; memoryIndex < actualSize; memoryIndex += 2 * offset) {             //for loop for iterating through the shared memory
            pid_t pid = fork();
            if(pid == -1) {
                printf("Fork call failed\n");
                exit(-1);
            }
            else if(pid == 0) {
                //("This process is the child %d\n", childAction());        
                //offset will be 1 when stage is 1, and 2 when stage = 2, and 4 when stage = 3
                //assert(stage != 3 && offset != 4);
                sum = shmPtr->intArr[memoryIndex] + shmPtr->intArr[memoryIndex + offset];
                shmPtr->intArr[memoryIndex] = sum;
                shmPtr->intArr[memoryIndex + offset] = '\0';

                printf("stage = %d, memory index = %d, offset = %d\n", stage, memoryIndex, offset);

                // //prints the shared memory array
                // for(int i = 0; i < actualSize; i++) {
                //     printf("  %d  ", shmPtr->intArr[i]);
                // }
                // printf("\n");

                //printf("Exiting from child\n");
                exit(0);
            }
            else {
                printf("This is the parent process and the child process is %d\n", pid);
                pidArr[pidCounter++] = pid;
            }//did the C Beautifier do a good job of beautifying this
        }
    }

    //https://stackoverflow.com/questions/19461744/how-to-make-parent-wait-for-all-child-processes-to-finish
    while (wait(NULL) > 0);

    //prints the shared memory array
    for(int i = 0; i < actualSize; i++) {
        printf("  %d  ", shmPtr->intArr[i]);
    }
    printf("\n");

/*pseduocode:
    #ofProcesses = actualSize/2
    loop log2(actualSize)
        create #ofProcesses


    loop log2(actualSize)
        wait a child        //we need to know the pids of all the children



    
*/
    //deallocate shared memory shmdt()
    int shmdt(shmID);

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
    This readDatafiletion fills the array of integers
    On input *size is how big the array can be
    On output *size is how big the array actually is

    Things that can go wrong:
        -array isn't big enough
        -there could be a problem opening up the file
        -there could be a problem converting the string to an integer
*/
int readDatafile(int *arr, int *size, char* filename) {

    //declare variables
    string line;
    //opens the file
    ifstream myfile(filename);

    //counts how many lines are in the file; it puts the number of lines *size
    int counter = 0;

    while (getline(myfile, line)) {
        counter++;
    }

    //rewinds the file
    //rewind();take out later
    myfile.clear();
    myfile.seekg(0);

    //reads integers from file into the array; it reads the integers into *arr
    for (int i = 0; i < counter; i++) {
        getline(myfile, line);
        arr[i] = stoi(line);
    }

    *size = counter;

    //closes the file
    myfile.close();

    return 0;
}

//log2 function
static unsigned int mylog2 (unsigned int val) {
    if (val == 0) return UINT_MAX;
    if (val == 1) return 0;
    unsigned int ret = 0;
    while (val > 1) {
        val >>= 1;
        ret++;
    }
    return ret;
}

//power function that return integer
static unsigned int ipow(unsigned int val, unsigned int exp) {
    int total = 1;
    
    for(int i = 0; i < exp; i++) {
        total = total * val;    
    }

    return total;
}

/* Test Cases:
    -datafile has 2^x positive integers
    -datafile has a different number other than 2^x positive integers
    -datafile has some negative integers
    -datafile not found or readable
    -what happens if the fork call fails
    -all command line parse errors
    -when there are more than 20 children
    *-Figure out how to make child processes wait
    -Can we automate this somehow
    
    
    *Jeff is going to find out a better way to calculate the logarithm base 2 when the argument is an integer by counting bits
    *Ash comment in shared memory struct have to find out if it is correct
    *Ash Find out how to store the pids of the children processes so the parent process know when to wait 
    *Jeff how to abort the program if it takes too long

*/