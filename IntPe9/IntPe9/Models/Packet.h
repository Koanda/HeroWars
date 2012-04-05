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
	QVariant getSize();
	QVariant getBody();

	QByteArray *getData();

private:
	//Data
	uint32 _lenght;
	PacketType _type;
	QByteArray *_data;

	//List view fields
	QString _summary;
};

#endif