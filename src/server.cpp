/*
 *  SOURCE FILE:
 *  server.cpp
 *  --
 *  PROGRAM: 4981_ass3
 *  --
 *  FUNCTIONS:
 *  void server();
 *  void listenForPackets();
 *  void listenTCP(int Socket, unsigned long ip, unsigned short port);
 *  int createSocket(bool nonblocking);
 *  void makeNonBlock(int socket);
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
 *  NOTES:
 *  This file handles server-specific calls, as well as a shared method used for handling the receiving of packets
 */
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
#include <string>
#include <iostream>
#include <thread>
#include <algorithm>
#include <map>

#include "headers/server.h"
#include "headers/main.h"

char buffer[MAXPACKETSIZE];
std::map<int,std::string> socketList;

/*
 *  FUNCTION:
 *  server
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
 *  void server();
 *  --
 *  RETURNS:
 *  void
 *  --
 *  NOTES:
 *  Main server method called from main. It initializes server socket and starts listening for packets
 */
void server(){
    Socket = createSocket(true);
    memset(buffer, 0, MAXPACKETSIZE);

    listenTCP(Socket, INADDR_ANY, LISTEN_PORT_TCP);
    listenForPackets();
}

/*
 *  FUNCTION:
 *  listenForPackets
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
 *  void listenForPackets();
 *  --
 *  RETURNS:
 *  void
 *  --
 *  NOTES:
 *  Listens for incoming packets on the server socket. It does this through a combination of epoll for event-driven reads
 *  and OpenMP handling simultaneous processing of clients. New clients are added to a client list, while existing clients have their messages sent
 *  off for processing.
 */
void listenForPackets() {
    int epollfd;
    epoll_event ev;
    epoll_event *events;

    if (!(events = (epoll_event *) calloc(MAXEVENTS, sizeof(epoll_event)))) {
        perror("Calloc failure");
        exit(2);
    }

    if ((epollfd = epoll_create1(0)) == -1) {
        perror("Epoll create");
        exit(3);
    }

    ev.events = EPOLLIN | EPOLLET | EPOLLEXCLUSIVE;
    ev.data.fd = Socket;

    if ((epoll_ctl(epollfd, EPOLL_CTL_ADD, Socket, &ev)) == -1) {
        perror("epoll_ctl");
        exit(4);
    }

    ssize_t nbytes = 0;
    int nevents = 0;

    for (;;) {
        if ((nevents = epoll_wait(epollfd, events, MAXEVENTS, -1)) == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            }
            perror("epoll_wait");
            exit(5);
        }

#pragma omp parallel for schedule (static)
        for (int i = 0; i < nevents; ++i) {
            if (events[i].events & EPOLLERR) {
                if (mode) {
                    //Handle error on client side
                    ssize_t errorVal;
                    socklen_t errorSize = sizeof(ssize_t);
                    
                    if (getsockopt(events[i].data.fd, SOL_SOCKET, SO_ERROR, &errorVal, &errorSize) < 0) {
                        perror("GetSockOpt");
                        close(events[i].data.fd);
                        continue;
                    }
                    if (errorVal != 0) {
                        //Connect failed
                        perror("Connection failure");
                        close(events[i].data.fd);
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
                    exit(6);
                } else {
                    socketList.erase(events[i].data.fd);
                }
                close(events[i].data.fd);
                continue;
            }
            if (events[i].events & EPOLLIN) {
                if (!mode && events[i].data.fd == Socket) {
                    epoll_event newEv;
                    memset(&newEv, 0, sizeof(epoll_event));
                    newEv.events = EPOLLIN | EPOLLET | EPOLLEXCLUSIVE;

                    int clientSock = accept(Socket, nullptr, nullptr);
                    if (clientSock == -1) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            continue;
                        }
                        perror("Accept");
                        continue;
                    }

                    makeNonBlock(clientSock);

                    newEv.data.fd = clientSock;

                    if ((epoll_ctl(epollfd, EPOLL_CTL_ADD, clientSock, &newEv)) == -1) {
                        perror("epoll_ctl");
                        exit(7);
                    }
                    //Save address of new client
                    if(socketList.find(clientSock) == socketList.end()){
                        socketList[clientSock] = "No Name";
                    }
                } else {
                    while ((nbytes = recv(events[i].data.fd, buffer, MAXPACKETSIZE, 0)) > 0) {
#pragma omp task
                        {
                            buffer[nbytes] = '\0';
                            if(mode && ui){
                                if(buffer[0] == 'u'){
                                    ui->addUser(buffer+1);
                                } else if(buffer[0] == 'm'){
                                    ui->addMsg(buffer+1);
                                }
                            } else {
                                printf("read %d from %d\n", static_cast<int>(nbytes), static_cast<int>(events[i].data.fd));
                                if(buffer[0] == 'u'){
                                    socketList[events[i].data.fd] = std::string(buffer+1);
                                    for(const auto& fd : socketList){
                                        if(fd.first != events[i].data.fd){
                                            send(events[i].data.fd, ("u" + fd.second).c_str(), fd.second.size()+1, 0);
                                        }
                                    }
                                }
                                for(const auto& fd : socketList) {
                                    if(fd.first != events[i].data.fd){
                                        send(fd.first, buffer, nbytes, 0);
                                    }
                                }
                            }
                        }
                    }
                    if (nbytes == -1) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            continue;
                        }
                        perror("Packet read failure");
                        exit(8);
                    }
                }
            }
        }
    }
    free(events);
}

