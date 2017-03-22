#include "headers/networkhelpers.h"
#include "headers/client.h"
#include "headers/server.h"

#include <unistd.h>
#include <omp.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <map>

char buffer[MAXBUFFER];
int socketfd = 0;

int createNonblock() {
    int sock;
    const int on = 1;
    if((sock = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) == -1) {
        perror("Create Socket");
        exit(1);
    }
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
        exit(2);
    }
#ifdef SO_REUSEPORT
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(int)) < 0) { 
        perror("setsockopt(SO_REUSEPORT) failed");
        exit(3);
    }
#endif
    return sock;
}

void makeNonblock(int sock) {
    if(fcntl(sock,F_SETFL, O_NONBLOCK) == -1){
        perror("fcntl(O_NONBLOCK) failed");
        close(sock);
        exit(4);
    }
}

void listenForPackets(bool isClient, const char *host) {
    int epollfd;
    epoll_event evl;
    epoll_event *events;

    ssize_t nbytes = 0;
    int nevents = 0;
    memset(buffer, 0, MAXBUFFER);

    if (!(events = (epoll_event *) calloc(MAXEVENTS, sizeof(epoll_event)))) {
        perror("calloc failure");
        exit(1);
    }

    if ((epollfd = epoll_create1(0)) == -1) {
        perror("epoll create");
        exit(2);
    }

    evl.events = EPOLLIN | EPOLLET | EPOLLEXCLUSIVE;
    evl.data.fd = createNonblock();
    socketfd = evl.data.fd;
    if(isClient){
        connectSock(evl.data.fd, host);
    } else {
        listenSock(evl.data.fd);
    }

    if ((epoll_ctl(epollfd, EPOLL_CTL_ADD, evl.data.fd, &evl)) == -1) {
        perror("epoll_ctl");
        exit(3);
    }

    for (;;) {
        if ((nevents = epoll_wait(epollfd, events, MAXEVENTS, -1)) == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            }
            perror("epoll_wait");
            exit(4);
        }

#pragma omp parallel for schedule (static)
        for (int i = 0; i < nevents; ++i) {
            //error on a socket
            if (events[i].events & EPOLLERR) {
                ssize_t errorVal;
                socklen_t errorSize = sizeof(ssize_t);

                if (getsockopt(events[i].data.fd, SOL_SOCKET, SO_ERROR, &errorVal, &errorSize) < 0) {
                    perror("getsockopt");
                    close(events[i].data.fd);
                    if(isClient)
                        closeClient(events[i].data.fd);
                    else
                        closeServer(events[i].data.fd);
                    continue;
                }

                if (errorVal != 0) {
                    perror("connection failure");
                    close(events[i].data.fd);
                    if(isClient)
                        closeClient(events[i].data.fd);
                    else
                        closeServer(events[i].data.fd);
                    continue;
                }
            //socket disconnected
            } else if (events[i].events & EPOLLHUP) {
                close(events[i].data.fd);
                if(isClient)
                    closeClient(events[i].data.fd);
                else
                    closeServer(events[i].data.fd);
                continue;
            //socke sent info
            } else if (events[i].events & EPOLLIN) {
                //info is a connection request
                if (!isClient && events[i].data.fd == evl.data.fd) {
                    epoll_event evt;
                    evt.events = EPOLLIN | EPOLLET | EPOLLEXCLUSIVE;
                    evt.data.fd = accept(events[i].data.fd, 0, 0);

                    makeNonblock(evt.data.fd);

                    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, evt.data.fd, &evt) == -1){
                        perror("epoll_ctl(EPOLL_CTL_ADD) failure");
                        exit(5);
                    }
                //info is a message
                } else {
                    while ((nbytes = recv(events[i].data.fd, buffer, MAXBUFFER, 0)) > 0) {
#pragma omp task
                        {
                            buffer[nbytes] = '\0';
                            if(isClient)
                                recvClient(events[i].data.fd, buffer, nbytes);
                            else
                                recvServer(events[i].data.fd, buffer, nbytes);
                        }
                    }
                    if (nbytes == -1) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            continue;
                        }
                        perror("Packet read failure");
                        exit(8);
                    } else if (nbytes == 0) { // connection closed by peer during read
                        if(isClient)
                            closeClient(events[i].data.fd);
                        else
                            closeServer(events[i].data.fd);
                    }
                }
            }
        }
    }
    free(events);
}

