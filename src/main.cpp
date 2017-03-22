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
 *  Isaac Morneau
 *  --
 *  PROGRAMMER:
 *  Isaac Morneau
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
 *  
 */
int main(int argc, char **argv) {
    int opt;
    bool isClient = true;
    //set openmp variables for even better performance
    setenv("OMP_PROC_BIND", "TRUE", 1);
    setenv("OMP_DYNAMIC", "TRUE", 1);

    while((opt = getopt(argc, argv, "cs")) != -1){
        switch(opt){
            case 's'://server
                isClient = false;
                break;
            case 'c'://client
            default:
                isClient = true;
                break;
        }
    }
    if (isClient) {
        cout << "[Client]" << endl;
        client();
        cout << "[Done]" << endl;
    } else {
        cout << "[Server]" << endl;
        server();
        cout << "[Done]" << endl;
    }
}
