#ifndef CLIENT_H
#define CLIENT_H

#include <cstdarg>
#include <climits>
#include <unordered_map>


void client();
void listenTCP(int socket, unsigned long ip, unsigned short port);
void connect(const char *host);
void getMesg();
int createSocket(bool nonblocking);
bool isMessagePending();
std::string getUserMessage();

#endif
