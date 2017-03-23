/*
 *  HEADER FILE:
 *  server.h
 *  --
 *  PROGRAM: 4981_ass3
 *  --
 *  FUNCTIONS:
 *  void server();
 *  void closeServer(int sock);
 *  void listenSock(int socket);
 *  void recvServer(int sock, const char *buffer, int packetSize); 
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
 *  This header contains all server related method declarations
 */
#ifndef SERVER_H
#define SERVER_H

void server();
void closeServer(int sock);
void listenSock(int socket);
void recvServer(int sock, const char *buffer, int packetSize); 

#endif
