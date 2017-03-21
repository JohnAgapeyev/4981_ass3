/*
 *  HEADER FILE:
 *  server.h
 *  --
 *  PROGRAM: 4981_ass3
 *  --
 *  FUNCTIONS:
 *  void server();
 *  void listenForPackets();
 *  void listenTCP(int socket, unsigned long ip, unsigned short port);
 *  void connect(const char *host);
 *  int createSocket(bool nonblocking);
 *  void makeNonBlock(int socket);
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
 *  This header contains all server related method declarations
 */
#ifndef SERVER_H
#define SERVER_H

#include <cstdarg>
#include <climits>
#include <string>
#include <map>

#define LISTEN_PORT_TCP 35223
#define LISTENQ 25 //although many kernals define it as 5 usually it can support many more
#define MAXPACKETSIZE USHRT_MAX
#define MAXEVENTS 100

extern char buffer[MAXPACKETSIZE];
extern int Socket;

extern std::map<int,std::string> socketList;

void server();
void listenForPackets();
void listenTCP(int socket, unsigned long ip, unsigned short port);
void connect(const char *host);
int createSocket(bool nonblocking);
void makeNonBlock(int socket);
#endif
