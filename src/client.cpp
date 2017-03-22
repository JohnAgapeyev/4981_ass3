#include "headers/networkhelpers.h"
#include "headers/server.h"
#include "headers/client.h"


#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <cstdio>
#include <iostream>
#include <cstring>
#include <thread>

using namespace std;

void client(const char *host){
    thread t(listenForPackets, true, host);

    string text;

    cout << "Please enter your username: " << endl;
    cin >> text;

    if(send(socketfd, text.c_str(), text.size()+1, 0) == -1){
        perror("send failure");
        exit(3);
    }

    while(getline(cin, text)) {
        if(text == "/exit")
            break;
        if(send(socketfd, text.c_str(), text.size()+1, 0) == -1){
            perror("send failure");
            exit(4);
        }
    }

    exit(0);
    t.join();
}

void connectSock(int socket, const char *host) {
    hostent *server;

    if ((server = gethostbyname(host)) == nullptr) {
        perror("Bad host");
        exit(1);
    }

    sockaddr_in addr;
    memset(&addr, 0, sizeof(sockaddr_in));
    addr.sin_family = AF_INET;
    memmove(&addr.sin_addr.s_addr, server->h_addr, server->h_length);
    addr.sin_port = htons(LISTEN_PORT);

    if (connect(socket, (sockaddr *) &addr, sizeof(addr)) < 0) {
        if (errno != EINPROGRESS && errno != EALREADY) {
            perror("Failed to connect");
            exit(2);
        }
    }
}

void closeClient(int socket){

}
void recvClient(int socket, const char *buffer, int packetSize){
    printf("[%d]> %s\n", socket, buffer);
}
