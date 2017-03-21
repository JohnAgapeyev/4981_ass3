/*
 *  SOURCE FILE:
 *  packet.cpp
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
 *  This file handles the creation of packets
 */
#include "headers/packet.h"
#include <cstring>

/*
 *  FUNCTION:
 *  genUserPacket
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
 *  INTERFACE:
 *  void genUserPacket(char *packet, int *packetSize, const char *name);
 *  --
 *  ARGS:
 *  char *packet - The packet buffer to write to
 *  int *packetSize - The size of the packet
 *  const char *name - The name of the user that sent the packet
 *  --
 *  RETURNS:
 *  void
 *  --
 *  NOTES:
 *  Generates a packet that is sent when a new user joins
 */
void genUserPacket(char *packet, int *packetSize, const char *name){
    packet[0] = 'u';
    int i = 1;
    for(size_t j = 0; i < *packetSize && j < strlen(name); ++i, ++j) {
        packet[i] = name[j];
    }
    *packetSize = i;
}

/*
 *  FUNCTION:
 *  genMsgPacket
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
 *  INTERFACE:
 *  void genMsgPacket(char *packet, int *packetSize, const char *message, const char *name);
 *  --
 *  ARGS:
 *  char *packet - The packet buffer to write to
 *  int *packetSize - The size of the packet
 *  const char *message - The user's message to send
 *  const char *name - The name of the user that sent the packet
 *  --
 *  RETURNS:
 *  void
 *  --
 *  NOTES:
 *  Generates the packet sent whenever a message is sent by a user
 */
void genMsgPacket(char *packet, int *packetSize, const char *message, const char *name){
    int i = 0;
    packet[i++] = 'm';
    for(size_t j = 0; i < *packetSize && j < strlen(name); ++j) {
        packet[i++] = name[j];
    }
    packet[i++] = ':';
    packet[i++] = ' ';
    for(size_t j = 0; i < *packetSize && j < strlen(message);++j) {
        packet[i++] = message[j];
    }
    *packetSize = i;
}
