#include <omp.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <cstdarg>
#include <unordered_set>
#include <string>
#include <iostream>
#include <thread>
#include <algorithm>

#include "headers/server.h"
#include "headers/main.h"

char buffer[MAXPACKETSIZE];
std::unordered_set<int> socketList;

void server(){
    Socket = createSocket(true);
    listenTCP(Socket, INADDR_ANY, LISTEN_PORT_TCP);
    listenForPackets();
}


void listenForPackets() {
    int epollfd;
    epoll_event ev;
    epoll_event *events;

    if (!(events = (epoll_event *) calloc(MAXEVENTS, sizeof(epoll_event)))) {
        perror("Calloc failure");
        exit(1);
    }

    if ((epollfd = epoll_create1(0)) == -1) {
        perror("Epoll create");
        exit(1);
    }

    ev.events = EPOLLIN | EPOLLET | EPOLLEXCLUSIVE;
    ev.data.fd = Socket;

    if ((epoll_ctl(epollfd, EPOLL_CTL_ADD, Socket, &ev)) == -1) {
        perror("epoll_ctl");
        exit(1);
    }

    char buff[MAXPACKETSIZE];
    ssize_t nbytes = 0;
    int nevents = 0;

    for (bool running = true;running;) {
        if ((nevents = epoll_wait(epollfd, events, MAXEVENTS, -1)) == -1) {
            perror("epoll_wait");
            exit(1);
        }

#pragma omp parallel for schedule (static)
        for (int i = 0; i < nevents; ++i) {
            if (events[i].events & EPOLLERR) {
                perror("Socket error1");

                if (mode) {
                    //Handle error on client side
                    ssize_t errorVal;
                    socklen_t errorSize = sizeof(ssize_t);
                    
                    if (getsockopt(events[i].data.fd, SOL_SOCKET, SO_ERROR, &errorVal, &errorSize) < 0) {
                        perror("GetSockOpt");
                        close(events[i].data.fd);
                        running = false;
                        continue;
                    }
                    if (errorVal != 0) {
                        //Connect failed
                        perror("Connection failure");
                        close(events[i].data.fd);
                        running = false;
                        continue;
                    }
                } else {
                    socketList.erase(events[i].data.fd);
                }
                close(events[i].data.fd);
                continue;
            }
            if (events[i].events & EPOLLHUP) {
                //Peer closed the connection
                if (mode) {
                    //Handle disconnect on client side
                    running = false;
                } else {
                    socketList.erase(events[i].data.fd);
                }
                close(events[i].data.fd);
                continue;
            }
            if (events[i].events & EPOLLIN) {
                if (!mode && events[i].data.fd == Socket) {
                    sockaddr addr;
                    socklen_t addrLen;

                    ev.data.fd = accept(Socket, &addr, &addrLen);

                    if ((epoll_ctl(epollfd, EPOLL_CTL_ADD, ev.data.fd, &ev)) == -1) {
                        perror("epoll_ctl");
                        exit(1);
                    }

                    //Save address of new client
                    socketList.insert(ev.data.fd);
                } else {
                    while ((nbytes = recv(events[i].data.fd, buff, MAXPACKETSIZE, 0)) > 0) {
#pragma omp task
                        {
                            if(mode){
                                buff[nbytes] = '\0';
                                if(ui != nullptr)
                                    ui->addMsg(buff);
                            } else {
                                for(const auto fd : socketList) {
                                    if(fd != events[i].data.fd)
                                        send(fd, buff, nbytes, 0);
                                }
                            }
                        }
                    }
                    if (nbytes == -1) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            continue;
                        }
                        perror("Packet read failure");
                        exit(1);
                    }
                }
            }
        }
    }
    free(events);
}

void listenTCP(int Socket, unsigned long ip, unsigned short port) {
    //TCP Setup
    struct sockaddr_in servaddrtcp;
    memset(&servaddrtcp, 0, sizeof(servaddrtcp));
    servaddrtcp.sin_family = AF_INET;
    servaddrtcp.sin_addr.s_addr = htonl(ip);
    servaddrtcp.sin_port = htons(port); 

    if ((bind(Socket, (struct sockaddr *) &servaddrtcp, sizeof(servaddrtcp))) == -1) {
        perror("Bind TCP");
        exit(1);
    }

    if (listen(Socket, LISTENQ) == -1) {
        perror("Listen TCP");
        exit(1);
    }
}

int createSocket(bool nonblocking) {
    int sock = socket(AF_INET, SOCK_STREAM | (nonblocking * SOCK_NONBLOCK), 0);
    if (sock == -1) {
        perror("Create Socket");
        exit(1);
    }
    int enable = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
        exit(1);
    }
    return sock;
}
