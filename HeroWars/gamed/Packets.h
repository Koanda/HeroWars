#ifndef _PACKETS_H
#define _PACKETS_H

#include <enet/enet.h>
#include "common.h"

#if defined( __GNUC__ )
#pragma pack(1)
#else
#pragma pack(push,1)
#endif

typedef struct _PacketHeader
{
	_PacketHeader()
	{
		type = 0;
		unk1 = 0;
	}
	PacketCmd cmd;
	uint8 type;
	uint16 unk1;	//?
} PacketHeader;

typedef struct _PingLoadInfo
{
	PacketHeader header;
	uint8 unk0;
	uint32 unk1;
	uint64 userId;
	float loaded;
	float ping;
	float f3;
	uint8 unk4;
}PingLoadInfo;

uint8 *createDynamicPacket(uint8 *str, uint32 size);

typedef struct _SpawnPacket
{
	static _SpawnPacket* create(PacketCmd cmd, int8 *str, uint32 size)
	{
		uint32 totalSize = sizeof(_SpawnPacket)+size+1;
		uint8* buf = new uint8[totalSize];
		memset(buf, 0, totalSize);

		_SpawnPacket *type = (_SpawnPacket *)buf;
		type->header.cmd = cmd;
		type->length = size;
		memcpy(type->getDescription(), str, type->length);
		return type;
	}

	PacketHeader header;
	uint32 unk1;
	uint64 userId;
	uint32 unk2;
	uint32 length;
	uint8 description;
	uint8 *getDescription()
	{
		return &description;
	}

	uint32 getPacketLength()
	{
		return sizeof(_SpawnPacket)+length;
	}
} SpawnPacket;


typedef struct _KeyCheck
{
	_KeyCheck()
	{
		header.cmd = PKT_KeyCheck;
		userId = checkId = 0; //memset(testVar, 0, sizeof(testVar));
		//memset(checkVar, 0, sizeof(checkVar));
		unk1 = 0;
	}

	PacketHeader header;
	uint32 unk1;
	uint64 userId; //uint8 testVar[8];   //User id + padding
	uint64 checkId; //uint8 checkVar[8];  //Encrypted testVar

	uint8 *pCheckId()
	{
		return reinterpret_cast<uint8*>((reinterpret_cast<uint64*>(&checkId)));
	}
} KeyCheck;

typedef struct _AttentionPing
{
	_AttentionPing()
	{

	}
	_AttentionPing(_AttentionPing *ping)
	{
		cmd = ping->cmd;
		unk1 = ping->unk1;
		x = ping->x;
		y = ping->y;
		z = ping->z;
		unk2 = ping->unk2;
		type = ping->type;
	}

	uint8 cmd;
	uint32 unk1;
	float x;
	float y;
	float z;
	uint32 unk2;
	uint8 type;
} AttentionPing;

typedef struct _AttentionPingAns
{
	_AttentionPingAns(AttentionPing *ping)
	{
		attentionPing = AttentionPing(ping);
		attentionPing.cmd = PKT_S2C_AttentionPing;
	}

	AttentionPing attentionPing;
	uint32 response;
} AttentionPingAns;
typedef struct _ViewReq
{
	uint8 cmd;
	uint32 unk1;
	float x;
	float zoom;
	float y;
	float y2;		//Unk
	uint32 width;	//Unk
	uint32 height;	//Unk
	uint32 unk2;	//Unk
	uint8 requestNo;
} ViewReq;

typedef struct _ViewAns
{
	_ViewAns()
	{
		cmd = PKT_S2C_ViewAns;
		unk1 = 0;
	}

	uint8 cmd;
	uint32 unk1;
	uint8 requestNo;
} ViewAns;

typedef struct _FogUpdate
{
	_FogUpdate()
	{
		cmd = PKT_S2C_FogUpdate;
		x1 = 0;
		y1 = 0;
		x2 = 0.5;
		y2 = 0.5;
	}
	uint8 cmd;
	float x1;
	float y1;
	float x2;
	float y2;
} FogUpdate;

typedef struct _QueryStatus
{
	_QueryStatus()
	{
		header.cmd = PKT_S2C_QueryStatusAns;
		ok = ENET_HOST_TO_NET_16(1);
	}
	PacketHeader header;
	uint16 ok;
} QueryStatus;

/* TODO*/
typedef struct _SynchVersion
{
	PacketHeader header;
	uint8 ok;
	uint64 unk1;
	uint8 version[27];
	uint32 unk2;
	uint32 unk3;

} SynchVersion;
typedef struct _WorldSendGameNumber
{
	_WorldSendGameNumber()
	{
		header.cmd = PKT_World_SendGameNumber;
		memset(data, 0, sizeof(data));
		gameId = 0;
		ok = 0;
	}

	PacketHeader header;
	uint8 ok;
	uint64 gameId;
	uint8 data[0x80];
} WorldSendGameNumber;

#if defined( __GNUC__ )
#pragma pack()
#else
#pragma pack(pop)
#endif

#endif