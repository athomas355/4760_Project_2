#include "bin_addr.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#define DEBUG 1
using namespace std;

struct cli_struct{
    bool s, t;
    int x, time;
    char* datafile;
};

cli_struct parse_args(int argc,char* argv[]);

int main(int argc, char* argv[]) {
    
    cli_struct args = parse_args(argc, argv);
#if DEBUG 
    printf("s = %d, t = %d, x = %d, time = %d, datafile = %s\n", args.s, args.t, args.x, args.time, args.datafile);
#endif

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
            case 'h':
                printf("master [-h] [-s x] [-t time] datafile\n");
                exit(0);

            case 's':
                cli_args.s = true;
                if(optarg != 0) {
                    printf("%s\n", optarg);
                }
                cli_args.x = atoi(optarg); 

                break;

            case 't':
                cli_args.t = true;
                cli_args.time = atoi(optarg);
                break;

            case '?':
                printf("unknown option: %s\n", optarg);
                exit(-1);
        }
    }

    for(; optind < argc; optind++) {
        cli_args.datafile = argv[optind];
        printf("Extra arguments: %s\n", argv[optind]);
    }

    return cli_args;
}