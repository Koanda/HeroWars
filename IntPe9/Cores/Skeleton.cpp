#include "Skeleton.h"
#include <Windows.h>

void Skeleton::DbgPrint(const char* format, ...)
{
	//Reset the buffer
	memset(_dbgPrint, 0x0, 512);

	//Logic
	va_list args;
	va_start(args, format);
	vsprintf_s(_dbgPrint, 512, format, args);
	va_end(args);
	OutputDebugStringA(_dbg);
}

bool Skeleton::sendCommand()
{
	return false;
}

bool Skeleton::sendPacket(MessagePacket *packet)
{
	try
	{
		bool ret = _packetQue->try_send((char*)packet, packet->messagePacketSize(), 0); //instant returns, we can have packet loss but keeping client alive is more important
		return ret;
	}
	catch(...)
	{
		DbgPrint("Exception raised on try send, packetLength: %i, structLength: %i", packet->length, packet->messagePacketSize());
		return false;
	}
}

Skeleton::Skeleton()
{
	//Initalising of our DbgPrint buffer
	_dbg = (char*)malloc(521);
	memcpy(_dbg, "[IntPe9] ", 9);
	_dbgPrint = &_dbg[9];

	_upx = new Upx();
	//_masterQue = new message_queue(open_or_create, MQ_MASTER_NAME, MQ_MASTER_NUM_SIZE, 4);
	_packetQue = new message_queue(open_only, "packetSniffer");
}

Skeleton::~Skeleton()
{
	free(_dbgPrint);
}