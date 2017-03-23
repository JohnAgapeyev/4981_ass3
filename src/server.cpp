#include "headers/server.h"
#include "headers/networkhelpers.h"

#include <cstring>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <cstdio>
#include <map>
#include <sstream>
#include <iostream>

using namespace std;

map<int, string> sockets;

void server() {
    listenForPackets(false);
}

void listenSock(int socket){
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(LISTEN_PORT); 

    if ((bind(socket, (struct sockaddr *) &addr, sizeof(addr))) == -1) {
        perror("bind");
        exit(5);
    }

    if (listen(socket, LISTENQ) == -1) {
        perror("listen");
        exit(6);
    }
}

void closeServer(int sock){
    sockets.erase(sock);
}

void recvServer(int sock, const char *buffer, int packetSize) {
    string temp(buffer);
    stringstream ss(temp);
    switch (*buffer) {
        case '/':
            {
                ss >> temp;
                if (temp == "/ips") {
                    ss.str("");
                    ss.clear();
                    ss << "Connect users:";

                    struct sockaddr_in addr;
                    socklen_t len = sizeof(addr);

                    for(const auto& us : sockets) {
                        getpeername(us.first, (struct sockaddr*)(&addr), &len);
                        ss << '\n' << us.second << ':' << string(inet_ntoa(addr.sin_addr));
                    }

                    temp = ss.str();
                    if (send(sock, temp.c_str(), temp.size()+1, 0) == -1) {
                        perror("send");
                        exit(7);
                    }
                } else if (temp == "/set") {
                    ss >> temp;
                    if(temp == "name") {
                        ss >> temp;
                        sockets[sock] = temp;
                        if(getline(ss, temp))
                            sockets[sock] += temp;

                        ss.str("");
                        ss.clear();

                        ss << "/userupdate ";
                        for(const auto& fd : sockets)
                            ss << fd.first << ' ' << fd.second << '\n';
                        temp = ss.str();
                        for(const auto& fd : sockets) {
                            if (send(fd.first, temp.c_str(), temp.size()+1, 0) == -1) {
                                perror("send");
                                exit(7);
                            }
                        }
                    }
                }
            }
            break;
        default:
            {
                ss << sock << ' ' << temp;
                temp = ss.str();
                for (const auto& fd : sockets) {
                    if (fd.first != sock) {
                        if (send(fd.first, temp.c_str(), temp.size()+1, 0) == -1) {
                            perror("send");
                            exit(9);
                        }
                    }
                }
                break;
            }
    }
}
