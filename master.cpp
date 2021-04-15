#include "bin_addr.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <io.h>
#include <iostream>
using namespace std;

struct cli_struct{
    bool s, t;
    int x, time;
    string datafile;
};

cli_struct parse_args(int argc,char* argv[]);

int main(int argc, char* argv[]) {
    
    parse_args(argc, argv);
    return 0;
}

cli_struct parse_args(int argc,char* argv[]) {
    cli_struct cli_args;
    int option;
    cli_args.x = 20;
    cli_args.time = 100;

    while((option = getopt(argc, argv, "hst")) != -1) {
        switch(option) {
            case 'h':
                printf("master [-h] [-s x] [-t time] datafile");
                exit(0);

            case 's':
                cli_args.s = true;
                cli_args.x = ; 
                break;

            case 't':
                cli_args.t = true;
                cli_args.time = ;
                break;
        }
    }
}