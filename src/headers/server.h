#ifndef SERVER_H
#define SERVER_H

void server();
void closeServer(int sock);
void listenSock(int socket);
void recvServer(int sock, const char *buffer, int packetSize); 

#endif
