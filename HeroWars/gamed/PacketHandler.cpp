#include "PacketHandler.h"

typedef struct PacketTable
{
	PacketCmd cmd;
	bool (PacketHandler::*handler)(HANDLE_ARGS);
}AuthHandler;

const PacketTable table[] =
{
	{PKT_KeyCheck,				&PacketHandler::handleKeyCheck		},
	{PKT_C2S_Ping_Load_Info,	&PacketHandler::handleLoadPing		},
	{PKT_C2S_CharSelected,		&PacketHandler::handleSpawn			},
	{PKT_C2S_ClientReady,		&PacketHandler::handleMap			},
	{PKT_C2S_SynchVersion,		&PacketHandler::handleSynch			},
	{PKT_C2S_GameNumberReq,		&PacketHandler::handleGameNumber	},
	{PKT_C2S_QueryStatusReq,	&PacketHandler::handleQueryStatus	},
};
#define TOTAL_HANDLERS sizeof(table)/sizeof(PacketTable)

PacketHandler::PacketHandler(ENetHost *server, BlowFish *blowfish)
{
	_server = server;
	_blowfish = blowfish;
}

PacketHandler::~PacketHandler()
{

}

void PacketHandler::printPacket(uint8 *buf, uint32 len)
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

void PacketHandler::printLine(uint8 *buf, uint32 len)
{
	for(uint32 i = 0; i < len; i++)
		printf("%02X ", (uint8)buf[i]);
	printf("\n");
}

bool PacketHandler::sendPacket(ENetPeer *peer, uint8 *data, uint32 length, uint8 channelNo, uint32 flag)
{
	printf(" Sending packet:\n");
	printPacket(data, length);

	if(length >= 8)
		_blowfish->Encrypt(data, length-(length%8)); //Encrypt everything minus the last bytes that overflow the 8 byte boundary

	ENetPacket *packet = enet_packet_create(data, length, flag);
	if(enet_peer_send(peer, channelNo, packet) < 0)
	{
		printf("Warning fail, send!");
		return false;
	}
	return true;
}

bool PacketHandler::broadcastPacket(uint8 *data, uint32 length, uint8 channelNo, uint32 flag)
{
	printf(" Broadcast packet:\n");
	printPacket(data, length);

	if(length >= 8)
		_blowfish->Encrypt(data, length-(length%8)); //Encrypt everything minus the last bytes that overflow the 8 byte boundary

	ENetPacket *packet = enet_packet_create(data, length, flag);

	enet_host_broadcast(_server, channelNo, packet);
	return true;
}

bool PacketHandler::handlePacket(ENetPeer *peer, ENetPacket *packet)
{
	if(packet->dataLength >= 8)
	{
		if(peerInfo(peer)->keyChecked)
			_blowfish->Decrypt(packet->data, packet->dataLength-(packet->dataLength%8)); //Encrypt everything minus the last bytes that overflow the 8 byte boundary
	}

	PacketHeader *header = (PacketHeader*)packet->data;

	bool handled = false;
	for(uint32 i = 0; i < TOTAL_HANDLERS; ++i)
	{
		if(table[i].cmd ==  header->cmd)
		{
			if(!(*this.*table[i].handler)(peer, packet))
			{
				printf("Handler failed for: %i", header->cmd);
				return false;
			}
			handled = true;
		}
	}

	if(!handled)
	{
		printf("Unknown packet: %X(%i)\n", header->cmd, header->cmd);
	}

	return true;
}