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
#include <arpa/inet.h>
#include <cstdarg>
#include <unordered_map>
#include <string>
#include <iostream>

#include "server.h"

char buffer[MAXPACKETSIZE];
int listenSocketTCP;
int sendSocketTCP;
std::unordered_map<unsigned long, std::string> clientList;

int main(int argc, char **argv) {
    if ((listenSocketTCP = createSocket(true)) == -1) {
        perror("ListenSocket TCP");
        exit(1);
    }
    if ((sendSocketTCP = createSocket(true)) == -1) {
        perror("sendSocket TCP");
        exit(1);
    }

    listenTCP(listenSocketTCP, INADDR_ANY, LISTEN_PORT_TCP);
    listenForPackets();

    return 0;
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
    ev.data.fd = listenSocketTCP;

    if ((epoll_ctl(epollfd, EPOLL_CTL_ADD, listenSocketTCP, &ev)) == -1) {
        perror("epoll_ctl");
        exit(1);
    }

    char buff[MAXPACKETSIZE];
    ssize_t nbytes = 0;
    int nevents = 0;

    for (;;) {
        if ((nevents = epoll_wait(epollfd, events, MAXEVENTS, -1)) == -1) {
            perror("epoll_wait");
            exit(1);
        }

#pragma omp parallel for
        for (int i = 0; i < nevents; ++i) {
            if (events[i].events & EPOLLERR) {
                perror("Socket error");

                sockaddr_in addr{0};
                socklen_t addrLen = 0;

                if (getpeername(events[i].data.fd, (sockaddr *) &addr, &addrLen) == -1) {
                    perror("Get Peer Name");
                    exit(1);
                }

                close(events[i].data.fd);
                clientList.erase(addr.sin_addr.s_addr);
                continue;
            }
            if (events[i].events & EPOLLHUP) {
                //Peer closed the connection
                sockaddr_in addr{0};
                socklen_t addrLen = 0;

                if (getpeername(events[i].data.fd, (sockaddr *) &addr, &addrLen) == -1) {
                    perror("Get Peer Name");
                    exit(1);
                }

                close(events[i].data.fd);
                clientList.erase(addr.sin_addr.s_addr);
                continue;
            }
            if (events[i].events & EPOLLIN) {
                if (events[i].data.fd == listenSocketTCP) {
                    sockaddr addr;
                    socklen_t addrLen;

                    ev.data.fd = accept(listenSocketTCP, &addr, &addrLen);

                    if ((epoll_ctl(epollfd, EPOLL_CTL_ADD, ev.data.fd, &ev)) == -1) {
                        perror("epoll_ctl");
                        exit(1);
                    }
                    unsigned long ip;

                    inet_pton(AF_INET, addr.sa_data, &ip);

                    //Save address of new client
                    clientList.insert({ip, addr.sa_data});
                } else {
                    while ((nbytes = recv(events[i].data.fd, buff, MAXPACKETSIZE, 0)) > 0) {
#pragma omp task
                        {
                            send(events[i].data.fd, buff, nbytes, 0);
                            processPacket(buff);
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
}

void processPacket(const char *data) {
    std::cout << "Received: " << data << std::endl;
}

void listenTCP(int socket, unsigned long ip, unsigned short port) {
    //TCP Setup
    struct sockaddr_in servaddrtcp;
    memset(&servaddrtcp, 0, sizeof(servaddrtcp));
    servaddrtcp.sin_family = AF_INET;
    servaddrtcp.sin_addr.s_addr = htonl(ip);
    servaddrtcp.sin_port = htons(port); 

    if ((bind(socket, (struct sockaddr *) &servaddrtcp, sizeof(servaddrtcp))) == -1) {
        perror("Bind TCP");
        exit(1);
    }

    if (listen(socket, LISTENQ) == -1) {
        perror("Listen TCP");
        exit(1);
    }
}

int createSocket(bool nonblocking) {
    int sock = socket(AF_INET, SOCK_STREAM | (nonblocking * SOCK_NONBLOCK), 0);
    int enable = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
        exit(1);
    }
    return sock;
}
