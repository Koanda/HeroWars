#include "common.h"
#include <enet/enet.h>

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
	static _SpawnPacket* create(PacketCmd cmd, uint8 *str, uint32 size)
	{
		uint32 totalSize = sizeof(_SpawnPacket)+size;
		uint8* buf = new uint8[totalSize];
		memset(buf, 0, totalSize);

		_SpawnPacket *type = (_SpawnPacket *)buf;
		type->header.cmd = cmd;
		type->length = size+1;
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
		return sizeof(_SpawnPacket)+length-1;
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