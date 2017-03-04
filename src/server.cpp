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
#include <unordered_map>
#include <string>
#include <iostream>
#include <thread>

#include "headers/server.h"

char buffer[MAXPACKETSIZE];
int Socket;
bool isClient;
std::unordered_map<unsigned long, std::string> clientList;

int main(int argc, char **argv) {
    setenv("OMP_PROC_BIND", "TRUE", 1);
    setenv("OMP_DYNAMIC", "TRUE", 1);

    if (argc < 2) {
        perror("Not enough arguments");
        exit(1);
    }

    //TEMPORARY - to be replaced with getlongopts when written
    if (strcmp(argv[1], "-c") == 0) {
        isClient = true;
    } else {
        isClient = false;
    }


    if (isClient) {
        Socket = createSocket(false);
        connect("127.0.0.1");    
        listenForPackets();
    } else {
        Socket = createSocket(true);
        listenTCP(Socket, INADDR_ANY, LISTEN_PORT_TCP);
        listenForPackets();
    }

    close(Socket);
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

    ev.events = EPOLLIN | EPOLLET | EPOLLEXCLUSIVE | (isClient * EPOLLOUT);
    ev.data.fd = Socket;

    if ((epoll_ctl(epollfd, EPOLL_CTL_ADD, Socket, &ev)) == -1) {
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

#pragma omp parallel for schedule (static)
        for (int i = 0; i < nevents; ++i) {
            if (events[i].events & EPOLLERR) {
                perror("Socket error1");

                if (isClient) {
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
                    sockaddr_in addr{0};
                    socklen_t addrLen = 0;

                    if (getpeername(events[i].data.fd, (sockaddr *) &addr, &addrLen) == -1) {
                        perror("Get Peer Name");
                        exit(1);
                    }

                    clientList.erase(addr.sin_addr.s_addr);
                }
                close(events[i].data.fd);
                continue;
            }
            if (events[i].events & EPOLLHUP) {
                //Peer closed the connection
                if (isClient) {
                    //Handle disconnect on client side
                } else {
                    sockaddr_in addr{0};
                    socklen_t addrLen = 0;

                    if (getpeername(events[i].data.fd, (sockaddr *) &addr, &addrLen) == -1) {
                        perror("Get Peer Name");
                        exit(1);
                    }

                    clientList.erase(addr.sin_addr.s_addr);
                }
                close(events[i].data.fd);
                continue;
            }
            if (events[i].events & EPOLLOUT) {
                if (isMessagePending()) {
                    const auto& out = getUserMessage();
                    if (send(events[i].data.fd, out.c_str(), out.size(), 0) < 0) {
                        perror("Send failure");
                        close(events[i].data.fd);
                        continue;
                    }
                }
            }
            if (events[i].events & EPOLLIN) {
                if (!isClient && events[i].data.fd == Socket) {
                    sockaddr addr;
                    socklen_t addrLen;

                    ev.data.fd = accept(Socket, &addr, &addrLen);

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
    free(events);
}

void processPacket(const char *data) {
    std::cout << "Received: " << data << std::endl;
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

void connect(const char *host) {
    hostent *server;

    if ((server = gethostbyname(host)) == nullptr) {
        //Temporary - to signal error to user
        perror("Bad host");
        exit(1);
    }

    sockaddr_in addr;
    memset(&addr, 0, sizeof(sockaddr_in));
    addr.sin_family = AF_INET;
    memmove(&addr.sin_addr.s_addr, server->h_addr, server->h_length);
    addr.sin_port = htons(LISTEN_PORT_TCP);

    if (connect(Socket, (sockaddr *) &addr, sizeof(addr)) < 0) {
        //Temporary - to signal error to user
        if (errno != EINPROGRESS && errno != EALREADY) {
            perror("Failed to connect");
            exit(1);
        }
    }
}

bool isMessagePending() {
    //Temporary - to be replaced with ncurses integration
    return true;
}

std::string getUserMessage() {
    //Temporary - to be replaced with ncurses integration
    return "abcdef\n";
}
