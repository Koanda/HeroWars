#include "PacketHandler.h"

const PacketTable table[] =
{
	{PKT_KeyCheck,                   &PacketHandler::handleKeyCheck             },
	{PKT_C2S_Ping_Load_Info,         &PacketHandler::handleLoadPing             },
	{PKT_C2S_CharLoaded,             &PacketHandler::handleSpawn                },
	{PKT_C2S_ClientReady,            &PacketHandler::handleMap                  },
	{PKT_C2S_SynchVersion,           &PacketHandler::handleSynch                },
	{PKT_C2S_GameNumberReq,          &PacketHandler::handleGameNumber           },
	{PKT_C2S_QueryStatusReq,         &PacketHandler::handleQueryStatus          },
	{PKT_C2S_Loaded,                 &PacketHandler::handleInit                 },
	{PKT_C2S_Exit,                   &PacketHandler::handleNull                 },
	{PKT_C2S_ViewReq,                &PacketHandler::handleView                 },
	{PKT_C2S_Click,                  &PacketHandler::handleNull                 },
	{PKT_C2S_AttentionPing,          &PacketHandler::handleAttentionPing        },
};

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
	PDEBUG_LOG(Log::getMainInstance(),"   ");
	for(uint32 i = 0; i < len; i++)
	{
		PDEBUG_LOG(Log::getMainInstance(),"%02X ", static_cast<uint8>(buf[i]));
		if((i+1)%16 == 0)
			PDEBUG_LOG(Log::getMainInstance(),"\n   ");
	}
	PDEBUG_LOG(Log::getMainInstance(),"\n");
}

void PacketHandler::printLine(uint8 *buf, uint32 len)
{
	for(uint32 i = 0; i < len; i++)
		PDEBUG_LOG(Log::getMainInstance(),"%02X ", static_cast<uint8>(buf[i]));
	PDEBUG_LOG(Log::getMainInstance(),"\n");
}

bool PacketHandler::sendPacket(ENetPeer *peer, uint8 *data, uint32 length, uint8 channelNo, uint32 flag)
{
	PDEBUG_LOG_LINE(Log::getMainInstance()," Sending packet:\n");
	if(length < 300)
		printPacket(data, length);

	if(length >= 8)
		_blowfish->Encrypt(data, length-(length%8)); //Encrypt everything minus the last bytes that overflow the 8 byte boundary

	ENetPacket *packet = enet_packet_create(data, length, flag);
	if(enet_peer_send(peer, channelNo, packet) < 0)
	{
		PDEBUG_LOG_LINE(Log::getMainInstance(),"Warning fail, send!");
		return false;
	}
	return true;
}

bool PacketHandler::broadcastPacket(uint8 *data, uint32 length, uint8 channelNo, uint32 flag)
{
	PDEBUG_LOG_LINE(Log::getMainInstance()," Broadcast packet:\n");
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

	PacketHeader *header = reinterpret_cast<PacketHeader*>(packet->data);

	bool handled = false;

	for(uint32 i = 0; i < TOTAL_HANDLERS; ++i)
	{
		if(table[i].cmd == header->cmd)
		{
			handled = true;
			return (*this.*table[i].handler)(peer, packet);
		}
	}

	if(!handled)
	{
		PDEBUG_LOG_LINE(Log::getMainInstance(),"Unknown packet: %X(%i)\n", header->cmd, header->cmd);
	}

	return handled;
}