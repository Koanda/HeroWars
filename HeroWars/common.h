#ifndef _COMMON_H
#define _COMMON_H
#include <stdio.h>
#include <cstring>

typedef char int8;
typedef short int16;
typedef int int32;
typedef long long int64; 
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64; 

enum PacketCmd : uint8
{
	PKT_KeyCheck = 0x00,

	PKT_S2C_QueryStatusAns = 0x8e,
	PKT_World_SendGameNumber = 0x99,
	PKT_C2S_QueryStatusReq = 0x17,
	PKT_C2S_SynchVersion = 0xc6,
	PKT_C2S_CharSelected = 0xc7,
	PKT_C2S_Exit = 0x95,
	PKT_C2S_ClientReady = 0x64,
	PKT_C2S_GameNumberReq = 0xA5,
	PKT_C2S_Ping_Load_Info = 0x19,
	PKT_S2C_Ping_Load_Info = 0x9E,



	PKT_SpawnStart = 0x67,
	PKT_LoadName = 0x66,
	PKT_LoadType = 0x65,

	TOTAL_NUM = 0xff,
};

#endif