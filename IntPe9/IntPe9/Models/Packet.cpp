#include "Packet.h"

#define SUMMARY_LEN 18
Packet::Packet(MessagePacket *data, QObject *parent /* = 0 */) : QAbstractListModel(parent)
{
	try
	{	
	uint32 max;
	QString temp;
	QChar character;

	//Copy data
	_lenght = data->length;
	_type = data->type;

	//Create list view data
	for(uint8 i = 0; i < ((_lenght > SUMMARY_LEN) ? SUMMARY_LEN : _lenght); i++)
	{
		temp.sprintf("%02X ", (uint8)data->getData()[i]);
		_summary.append(temp);
	}
	if(_lenght > SUMMARY_LEN)
	{
		temp.sprintf("... %02X %02X", (uint8)data->getData()[_lenght-2], (uint8)data->getData()[_lenght-1]);
		_summary.append(temp);
	}

	//Create detail data
	for(uint16 i = 0; i < _lenght; i += 16)
	{
		//Line numbers
		temp.sprintf("%06i    \n", i);
		_dLine.append(temp);

		//Hex and Ascii view
		max = (i + 16 > _lenght) ? i + (_lenght % 16) : i + 16;
		if(i == 0 && max == 0)
			max = _lenght;
		for(uint16 j = i; j < max; j++)
		{
			//Hex
			temp.sprintf("%02X ", (uint8)data->getData()[j]);
			_dHex.append(temp);

			//Ascii
			character = isprint(data->getData()[j]) ? data->getData()[j] : '.';
			_dAscii.append(character);
		}


		//Line break
		_dHex.append("   \n");
		_dAscii.append("\n");
	}
	}
	catch(...)
	{

	}
	//Let the core clean it up (for now perhaps later to improve this)
	//free((char*)data);
}

Packet::~Packet()
{

}

QVariant Packet::getField(int column)
{
	switch(column)
	{
		case 0:
			return getIcon();
		case 1:
			return getSize();
		case 2:
			return getBody();
		default:
			return QVariant();
	}
}

int Packet::columnCount(const QModelIndex &parent) const
{
	return 3;
}

int Packet::rowCount(const QModelIndex &parent) const
{
	return 1;
}

QVariant Packet::data(const QModelIndex &index, int role) const
{
	if (!index.isValid() || index.row() > 1 || index.column() > 2 || (role != Qt::DisplayRole) )
		return QVariant();

	switch(index.column())
	{
		case 0:
			return _dLine;
		case 1:
			return _dHex;
		case 2:
			return _dAscii;
		default:
			return QVariant();
	}
}

QVariant Packet::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
		return QVariant();

	switch(section)
	{
	case 0:
		return QString("Offset");
	case 1:
		return QString("Hex");
	case 2:
		return QString("Ascii");
	default:
		return QVariant();
	}
}

QPixmap Packet::getIcon()
{
	if(_type == WSASENDTO)
		return QPixmap(":/Common/Resources/send.gif");
	else if(_type == WSARECVFROM)
		return QPixmap(":/Common/Resources/recv.gif");
	else if(_type == WSARECV)
		return QPixmap(":/Common/Resources/logo.png");
	else
		return QPixmap();
}	

QVariant Packet::getSize()
{
	return _lenght;
}

QVariant Packet::getBody()
{
	return _summary;
}