/*
 *  HEADER FILE:
 *  client.h
 *  --
 *  PROGRAM: 4981_ass3
 *  --
 *  FUNCTIONS:
 *  void listenTCP(int socket, unsigned long ip, unsigned short port);
 *  void connect(const char *host);
 *  void client(const char *host);
 *  int createSocket(bool nonblocking);
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
 *  This header contains all client related method declarations
 */
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
