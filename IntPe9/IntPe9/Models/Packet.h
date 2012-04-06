#ifndef PACKET_H
#define PACKET_H

#include <QVariant>
#include <QAbstractListModel>
#include <QPixmap>
#include <QByteArray>
#include "Common.h"
#include <ctype.h>

class Packet : public QObject
{
	Q_OBJECT

public:
	Packet(MessagePacket *data);
	~Packet();

	//Custom stuff
	QVariant getField(int column);
	QPixmap getIcon();
	QVariant getBody();

	uint32 getLength();
	PacketType getType();
	QByteArray *getData();

	QString strInfoHeader();
	QString strFullDump();

private:
	//Data
	uint32 _length;
	PacketType _type;
	QByteArray *_data;

	//List view fields
	QString _summary;
};

#endif