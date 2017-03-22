#ifndef NETWORKHELPERS_H
#define NETWORKHELPERS_H
#include <limits.h>

constexpr int LISTEN_PORT = 45735;
constexpr int LISTENQ = 20;
constexpr int MAXEVENTS = 100;
constexpr int MAXBUFFER = USHRT_MAX;

int createNonblock();
void makeNonblock(int sock);
void listenForPackets(bool isClient);

//the socket made and used as the primary in epoll
extern int socketfd;

#endif
