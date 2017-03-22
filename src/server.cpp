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
#include <set>

using namespace std;

set<int> sockets;

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

void recvServer(int sock, const char *buffer, int packetSize){
    if(!sockets.count(sock))
        sockets.insert(sock);
    printf("from %d read %d:%s\n", sock, packetSize, buffer);
    for(const auto fd : sockets) {
        if(fd != sock) {
            if(send(fd, buffer, packetSize, 0) == -1){
                perror("send");
                //dont exit, try and continue
            }
        }
    }
}
