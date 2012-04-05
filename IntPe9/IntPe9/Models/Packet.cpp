#include "Packet.h"

#define SUMMARY_LEN 18

Packet::Packet(MessagePacket *data)
{
	try
	{	
		//Copy data
		_lenght = data->length;
		_type = data->type;
		_data = new QByteArray((const char*)data->getData(), data->length);

		//Create list view data
		QString temp;
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
	}
	catch(...)
	{

	}
}

Packet::~Packet()
{
	delete _data;
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

QByteArray *Packet::getData()
{
	return _data;
}