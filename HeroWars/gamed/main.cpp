/*HeroWars try's to by analoge to the League of Legends server
Copyright (C) 2012  Intline9

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <enet/enet.h>
#include <intlib/base64.h>
#include <intlib/blowfish.h>

/* enet_address_set_host (& address, "x.x.x.x"); */
#define SERVER_HOST ENET_HOST_ANY 
#define SERVER_PORT 5109
#define SERVER_KEY "BQ/rlw1ivzbg7IG/6/iXRQ=="

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64; 



ENetHost *server;
enum LolPacketId : uint8
{
	PKT_KeyCheck = 0x00,
	PKT_S2C_QueryStatusAns = 0x8e,
	PKT_World_SendGameNumber = 0x99,
	PKT_C2S_QueryStatusReq = 0x17,
	PKT_SynchVersionC2S = 0xc6,

	PKT_SpawnStart = 0x67,
	PKT_SpawnName = 0x66,
	PKT_SpawnType = 0x65,
};


typedef struct _ClientInfo
{
	_ClientInfo()
	{
		keyChecked = false;
	}
	bool keyChecked;
	uint64 userId;
} ClientInfo;
#define peerInfo(p) ((ClientInfo*)p->data)

#pragma pack(push, 1) //byte-aligned
typedef struct _PacketHeader
{
	_PacketHeader()
	{
		cmd = type = 0;
		unk1 = 0;
	}
	uint8 cmd;
	uint8 type;
	uint16 unk1;	//?
} PacketHeader;

uint8 *createDynamicPacket(uint8 *str, uint32 size);

