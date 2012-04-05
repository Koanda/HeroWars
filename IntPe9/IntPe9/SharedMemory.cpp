#include "IntPe.h"
#include <boost/interprocess/ipc/message_queue.hpp>

using namespace boost::interprocess;

void MessageQueue::run()
{
	message_queue::remove(MQ_MASTER_NAME);
	message_queue *mq = new message_queue(open_or_create, MQ_MASTER_NAME, MQ_NUM_SIZE, MQ_MAX_SIZE);
	size_t sizeReceived;
	unsigned int priority;
	RawPacket *buffer = (RawPacket*)malloc(MQ_MAX_SIZE);
	/**
	 * Start polling
	 */
	while(intPe->isRunning)
	{
		while(mq->get_num_msg() > 0)
		{
			mq->try_receive((char*)buffer, MQ_MAX_SIZE, sizeReceived, priority);
			RawPacket *packet = RawPacket::createPacket(buffer->length, buffer->packet, buffer->type, buffer->cmd);
			intPe->addPacket(packet);
		}
		Sleep(100);
	}

	exit();
}