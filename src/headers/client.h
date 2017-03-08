#ifndef CLIENT_H
#define CLIENT_H

#include <cstdarg>
#include <climits>
#include <unordered_map>


void listenTCP(int socket, unsigned long ip, unsigned short port);
void connect(const char *host);
void client(const char *host);
int createSocket(bool nonblocking);

#endif
