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
    stringstream ss;
    switch (*buffer) {
        case '/':
            {
                ss.str(temp);
                ss.clear();

                ss >> temp;
                if (temp == "/users") {
                    ss.str("");
                    ss << "0 Current Users:";
                    for(const auto& us : sockets)
                        ss << ' ' << us.first << ':' << us.second;
                    temp = ss.str();
                    printf("sending userlist %s\n", temp.c_str());
                    if (send(sock, temp.c_str(), temp.size()+1, 0) == -1) {
                        perror("send");
                        exit(7);
                    }
                } else if (temp == "/set") {
                    ss >> temp;
                    if(temp == "name") {
                        ss >> temp;
                        sockets[sock] = temp;
                        getline(ss, temp);
                        sockets[sock] = temp;

                        ss.str("");
                        ss.clear();
                        ss << "/userupdate";
                        for(const auto& fd : sockets)
                            ss << '\n' << fd.first << ' ' << fd.second;
                        temp = ss.str();
                        printf("sending updated client list, %s\n", temp.c_str());
                        for(const auto& fd : sockets) {
                            if (send(fd.first, temp.c_str(), temp.size()+1, 0) == -1) {
                                perror("send");
                                exit(8);
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
