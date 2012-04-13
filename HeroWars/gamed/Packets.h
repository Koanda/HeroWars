#ifndef _PACKETS_H
#define _PACKETS_H

#include <enet/enet.h>
#include "common.h"
#include "ChatBox.h"

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

typedef struct _SynchBlock
{
	_SynchBlock()
	{
		userId = 0xFFFFFFFFFFFFFFFF;
		someId = 0;
		unk1 = unk2 = 0;
		flag = 0; //1 for bot?
		memset(data1, 0, 64);
		memset(data2, 0, 64);
		unk3 = 0xFFFFFFFF;
	}

	uint64 userId;
	uint16 someId;
	uint32 skill1;
	uint32 skill2;
	uint8 flag;
	uint32 unk1;     //Often 0x64
	uint8 data1[64];
	uint8 data2[64];
	uint32 unk2;     //Low numbers
	uint32 unk3;
} SynchBlock;

typedef struct _SynchVersionAns
{
	_SynchVersionAns()
	{
		cmd = PKT_S2C_SynchVersion;
		unk1 = 0;
		ok = ok2 = 1;
		memcpy(version, "Version 1.0.0.136 [PUBLIC]", 27);
		memcpy(gameMode, "CLASSIC", 8);
		memset(zero, 0, 2232);
		end1 = 0xE2E0;
		end2 = 0xA0;
	}

	uint8 cmd;
	uint32 unk1;
	uint8 ok;
	uint32 mapId;
	SynchBlock players[12];
	uint8 version[27];      //Ending zero so size 26+0x00
	uint8 ok2;              //1
	uint8 unknown[228];     //Really strange shit
	uint8 gameMode[8];
	uint8 zero[2232];
	uint16 end1;            //0xE2E0
	uint8 end2;             //0xA0 || 0x08
} SynchVersionAns;

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

typedef struct _LoadScreenInfo
{
	_LoadScreenInfo()
	{
		//Zero this complete buffer
		memset(this, 0, sizeof(_LoadScreenInfo));

		cmd = PKT_S2C_LoadScreenInfo;
		unk1 = unk2 = 6;		
	}

	uint32 cmd;
	uint32 unk1; //6 in both cases
	uint32 unk2; //""
	uint32 unk3;
	uint64 bluePlayerIds[6]; //Team 1, 6 players max
	uint8 blueData[144];
	uint64 redPlayersIds[6]; //Team 2, 6 players max
	uint8 redData[144];
	uint32 bluePlayerNo;
	uint32 redPlayerNo;
} LoadScreenInfo;

typedef struct _LoadScreenPlayer
{
	static _LoadScreenPlayer* create(PacketCmd cmd, int8 *str, uint32 size)
	{
		//Buils packet buffer
		uint32 totalSize = sizeof(_LoadScreenPlayer)+size+1;
		uint8* buf = new uint8[totalSize];
		memset(buf, 0, totalSize);

		//Set defaults
		_LoadScreenPlayer *packet = (_LoadScreenPlayer *)buf;
		packet->cmd = cmd;
		packet->length = size;
		packet->unk1 = 0;
		packet->skinId = 0;
		memcpy(packet->getDescription(), str, packet->length);
		return packet;
	}

	static void destroy(_LoadScreenPlayer *packet)
	{
		delete [](uint8*)packet;
	}

	uint32 cmd;
	uint32 unk1;
	uint64 userId;
	uint32 skinId;
	uint32 length;
	uint8 description;
	uint8 *getDescription()
	{
		return &description;
	}

	uint32 getPacketLength()
	{
		return sizeof(_LoadScreenPlayer)+length;
	}
} LoadScreenPlayer;


