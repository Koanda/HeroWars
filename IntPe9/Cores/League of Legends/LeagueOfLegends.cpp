#include "LeagueOfLegends.h"

bool doFirst = true;
MessagePacket *sendBuf;
MessagePacket *recvBuf;
map<uint16, MessagePacket*> reassemble;

LeagueOfLegends::LeagueOfLegends()
{
	//Blowfish key extraction
	char *cmd = GetCommandLineA();
	uint8 q = 0, sCur = 0;
	uint16 sStart = 0, sEnd = 0;
	for(uint16 i = 0; i < strlen(cmd); i++)
	{
		if(q >= 9)
		{
			if(cmd[i] == ' ')
				sCur++;
			if(sCur == 1)
				sStart = i+2;
			if(sCur == 2)
				sEnd = i+1;
		}
		else
			if(cmd[i] == '"')
				q++;
	}
	_keySize = sEnd-sStart;
	_key = (uint8*)malloc(_keySize+1);
	uint8 *_keyDecrypted = (uint8*)malloc(_keySize*2);
	memcpy((char*)_key, &cmd[sStart], _keySize);

	//Create blowfish
	DbgPrint("The base64 key they use is: %s", _key);

	uint32 size = decode_base64(_keyDecrypted, (const char*)_key);
	blowfish = new BlowFish(_keyDecrypted, size);

}

void LeagueOfLegends::initialize()
{
	_oldWSASendTo = (defWSASendTo)_upx->hookIatFunction(NULL, "WSASendTo", (unsigned long)&newWSASendTo);
	_oldWSARecvFrom = (defWSARecvFrom)_upx->hookIatFunction(NULL, "WSARecvFrom", (unsigned long)&newWSARecvFrom);

	sendBuf = (MessagePacket*)new uint8[MQ_MAX_SIZE];
	recvBuf = (MessagePacket*)new uint8[MQ_MAX_SIZE];
	DbgPrint("League of Legends engine started!");
}

void LeagueOfLegends::finalize()
{
	delete[]sendBuf;
	delete[]recvBuf;
}

uint8 *reassemblePacket(ENetProtocol *command, uint32 length, uint32 *dataLength)
{
	uint32 fragmentNumber, fragmentCount, fragmentOffset, totalLength, startSequenceNumber, fragmentLength;

	fragmentLength = ENET_NET_TO_HOST_16(command->sendFragment.dataLength);
	startSequenceNumber = ENET_NET_TO_HOST_16(command->sendFragment.startSequenceNumber);
	fragmentNumber = ENET_NET_TO_HOST_32(command->sendFragment.fragmentNumber);
	fragmentCount = ENET_NET_TO_HOST_32(command->sendFragment.fragmentCount);
	fragmentOffset = ENET_NET_TO_HOST_32(command->sendFragment.fragmentOffset);
	totalLength = ENET_NET_TO_HOST_32(command->sendFragment.totalLength);

	//As i have no idea if these packets are getting correctly sequenced, but i hope so... else my code would crash so hard
	*dataLength = totalLength;
	if(fragmentNumber == 0)
	{
		MessagePacket *packet = (MessagePacket*)new uint8[totalLength+sizeof(MessagePacket)];
		packet->length = totalLength;
		reassemble[startSequenceNumber] = packet;	
	}

	MessagePacket *packet = reassemble[startSequenceNumber];
	memcpy(&packet->getData()[fragmentOffset], (uint8*)command+sizeof(ENetProtocolSendFragment), fragmentLength);

	//If reassembled, return data, else NULL
	if(fragmentNumber == fragmentCount-1)
		return (uint8*)packet;
	else
		return NULL;
}
uint8 *parseEnet(char *buffer, uint32 length, uint32 *dataLength, bool *isFragment)
{
	if(length < sizeof(ENetProtocolHeader) || length > MQ_MAX_SIZE)
		return NULL;

	*dataLength = 0;
	uint16 peerId, flags, headerSize, size;
	ENetProtocolHeader* header = (ENetProtocolHeader*)buffer;
	peerId = ENET_NET_TO_HOST_16 (header->peerID);
	flags = peerId & ENET_PROTOCOL_HEADER_FLAG_MASK;
	peerId &= ~ ENET_PROTOCOL_HEADER_FLAG_MASK;
	headerSize = (flags & ENET_PROTOCOL_HEADER_FLAG_SENT_TIME ? sizeof (ENetProtocolHeader) : (size_t) & ((ENetProtocolHeader *) 0) -> sentTime);

	if(length <  headerSize+sizeof(ENetProtocolCommandHeader))
		return NULL;


	ENetProtocol *command = (ENetProtocol*)(buffer+headerSize);

	uint8 cmd = command->header.command & ENET_PROTOCOL_COMMAND_MASK;

	if((cmd == ENET_PROTOCOL_COMMAND_SEND_RELIABLE || cmd == ENET_PROTOCOL_COMMAND_SEND_UNRELIABLE || cmd == ENET_PROTOCOL_COMMAND_SEND_UNSEQUENCED))
	{
		uint32 maLen = 0;
		switch (cmd)
		{
			case ENET_PROTOCOL_COMMAND_SEND_RELIABLE:
				maLen = ENET_NET_TO_HOST_16(command->sendReliable.dataLength);
				break;
			case ENET_PROTOCOL_COMMAND_SEND_UNRELIABLE:
				maLen = ENET_NET_TO_HOST_16(command->sendUnreliable.dataLength);
				break;
			case ENET_PROTOCOL_COMMAND_SEND_UNSEQUENCED:
				maLen = ENET_NET_TO_HOST_16(command->sendUnsequenced.dataLength);
				break;
		}

		leagueOfLegends->DbgPrint("[RECV] Parse, cmd: %i, channel: %i, size: %i", cmd, command->header.channelID, maLen);
	}

	switch (command->header.command & ENET_PROTOCOL_COMMAND_MASK)
	{
		case ENET_PROTOCOL_COMMAND_SEND_RELIABLE:
			*dataLength = size = ENET_NET_TO_HOST_16(command->sendReliable.dataLength);
			return (uint8*)command + sizeof(ENetProtocolSendReliable);
		case ENET_PROTOCOL_COMMAND_SEND_UNRELIABLE:
			 *dataLength = size = ENET_NET_TO_HOST_16(command->sendUnreliable.dataLength);
			return (uint8*)command + sizeof(ENetProtocolSendUnreliable);
		case ENET_PROTOCOL_COMMAND_SEND_UNSEQUENCED:
			*dataLength = size = ENET_NET_TO_HOST_16(command->sendUnsequenced.dataLength);
			return (uint8*)command + sizeof(ENetProtocolSendUnsequenced);
		case ENET_PROTOCOL_COMMAND_SEND_FRAGMENT:
			*isFragment = true;
			return reassemblePacket(command, length, dataLength);

		//All enet specefic stuff, so ignore it
		case ENET_PROTOCOL_COMMAND_ACKNOWLEDGE:
		case ENET_PROTOCOL_COMMAND_CONNECT:
		case ENET_PROTOCOL_COMMAND_VERIFY_CONNECT:
		case ENET_PROTOCOL_COMMAND_DISCONNECT:
		case ENET_PROTOCOL_COMMAND_PING:
		case ENET_PROTOCOL_COMMAND_BANDWIDTH_LIMIT:
		case ENET_PROTOCOL_COMMAND_THROTTLE_CONFIGURE:
		default:
			return NULL;
	}
	return NULL;
}

