#ifndef SERVER_H
#define SERVER_H

#include <cstdarg>
#include <climits>
#include <unordered_set>

#define LISTEN_PORT_TCP 35223
#define LISTENQ 25 //although many kernals define it as 5 usually it can support many more
#define MAXPACKETSIZE USHRT_MAX
#define MAXEVENTS 100

extern char buffer[MAXPACKETSIZE];
extern int Socket;

extern std::unordered_set<int> socketList;


void server();
void listenForPackets();
void listenTCP(int socket, unsigned long ip, unsigned short port);
void connect(const char *host);
void getMesg();
int createSocket(bool nonblocking);
bool isMessagePending();
std::string getUserMessage();

#endif
