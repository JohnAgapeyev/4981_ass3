/*
 *  HEADER FILE:
 *  networkhelpers.h
 *  --
 *  PROGRAM: 4981_ass3
 *  --
 *  FUNCTIONS:
 *  int createNonblock();
 *  void makeNonblock(int sock);
 *  void listenForPackets(bool isClient);
 *  --
 *  DATE:
 *  March 20, 2017
 *  --
 *  DESIGNER:
 *  Isaac Morneau
 *  John Agapeyev
 *  --
 *  PROGRAMMER:
 *  John Agapeyev
 *  Isaac Morneau
 *  --
 *  NOTES:
 *  This header contains the shared shell for network interactions.
 *  It calls the respective server and client functions to handle each type of network event.
 */
#ifndef NETWORKHELPERS_H
#define NETWORKHELPERS_H
#include <limits.h>
#include <atomic>

//high port that shouldnt be used
constexpr int LISTEN_PORT = 45735;
//while many systems try and limit the number of listening
//connections to 5 it usually can support many more
constexpr int LISTENQ = 20;
//dont accept more than 100 events at the same time
constexpr int MAXEVENTS = 100;
//its impossible for any packet to exceede this size so its safe
constexpr int MAXBUFFER = USHRT_MAX;
//the socket made and used as the primary in epoll
extern std::atomic<int> socketfd;

int createNonblock();
void makeNonblock(int sock);
void listenForPackets(bool isClient);

#endif
