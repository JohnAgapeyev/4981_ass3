/*
 *  SOURCE FILE:
 *  main.cpp - A chat application using epoll and OpenMP
 *  --
 *  PROGRAM: 4981_ass3
 *  --
 *  FUNCTIONS:
 *  int main(int argc, char **argv);
 *  --
 *  DATE:
 *  March 20, 2017
 *  --
 *  DESIGNER:
 *  John Agapeyev
 *  Isaac Morneau
 *  --
 *  PROGRAMMER:
 *  Isaac Morneau
 */
#include <cstdio>
#include <iostream>
#include <getopt.h>
#include <cstdlib>
#include <climits>
#include <unistd.h>

#include "headers/server.h"
#include "headers/client.h"

using namespace std;
/*
 *  FUNCTION:
 *  main
 *  --
 *  DATE:
 *  March 20, 2017
 *  --
 *  DESIGNER:
 *  John Agapeyev
 *  --
 *  PROGRAMMER:
 *  John Agapeyev
 *  --
 *  INTERFACE:
 *  int main(int argc, char **argv);
 *  --
 *  ARGS:
 *  int argc - The number of commmand line args
 *  char **argv - The list of command args
 *  --
 *  RETURNS:
 *  int
 *  --
 *  NOTES:
 *  Main entry function
 */
int main(int argc, char **argv) {
    int opt, mode = 1;
    //set openmp variables for even better performance
    setenv("OMP_PROC_BIND", "TRUE", 1);
    setenv("OMP_DYNAMIC", "TRUE", 1);

    while((opt = getopt(argc, argv, "cs")) != -1){
        switch(opt){
            case 's'://server
                mode = 0;
                break;
            case 'c'://client
                mode = 1;
                break;
            default:
                break;
        }
    }

    if(mode){
        string host;
        cout << "Please enter hostname: " << endl;
        cin >> host;
        client(host.c_str());
    } else {
        server();
    }
}
