#ifndef PACKET_H
#define PACKET_H

#include "defines.h"

/*
Header for the Packet



*/
#pragma pack(push, 1)
struct packet
{
    enum message_type_e message_type;
    u8 id[ PUBLIC_ID_BYTE_SIZE];
    u32 timestamp_seconds;
    u8 message_length;
};
#pragma pack(pop) 


#endif