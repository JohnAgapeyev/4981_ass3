#ifndef PACKET_H
#define PACKET_H
#include <stdint.h>

void genUserPacket(char *packet, int *packetSize, const char *name);
void genMsgPacket(char *packet, int *packetSize, const char *message, int32_t channel = -1);

#endif
