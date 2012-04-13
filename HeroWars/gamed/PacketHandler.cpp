#include "PacketHandler.h"

PacketHandler::PacketHandler(ENetHost *server, BlowFish *blowfish)
{
	_server = server;
	_blowfish = blowfish;
	memset(_handlerTable,0,sizeof(_handlerTable));
	registerHandler(&PacketHandler::handleKeyCheck,PKT_KeyCheck,CHL_HANDSHAKE);
	registerHandler(&PacketHandler::handleLoadPing,PKT_C2S_Ping_Load_Info,CHL_C2S);
	registerHandler(&PacketHandler::handleSpawn,PKT_C2S_CharLoaded,CHL_C2S);
	registerHandler(&PacketHandler::handleMap,PKT_C2S_ClientReady,CHL_LOADING_SCREEN);
	registerHandler(&PacketHandler::handleSynch,PKT_C2S_SynchVersion,CHL_C2S);
	registerHandler(&PacketHandler::handleGameNumber,PKT_C2S_GameNumberReq,CHL_C2S);
	registerHandler(&PacketHandler::handleQueryStatus,PKT_C2S_QueryStatusReq,CHL_C2S);
	registerHandler(&PacketHandler::handleInit,PKT_C2S_Loaded,CHL_C2S);
	registerHandler(&PacketHandler::handleNull,PKT_C2S_Exit,CHL_C2S);
	registerHandler(&PacketHandler::handleView,PKT_C2S_ViewReq,CHL_C2S);
	registerHandler(&PacketHandler::handleNull,PKT_C2S_Click,CHL_C2S);
	registerHandler(&PacketHandler::handleAttentionPing,PKT_C2S_AttentionPing,CHL_C2S);
	registerHandler(&PacketHandler::handleChatBoxMessage ,PKT_ChatBoxMessage,CHL_COMMUNICATION);
}

PacketHandler::~PacketHandler()
{
	memset(_handlerTable,0,sizeof(_handlerTable)); //i prefer cleaning everything
}

void PacketHandler::registerHandler(bool (PacketHandler::*handler)(HANDLE_ARGS), PacketCmd pktcmd,Channel c)
{
	_handlerTable[pktcmd][c] = handler;
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

bool PacketHandler::handlePacket(ENetPeer *peer, ENetPacket *packet, uint8 channelID)
{
	if(packet->dataLength >= 8)
	{
		if(peerInfo(peer)->keyChecked)
			_blowfish->Decrypt(packet->data, packet->dataLength-(packet->dataLength%8)); //Encrypt everything minus the last bytes that overflow the 8 byte boundary
	}

	PacketHeader *header = reinterpret_cast<PacketHeader*>(packet->data);	
	bool (PacketHandler::*handler)(HANDLE_ARGS) = _handlerTable[header->cmd][channelID];
	
	if(handler)
	{
		return (*this.*handler)(peer,packet);
	}
	else
	{
		PDEBUG_LOG_LINE(Log::getMainInstance(),"Unknown packet: CMD %X(%i) CHANNEL %X(%i)\n", header->cmd, header->cmd,channelID,channelID);
	}
	return false;	
}