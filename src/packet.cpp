#include "headers/packet.h"
#include <cstring>


void genUserPacket(char *packet, int *packetSize, const char *name){
    packet[0] = 'u';
    int i = 1;
    for(int j = 0; i < *packetSize && j < strlen(name); ++i, ++j)
        packet[i] = name[j];
    *packetSize = i;
}

void genMsgPacket(char *packet, int *packetSize, const char *message, const char *name){
    int i = 0;
    packet[i++] = 'm';
    for(int j = 0; i < *packetSize && j < strlen(name); ++j)
        packet[i++] = name[j];
    packet[i++] = ':';
    packet[i++] = ' ';
    for(int j = 0; i < *packetSize && j < strlen(message);++j)
        packet[i++] = message[j];
    *packetSize = i;
}
