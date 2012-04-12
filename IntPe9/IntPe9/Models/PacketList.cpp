#include "PacketList.h"

PacketList::PacketList(QObject *parent /* = 0 */) : QAbstractListModel(parent)
{
	_running = true;
	_thread = new QThread;
	moveToThread(_thread);
	_thread->start();

	QMetaObject::invokeMethod(this, "packetPoll", Qt::QueuedConnection);
}

PacketList::~PacketList()
{
	_running = false;
	_thread->exit();
	delete _thread;
}

void PacketList::packetPoll()
{
	MessagePacket *recvPacket = (MessagePacket*)new uint8[MQ_MAX_SIZE];
	uint32 recvdSize, priority;

	message_queue::remove("packetSniffer");
	message_queue *packetQueue = new message_queue(create_only, "packetSniffer", 1000, MQ_MAX_SIZE);
	while(_running)
	{
		packetQueue->receive(recvPacket, MQ_MAX_SIZE, recvdSize, priority);
		if(recvPacket->messagePacketSize() != recvdSize)
			throw; //Malformed packet
		else
		{
			//Handle this packet
			_packets.push_back(new Packet(recvPacket));
			emit layoutChanged();
		}
		//packetQueue
	}
	delete[] recvPacket;
}

Packet *PacketList::getPacketAt(int index)
{
	if(index < 0 && index >= _packets.count())
		return NULL;

	return _packets.at(index);
}

int PacketList::columnCount(const QModelIndex &parent) const
{
	return 4;
}

int PacketList::rowCount(const QModelIndex &parent) const
{
	return _packets.count();
}

QVariant PacketList::data(const QModelIndex &index, int role) const
{
	if (!index.isValid() || index.row() >= _packets.count() || index.column() > 3 || (role != Qt::DisplayRole && role != Qt::DecorationRole) )
		return QVariant();

	return _packets.at(index.row())->getField(index.column());
}

QVariant PacketList::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
		return QVariant();

	switch(section)
	{
		case 0:
			return QString();
		case 1:
			return QString("Size");
		case 2:
			return QString("Data");
		case 3:
			return QString("Description");
		default:
			return QString();
	}
}