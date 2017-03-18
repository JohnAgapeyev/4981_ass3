#ifndef PACKET_H
#define PACKET_H
#include <stdint.h>

void genUserPacket(char *packet, int *packetSize, const char *name);
void genMsgPacket(char *packet, int *packetSize, const char *message, const char *name);

#endif
