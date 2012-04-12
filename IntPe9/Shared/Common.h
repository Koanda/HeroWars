#ifndef _COMMON_H_
#define _COMMON_H_


#include <iostream>
#include <vector>
#include <cstdarg>

#define MQ_NUM_SIZE	0x400
#define MQ_MAX_SIZE	0x4000

#define MQ_MASTER_NUM_SIZE	0x100
#define MQ_MASTER_MAX_SIZE	0x100
#define MQ_MASTER_NAME		"IntPe9Master"

typedef long long		int64;
typedef int				int32;
typedef short			int16;
typedef char			int8; 
typedef unsigned long long	uint64;
typedef unsigned int		uint32;
typedef unsigned short		uint16;
typedef unsigned char		uint8;

enum PacketType : uint8
{
	SEND = 1,
	RECV,
	SENDTO,
	RECVFROM,
	WSASEND,
	WSARECV,
	WSASENDTO,
	WSARECVFROM,
};

#pragma pack(push)
#pragma pack(1)
#pragma warning(push)
#pragma warning(disable: 4200)

class MessagePacket
{
public:
	PacketType type;
	int8 description[50];
	uint32 length;
	uint8 data;

	uint8 *getData()
	{
		return &data;
	}

	uint32 messagePacketSize()
	{
		return length+sizeof(MessagePacket);
	}
};

#pragma warning(pop)
#pragma pack(pop)

#endif