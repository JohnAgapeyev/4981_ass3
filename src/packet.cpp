#include "headers/packet.h"
#include <cstring>


void genUserPacket(char *packet, int *packetSize, const char *name){
    packet[0] = 'u';
    int i = 1;
    for(int j = 0; i < *packetSize && j < strlen(name); ++i, ++j)
        packet[i] = name[j];
    *packetSize = i;
}

void genMsgPacket(char *packet, int *packetSize, const char *message, int32_t channel){
    packet[0] = 'm';
    int32_t *id = reinterpret_cast<int32_t*>(packet + 1);
    *id = channel;
    int i = 5;
    for(int j = 0; i < *packetSize && j < strlen(message); ++i, ++j)
        packet[i] = message[j];
    *packetSize = i;
}
