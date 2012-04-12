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
	PKT_C2S_CharLoaded = 0xc7,
	PKT_C2S_Exit = 0x95,
	PKT_C2S_ClientReady = 0x64,
	PKT_C2S_GameNumberReq = 0xA5,
	PKT_C2S_Ping_Load_Info = 0x19,
	PKT_S2C_Ping_Load_Info = 0x9E,
	PKT_C2S_Loaded = 0x56,
	PKT_C2S_ViewReq = 0x30,
	PKT_S2C_ViewAns = 0x2E,
	PKT_S2C_FogUpdate = 0x7A,
	PKT_C2S_Click = 0xB8,
	PKT_C2S_AttentionPing = 0x5B,
	PKT_S2C_AttentionPing = 0x47,
	PKT_C2S_Emotion = 0x4D,
	PKT_S2C_Emotion = 0x49,
	PKT_C2S_ReqBuyItem = 0x88,
    PKT_C2S_SkillUp = 0x3E;
    PKT_S2S_SkillUp = 0x18;
    PKT_C2S_OpenShop = 0x61;
    PKT_C2S_PlayerMove = 0x76;

	PKT_S2C_SynchVersion = 0x58,



	PKT_SpawnStart = 0x67,
	PKT_LoadName = 0x66,
	PKT_LoadType = 0x65,

	TOTAL_NUM = 0xff,
};

#endif