/** IMPORTANT
 * Enet sends all headers and buff seperated, lucky for us!
 * lpBuffer[0] = ENetProtocolHeader
 * lpBuffer[1,3,5...] = ENetProtocol
 * lpBuffer[2,4,6...] = Packet data
 */
int WSAAPI newWSASendTo(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesSent, DWORD dwFlags, const struct sockaddr *lpTo, int iToLen, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
{
	if(dwBufferCount >= 3)
	{
		//Loop if we have 2 buffers left
		for(uint32 i = 1; i <= dwBufferCount-2; i+=2)
		{
			ENetProtocol *command = (ENetProtocol*)(lpBuffers[i].buf);
			uint8 cmd = command->header.command & ENET_PROTOCOL_COMMAND_MASK;

			//Skip if not a data packet
			if(!(cmd == ENET_PROTOCOL_COMMAND_SEND_RELIABLE || cmd == ENET_PROTOCOL_COMMAND_SEND_UNRELIABLE || cmd == ENET_PROTOCOL_COMMAND_SEND_UNSEQUENCED))
				continue;

			leagueOfLegends->DbgPrint("[SEND] Parse, cmd: %i, channel: %i, size: %i", cmd, command->header.channelID, lpBuffers[i+1].len);
		
			sendBuf->type = WSASENDTO;
			sendBuf->length = lpBuffers[i+1].len;
			memcpy(sendBuf->getData(), lpBuffers[i+1].buf, sendBuf->length);
			if(sendBuf->length >= 8)
				if(!doFirst)
					leagueOfLegends->blowfish->Decrypt(sendBuf->getData(), sendBuf->length-(sendBuf->length%8));
				else
					doFirst = false;
			leagueOfLegends->sendPacket(sendBuf);
		}

	}
	return leagueOfLegends->_oldWSASendTo(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags, lpTo, iToLen, lpOverlapped, lpCompletionRoutine);
}

int WSAAPI newWSARecvFrom(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesRecvd, LPDWORD lpFlags, struct sockaddr *lpFrom, LPINT lpFromlen, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
{
	int returnLength = leagueOfLegends->_oldWSARecvFrom(s, lpBuffers, dwBufferCount, lpNumberOfBytesRecvd, lpFlags, lpFrom, lpFromlen, lpOverlapped, lpCompletionRoutine);

	if(returnLength == 0)
	{
		bool isFragment = false;
		uint32 dataLength;
		uint8 *data = parseEnet(lpBuffers[0].buf, *lpNumberOfBytesRecvd, &dataLength, &isFragment);		
		if(data != NULL && dataLength > 0)
		{
			try
			{
				recvBuf->type = WSARECVFROM;
				if(isFragment)
				{
					MessagePacket *total = (MessagePacket*)data;
					if(total->length >= 8)
						leagueOfLegends->blowfish->Decrypt(total->getData(), total->length-(total->length%8));
					leagueOfLegends->sendPacket(total);
				}
				else
				{				
					recvBuf->length = dataLength;
					memcpy(recvBuf->getData(), data, recvBuf->length);
					if(recvBuf->length >= 8)
						leagueOfLegends->blowfish->Decrypt(recvBuf->getData(), recvBuf->length-(recvBuf->length%8));
					leagueOfLegends->sendPacket(recvBuf);
				}

			}
			catch(...)
			{
				leagueOfLegends->DbgPrint("Dafuq this bug splat!!!");
			}

		}
	}

	return returnLength;
}