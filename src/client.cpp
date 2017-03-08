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
#include <atomic>


#include "headers/main.h"
#include "headers/server.h"
#include "headers/client.h"

void client(const char *host){
    Socket = createSocket(true);
    connect(host);
    listenForPackets();
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