typedef struct _KeyCheck
{
	_KeyCheck()
	{
		cmd = PKT_KeyCheck;
		netId = 0;
		checkId = 0;

	}

	uint8 cmd;
	uint8 partialKey[3];   //Bytes 1 to 3 from the blowfish key for that client
	uint32 netId;
	uint64 userId;         //uint8 testVar[8];   //User id + padding
	uint64 checkId;        //uint8 checkVar[8];  //Encrypted testVar
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

typedef struct _MoveReq
{
	uint8 cmd;
	uint8 sub_cmd1;
	uint16 sub_cmd1Pad; //padding
	uint16 sub_cmd1Def; // defintion of cmd
	float x1;
	float y1;
	float z1;
	uint32 pad2; //padding
	uint32 unk1; //Unk
	uint8 sub_cmd2;
	uint16 sub_cmd2Pad; //padding
	uint16 sub_cmd2Def; // definition of cmd
	short x2; // I don't get that shit
	short y2; // -||-
	short z2; // -||-
} MoveReq;

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

typedef struct _ChatBoxMessage
{
	PacketHeader header;
	ChatMessageType cmd;
	uint32 msgLength;	
} ChatBoxMessage;

typedef struct _HeroSpawnPacket
{
	static _HeroSpawnPacket* create(PacketCmd cmd, int8 *name, uint32 nameLen, int8 *heroType, uint32 heroLen)
	{

		uint32 totalSize = sizeof(_HeroSpawnPacket)+232+1;
		uint8* buf = new uint8[totalSize];
		uint8 packet[] ={ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x50, 0x00, 0xba, 0x19, 0x00,\
		0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x40, 0x18, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,\
		0x49, 0x4d, 0x78, 0x48, 0x6f, 0x74, 0x00, 0xf7, 0x02, 0x5c, 0xcf, 0xa3, 0x02, 0xb8, 0x77, 0xff,\
		0x02, 0xb5, 0xd0, 0x74, 0x00, 0x13, 0xd3, 0x74, 0x00, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,\
		0x00, 0x0c, 0xf8, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0xd0, 0x84, 0x76, 0x00, 0xff, 0xff, 0xff,\
		0xff, 0xf0, 0xa7, 0x5b, 0x00, 0x01, 0x00, 0x00, 0x00, 0xfe, 0xff, 0xff, 0xff, 0x4c, 0xf8, 0x18,\
		0x00, 0x00, 0x00, 0x00, 0x00, 0x51, 0xf8, 0x18, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00,\
		0x00, 0x88, 0x54, 0x78, 0x00, 0x0c, 0xf8, 0x18, 0x00, 0xa0, 0x77, 0xf2, 0x02, 0x20, 0xa3, 0xb6,\
		0x17, 0x9b, 0x35, 0x67, 0x00, 0xff, 0xff, 0xff, 0xff, 0x07, 0x00, 0x00, 0x00, 0x4c, 0xf8, 0x18,\
		0x00, 0x24, 0xf9, 0x18, 0x00, 0x28, 0xf9, 0x18, 0x00, 0x66, 0x00, 0x00, 0x00, 0x3c, 0xe4, 0xa3,\
		0x41, 0x68, 0x72, 0x69, 0x00, 0x3c, 0xe4, 0xa3, 0x02, 0xf8, 0x77, 0xff, 0x02, 0x00, 0x00, 0x00,\
		0x00, 0x8e, 0x31, 0x61, 0x00, 0x07, 0x00, 0x00, 0x00, 0x4c, 0xf8, 0x18, 0x00, 0x03, 0x00, 0x00,\
		0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x14, 0x8b, 0x19, 0x00, 0x00, 0x40, 0x00, 0x00,\
		0x00, 0x00, 0x00, 0x17, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x17, 0x00, 0x02, 0x00, 0x00, 0x00,\
		0x00, 0x17, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00};
		memset(buf, 0, totalSize);
		memcpy(buf+4, packet, totalSize);

		_HeroSpawnPacket *type = (_HeroSpawnPacket *)buf;
		type->header.cmd = cmd;
		type->header.type = 0x06;
		type->header.unk1 = 0x6607;
		
		memcpy(type->playerName, name, nameLen+1);
		memcpy(type->heroType, heroType, heroLen+1);
						
		return type;
	}

	PacketHeader header;
	uint8 top[28];
	uint8 playerName[16];
	uint8 middle[112];
	uint8 heroType[11];
	uint8 bottom[61];

	uint32 getPacketLength()
	{
		return 232;
	}
	
} HeroSpawnPacket;

typedef struct _SkillUpPacket
{
	
	PacketHeader header;
	uint8 unk;
	uint8 skill;
	
	
} SkillUpPacket;

typedef struct _SkillUpResponse
{
	_SkillUpResponse()
	{
		header.cmd = PKT_S2C_SkillUp;
		header.type = 0x19;
		unk = 0x40;
		level = 0x0000;
	}
	PacketHeader header;
	uint8 unk;
	uint8 skill;
	uint16 level; //?
	
	
} SkillUpResponse;

#if defined( __GNUC__ )
#pragma pack()
#else
#pragma pack(pop)
#endif

#endif