/*
 *  FUNCTION:
 *  listenTCP
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
 *  void listenTCP(int Socket, unsigned long ip, unsigned short port);
 *  --
 *  ARGS:
 *  int Socket - The socket to bind and set to listen mode
 *  unsigned long ip - The ip address in network byte order to bind to
 *  unsigned short port - The prot number in network byte order to bind to
 *  --
 *  RETURNS:
 *  void
 *  --
 *  NOTES:
 *  Binds the socket and sets it to listen on the specified address and port
 */
void listenTCP(int Socket, unsigned long ip, unsigned short port) {
    //TCP Setup
    struct sockaddr_in servaddrtcp;
    memset(&servaddrtcp, 0, sizeof(servaddrtcp));
    servaddrtcp.sin_family = AF_INET;
    servaddrtcp.sin_addr.s_addr = htonl(ip);
    servaddrtcp.sin_port = htons(port); 

    if ((bind(Socket, (struct sockaddr *) &servaddrtcp, sizeof(servaddrtcp))) == -1) {
        perror("Bind TCP");
        exit(9);
    }

    if (listen(Socket, LISTENQ) == -1) {
        perror("Listen TCP");
        exit(10);
    }
}

/*
 *  FUNCTION:
 *  createSocket
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
 *  int createSocket(bool nonblocking);
 *  --
 *  ARGS:
 *  bool nonblocking - Whether the socket being create is non-blocking
 *  --
 *  RETURNS:
 *  int - The descriptor for the socket that was created
 *  --
 *  NOTES:
 *  Creates the socket and sets it to non-blocking mode if specified
 */
int createSocket(bool nonblocking) {
    int sock = socket(AF_INET, SOCK_STREAM | (nonblocking * SOCK_NONBLOCK), 0);
    if (sock == -1) {
        perror("Create Socket");
        exit(11);
    }
    int enable = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
        exit(12);
    }
    return sock;
}

/*
 *  FUNCTION:
 *  makeNonBlock
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
 *  void makeNonBlock(int socket);
 *  --
 *  ARGS:
 *  int socket - The socket descriptor to change
 *  --
 *  RETURNS:
 *  void
 *  --
 *  NOTES:
 *  Sets an existing socket to nonblocking mode
 */
void makeNonBlock(int socket){
    if(fcntl(socket, F_SETFL, O_NONBLOCK) == -1){
        perror("fctl nonblock");
        exit(13);
    }
}
