/*
 *  HEADER FILE:
 *  client.h
 *  --
 *  PROGRAM: 4981_ass3
 *  --
 *  FUNCTIONS:
 *  void client();
 *  void connectSock(int socket);
 *  void closeClient(int socket);
 *  void recvClient(int socket, const char *buffer, int packetSize);
 *  --
 *  DATE:
 *  March 20, 2017
 *  --
 *  DESIGNER:
 *  Isaac Morneau
 *  --
 *  PROGRAMMER:
 *  Isaac Morneau
 *  --
 *  NOTES:
 *  This header contains all client related method declarations
 */
#ifndef CLIENT_H
#define CLIENT_H

#include <cstdarg>
#include <climits>
#include <unordered_map>

void client();
void connectSock(int socket);
void closeClient();
void recvClient(int socket, const char *buffer, int packetSize);
#endif
