/*
 *  SOURCE FILE:
 *  client.cpp
 *  --
 *  PROGRAM: 4981_ass3
 *  --
 *  FUNCTIONS:
 *  void client(const char *host);
 *  void connect(const char *host);
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
 *  This files handles the client specific connection calls.
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
#include <unordered_map>
#include <string>
#include <iostream>
#include <thread>
#include <atomic>


#include "headers/main.h"
#include "headers/server.h"
#include "headers/client.h"
#include "headers/packet.h"

/*
 *  FUNCTION:
 *  client
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
 *  void client(const char *host);
 *  --
 *  ARGS:
 *  const char *host - The hostname to connect to
 *  --
 *  RETURNS:
 *  void
 *  --
 *  NOTES:
 *  Main client function called from main
 */
void client(const char *host){
    Socket = createSocket(true);
    connect(host);
    listenForPackets();
}

/*
 *  FUNCTION:
 *  connect
 *  --
 *  DATE:
 *  March 20, 2017
 *  --
 *  DESIGNER:
 *  John Agapeyev
 *  --
 *  PROGRAMMER:
 *  John Agapeyev
 *  Isaac Morneau
 *  --
 *  INTERFACE:
 *  void connect(const char *host);
 *  --
 *  ARGS:
 *  const char *host - The hostname to connect to
 *  --
 *  RETURNS:
 *  void
 *  --
 *  NOTES:
 *  Connects the client to a given hostname
 */
void connect(const char *host) {
    hostent *server;

    if ((server = gethostbyname(host)) == nullptr) {
        //Temporary - to signal error to user
        perror("Bad host");
        exit(14);
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
            exit(15);
        }
    }
    const auto& name = ui->getName();
    int buffSize = 33;
    char nameBuff[33];
    genUserPacket(nameBuff, &buffSize, name.c_str());
    if(send(Socket,nameBuff, buffSize, 0) == -1){
        perror("send failure");
        exit(16);
    }
}
