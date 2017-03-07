#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <limits.h>
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

    if(mode){
        ui = new UI;
        //dummy data
        /*
        for(int i = 0; i < 30; ++i){
            ui->addUser("A User");
            ui->addUser("Another User");
            ui->addUser("Wow Users");
        }

        for(int i = 0; i < 20; ++i){
            ui->addMsg("This is a message");
        }
        */
        ui->updateOnlineItems();
        ui->updateMessages();
        

        std::thread uiWorker([&]{
                ui->loop();
                delete ui;
                ui = 0;
                });

        client();
        //ensure it closed
        if(ui != nullptr)
            ui->close();
        //wait for that to actually happen
        uiWorker.join();
    } else {
        server();
    }
    close(Socket);
}