typedef struct _SpawnPacket
{
	static _SpawnPacket* create(uint8 cmd, uint8 *str, uint32 size)
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
		return (uint8*)((uint64*)&checkId);
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
#pragma pack(pop)

BlowFish *blowfish;

void printPacket(uint8 *buf, uint32 len)
{
	printf("   ");
	for(uint32 i = 0; i < len; i++)
	{
		printf("%02X ", (uint8)buf[i]);
		if((i+1)%16 == 0)
			printf("\n   ");
	}
	printf("\n");
}

void printLine(uint8 *buf, uint32 len)
{
	for(uint32 i = 0; i < len; i++)
		printf("%02X ", (uint8)buf[i]);
	printf("\n");
}

/*Key check table 

fIUDV63GzaJBP4bm 1: 3F FF 2C B4 10 D7 27 54
fIUDV63GzaJBP4bm 2: 80 BB 48 E3 47 C6 79 D5
*/
void sendPacket(ENetPeer *peer, uint8 channelNo, uint8 *data, uint32 length)
{
	printf(" Sending packet:\n");
	printPacket(data, length);

	if(length >= 8)
		blowfish->Encrypt(data, length-(length%8)); //Encrypt everything minus the last bytes that overflow the 8 byte boundary

	ENetPacket *packet = enet_packet_create(data, length, ENET_PACKET_FLAG_RELIABLE);
	if(enet_peer_send(peer, channelNo, packet) < 0)
		printf("Warning fail, send!");
}


bool handleKeyCheck(ENetPeer *peer, ENetPacket *packet)
{
	uint8 test[8] = {0xed, 0x8c, 0xd2, 0x74, 0xd6, 0x8e, 0xc6, 0xa1};
	uint8 out[8];
	/*
	//Try 1 (IMxTroll)
	C->S
	0000   00 e7 6f 00 00 00 00 00 82 cb 70 01 00 00 00 00  ..o.......p.....
	0010   73 5f d2 c7 c4 26 76 12                          s_...&v.

	S->C
	0000   00 0f eb 97 00 00 00 00 82 cb 70 01 00 00 00 00  ..........p.....
	0010   ed 8c d2 74 d6 8e c6 a1                          ...t....

	//Try 2 (leaving all the same)
	C->S
	0000   00 e7 6f 00 00 00 00 00 82 cb 70 01 00 00 00 00  ..o.......p.....
	0010   78 f5 e8 1c c2 56 a3 fc                          x....V..

	S->C
	0000   00 2a 43 30 00 00 00 00 82 cb 70 01 00 00 00 00  .*C0......p.....
	0010   ed 8c c4 74 7b b6 a3 6f                          ...t{..o

	//Try 3 (colddot)
	C->S
	0000   00 e7 6f 00 00 00 00 00 71 ae 33 01 00 00 00 00  ..o.....q.3.....
	0010   0d 4b f1 4f ba 3d ce 68                          .K.O.=.h

	S->C
	0000   00 e3 8a 4c 00 00 00 00 71 ae 33 01 00 00 00 00  ...L....q.3.....
	0010   ed 8c 31 74 57 6c 85 66                          ..1tWl.f
	*/

	printPacket(packet->data, packet->dataLength);

	KeyCheck *keyCheck = (KeyCheck*)packet->data;

	uint64 userId = blowfish->Decrypt(keyCheck->checkId);

	printf(" Decrypted id: %i\n", userId);
	printf(" User id: %i\n", keyCheck->userId);

	if(userId == keyCheck->userId)
	{
		printf(" User got the same key as i do, go on!\n");
		peerInfo(peer)->keyChecked = true;
	}
	else
	{
		printf(" WRONG KEY, GTFO!!!\n");
		return false;
	}

	keyCheck->checkId = blowfish->Encrypt(userId);
	printf("Encrypt id: ");printLine(keyCheck->pCheckId(), 8);

	printf("Goal: ");printLine(test, 8);
	blowfish->Decrypt(test, out, 8);
	printf("Goal dec: ");printLine(out, 8);
	blowfish->Encrypt(test, out, 8);
	printf("Goal enc: ");printLine(out, 8);

/*
	blowfish->Decrypt(keyCheck->testVar, out, 8);
	printf("Decrypt id: ");printLine(out, 8);

	blowfish->Encrypt(keyCheck->testVar, out, 8);
	printf("Encrypt id: ");printLine(out, 8);

	blowfish->Decrypt(keyCheck->testVar, out, 8);
	blowfish->Decrypt(out, 8);
	printf("Decrypt^2 id: ");printLine(out, 8);

	blowfish->Encrypt(keyCheck->testVar, out, 8);
	blowfish->Encrypt(out, 8);
	printf("Encrypt^2 id: ");printLine(out, 8);*/


	//Send response as this is correct (OFC DO SOME ID CHECKS HERE!!!)
	KeyCheck response;
	response.header.type = 0x1e;
	response.header.unk1 = 0x6f6c;
	response.userId = keyCheck->userId;
	memcpy(response.pCheckId(), test, 8);
	sendPacket(peer, 0, (uint8*)&response, sizeof(KeyCheck));

	WorldSendGameNumber world;
	world.gameId = 1;
	memcpy(world.data, "IMxTroll", 8);
	sendPacket(peer, 3, (uint8*)&world, sizeof(WorldSendGameNumber));

	return true;
}

bool handleVersion(ENetPeer *peer, ENetPacket *packet)
{
	SynchVersion *version = (SynchVersion*)packet->data;
	printf("Client version: %s\n", version->version);

	const char* name = "IMxTroll";
	const char* type = "Ashe";

	uint8 spawnStart[] = {0x67, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0xed, 0x42, 0x05, 0xd5, \
		0x82, 0xcb, 0x70, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
		0x00, 0x0f, 0xeb, 0x97, 0x00, 0x00, 0x00, 0x00, 0x82, 0xcb, 0x70, 0x01, 0x00, 0x00, 0x00, 0x00, \
		0xed, 0x8c, 0xd2, 0x74, 0xd6, 0x8e, 0xc6, 0xa1, 0xfe, 0xff, 0xff, 0xff, 0x3c, 0xf1, 0x18, 0x00, \
		0xb2, 0x0c, 0x6a, 0x00, 0xf0, 0xf5, 0x18, 0x00, 0xed, 0x8c, 0xd2, 0x74, 0x46, 0x8e, 0xc6, 0xa1, \
		0xfe, 0xff, 0xff, 0xff, 0x58, 0x4c, 0xd2, 0x74, 0xa0, 0x0c, 0x6a, 0x00, 0x50, 0x81, 0xed, 0x05, \
		0x28, 0x7d, 0x65, 0x03, 0x00, 0xf8, 0x5b, 0x48, 0x00, 0xf8, 0x5b, 0x48, 0x0c, 0x27, 0x37, 0x75, \
		0x18, 0x37, 0x3a, 0x04, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 0x28, 0x7d, 0x65, 0x03, \
		0x9e, 0xbc, 0x5b, 0x00, 0x9c, 0xf7, 0x18, 0x00, 0xdd, 0xbd, 0x5b, 0x00, 0x20, 0x74, 0x6f, 0x20, \
		0x61, 0x1b, 0x00, 0x00, 0x6d, 0x61, 0x70, 0x70, 0x69, 0x6e, 0x67, 0x73, 0x20, 0x3a, 0x2c, 0x20, \
		0x30, 0x2c, 0x20, 0x31, 0x00, 0x36, 0x2c, 0x20, 0x74, 0x6f, 0x20, 0x65, 0x6e, 0x65, 0x74, 0x20, \
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
		0x02, 0xc7, 0xd3, 0x74, 0x50, 0xf2, 0x18, 0x00, 0x08, 0x76, 0x60, 0x03, 0x98, 0x15, 0x3f, 0x00, \
		0x2f, 0x2e, 0xd2, 0x74, 0x00, 0xf2, 0x18, 0x00, 0xb4, 0xf2, 0x18, 0x00, 0x64, 0x1a, 0xd3, 0x74, \
		0x0a, 0x00, 0x00, 0x00, 0x91, 0x1a, 0xd3, 0x74, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x1b, 0xdb, 0x74, \
		0x63, 0x20, 0xd3, 0x74, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x91, 0x1a, 0xd3, 0x74, \
		0x00, 0x00, 0x00, 0x00, 0x90, 0xf4, 0x18, 0x00, 0x1c, 0xf5, 0x18, 0x00, 0x2b, 0x00, 0x00, 0x00, \
		0x08, 0x76, 0x60, 0x03, 0x98, 0x15, 0x3f, 0x00, 0xd0, 0x07, 0x3f, 0x00, 0x01, 0x00, 0x00, 0x00, \
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x86, 0x05, 0xac, 0x02, \
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
		0x46, 0x00, 0x00, 0x00, 0xf8, 0x1b, 0xdb, 0x74, 0x04, 0xf5, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, \
		0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};


	SpawnPacket *spawnName = SpawnPacket::create(PKT_SpawnName, (uint8*)name, strlen(name));
	spawnName->userId = ((ClientInfo*)peer->data)->userId;

	SpawnPacket *spawnType = SpawnPacket::create(PKT_SpawnType, (uint8*)type, strlen(type));
	spawnType->userId = ((ClientInfo*)peer->data)->userId;

	sendPacket(peer, 6, (uint8*)spawnStart, sizeof(spawnStart));
	sendPacket(peer, 6, (uint8*)spawnName, spawnName->getPacketLength());
	sendPacket(peer, 6, (uint8*)spawnType, spawnType->getPacketLength());


	return true;
}

bool handlePacket(ENetPeer *peer, ENetPacket *packet)
{
	if(packet->dataLength >= 8)
	{
		if(peerInfo(peer)->keyChecked)
			blowfish->Decrypt(packet->data, packet->dataLength-(packet->dataLength%8)); //Encrypt everything minus the last bytes that overflow the 8 byte boundary
	}

	PacketHeader *header = (PacketHeader*)packet->data;

	QueryStatus ok;
	switch(header->cmd)
	{
		case PKT_KeyCheck:
			printf("PKT_KeyCheck\n");
			handleKeyCheck(peer, packet);
		break;
		case PKT_C2S_QueryStatusReq:
			printf("PKT_C2S_QueryStatusReq\n");
			sendPacket(peer, 3, (uint8*)&ok, sizeof(QueryStatus));
		break;
		case PKT_SynchVersionC2S:
			printf("PKT_SynchVersionC2S\n");
			handleVersion(peer, packet);
		break;
		default:
			printf("Unknown packet: %X\n", header->cmd);
	}

	return true;
}

int main (int argc, char ** argv) 
{
	printf("HeroWars 0.0.0\n");
	if (enet_initialize () != 0)
	{
		printf ("An error occurred while initializing ENet.\n");
		return EXIT_FAILURE;
	}
	atexit (enet_deinitialize);


	ENetAddress address;
	address.host = SERVER_HOST;
	address.port = SERVER_PORT;

	server = enet_host_create(&address, 32, 0, 0);
	if (server == NULL)
	{
		printf ("An error occurred while trying to create an ENet server host.\n");
		exit (EXIT_FAILURE);
	}

	std::string key = base64_decode(SERVER_KEY);
	blowfish = new BlowFish((uint8*)key.c_str(), 16);

	ENetEvent event;

	/* Wait up to 1000 milliseconds for an event. */
	printf("NetLoop\n");
	while(1)
	{
		while (enet_host_service (server, & event, 10) > 0)
		{
			switch (event.type)
			{
			case ENET_EVENT_TYPE_CONNECT:
				uint8 ip[16];
				enet_address_get_host_ip(&event.peer->address, (char*)ip, 16);

				printf ("A new client connected from %s:%u\n", 
					ip,
					event.peer -> address.port);

				/* Store any relevant client information here. */
				event.peer->data = new ClientInfo();

				break;

			case ENET_EVENT_TYPE_RECEIVE:
				if(!handlePacket(event.peer, event.packet))
					enet_peer_disconnect(event.peer, 0);

				/* Clean up the packet now that we're done using it. */
				enet_packet_destroy (event.packet);
				break;

			case ENET_EVENT_TYPE_DISCONNECT:
				printf ("Peer disconected.\n");

				/* Reset the peer's client information. */
				delete event.peer->data;
			}
		}
	}
	enet_host_destroy(server);
	system("PAUSE");
}