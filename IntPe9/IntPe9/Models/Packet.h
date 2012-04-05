#ifndef PACKET_H
#define PACKET_H

#include <QVariant>
#include <QAbstractListModel>
#include <QPixmap>
#include "Common.h"
#include <ctype.h>

class Packet : public QAbstractListModel
{
	Q_OBJECT

public:
	Packet(RawPacket *data, QObject *parent = 0){};
	Packet(MessagePacket *data, QObject *parent = 0);
	~Packet();

	int columnCount(const QModelIndex &parent) const;
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

	//Custom stuff
	QVariant getField(int column);
	QPixmap getIcon();
	QVariant getSize();
	QVariant getBody();

private:
	//Data
	uint32 _lenght;
	PacketType _type;

	//List view fields
	QString _summary;

	//Detail view
	QString _dLine;
	QString _dHex;
	QString _dAscii;
};

#endif