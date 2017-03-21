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
#include <getopt.h>
#include <cstdlib>
#include <climits>
#include <unistd.h>
#include <thread>

#include "headers/main.h"
#include "headers/server.h"
#include "headers/client.h"
#include "headers/ui.h"

int Socket = 0;
int port = 54123;
int mode = 1;

UI *ui = nullptr;

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
    int opt;

    setenv("OMP_PROC_BIND", "TRUE", 1);
    setenv("OMP_DYNAMIC", "TRUE", 1);

    while((opt = getopt(argc, argv, "csp:")) != -1){
        switch(opt){
            case 's'://server
                mode = 0;
                break;
            case 'c'://client
                mode = 1;
                break;
            case 'p'://port
                if(!(port = atoi(optarg)) || port < 0 || port > USHRT_MAX) {
                    printf("port must be a positive integer\n");
                    return 1;
                }
                break;
        }
    }

    if(mode){
        UI uim;
        ui = &uim;

        uim.updateOnlineItems();
        uim.updateMessages();
        
        std::string host = uim.loopGetHost();
        if(!host.size())
            return 1;
        uim.loopGetName();
        std::thread uiWorker([&]{
                uim.loop();
                });

        client(host.c_str());
        uiWorker.join();
    } else {
        server();
    }
    close(Socket);
}
