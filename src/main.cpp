#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>

#include "headers/main.h"
#include "headers/server.h"
#include "headers/client.h"
#include "headers/ui.h"


int Socket = 0;
int port = 54123;
int mode = 1;

int main(int argc, char *argv[]){
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
    //throw these in threads?
    UI ui;
    for(int i = 0; i < 20; ++i){
        ui.addUser("1 asdf");
        ui.addUser("qwer 2");
        ui.addUser("qwer nhf 3");
    }
    
    for(int i = 0; i < 20; ++i){
        ui.addMsg("This is a message");
    }

    ui.updateOnlineItems();
    ui.updateMessages();
    ui.update();
    ui.loop();
    return 0;
    
    if(mode){
        client();
    } else {
        server();
    }
    close(Socket);
}
