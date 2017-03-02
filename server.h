#ifndef SERVER_H
#define SERVER_H

#include <cstdarg>
#include <climits>
#include <unordered_map>

#define LISTEN_PORT_TCP 35223
#define LISTENQ 25 //although many kernals define it as 5 usually it can support many more
#define MAXPACKETSIZE USHRT_MAX
#define MAXEVENTS 100

extern char buffer[MAXPACKETSIZE];
extern int listenSocketTCP;
extern int sendSocketTCP;

extern std::unordered_map<unsigned long, std::string> clientList;

void processPacket(const char *data);
void listenForPackets();
void listenTCP(int socket, unsigned long ip, unsigned short port);
int createSocket(bool nonblocking);

#endif
