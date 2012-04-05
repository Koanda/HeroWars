#ifndef _NETWORK_LISTENER_H
#define _NETWORK_LISTENER_H

#include <enet/enet.h>
#include <intlib/base64.h>
#include <intlib/blowfish.h>

#include "common.h"
#include "PacketHandler.h"

#define PEER_MTU 996
class NetworkListener
{
	public:
		NetworkListener();
		~NetworkListener();

		bool initialize(ENetAddress *address, const char *baseKey);
		void netLoop();

	private:
		bool _isAlive;
		PacketHandler *_handler;
		ENetHost *_server;
		BlowFish *_blowfish;
};

#endif

