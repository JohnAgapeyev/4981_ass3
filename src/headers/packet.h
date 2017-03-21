/*
 *  HEADER FILE:
 *  packet.h
 *  --
 *  PROGRAM: 4981_ass3
 *  --
 *  FUNCTIONS:
 *  void genUserPacket(char *packet, int *packetSize, const char *name);
 *  void genMsgPacket(char *packet, int *packetSize, const char *message, const char *name);
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
 *  This header contains all packetization related method declarations
 */
#ifndef PACKET_H
#define PACKET_H
#include <cstdint>

void genUserPacket(char *packet, int *packetSize, const char *name);
void genMsgPacket(char *packet, int *packetSize, const char *message, const char *name);

#endif
