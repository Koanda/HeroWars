#include "Packet.h"

#define SUMMARY_LEN 18

Packet::Packet(MessagePacket *data)
{
	try
	{	
		//Copy data
		_length = data->length;
		_type = data->type;
		_data = new QByteArray((const char*)data->getData(), data->length);
		_description = QString(data->description);

		//Create list view data
		QString temp;
		for(uint8 i = 0; i < ((_length > SUMMARY_LEN) ? SUMMARY_LEN : _length); i++)
		{
			temp.sprintf("%02X ", (uint8)data->getData()[i]);
			_summary.append(temp);
		}
		if(_length > SUMMARY_LEN)
		{
			temp.sprintf("... %02X %02X", (uint8)data->getData()[_length-2], (uint8)data->getData()[_length-1]);
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

QString Packet::strInfoHeader()
{
	QString out;
	out += "Packet: ";
		if(_type == WSASENDTO || _type == WSASEND)
			out += "C -> S";
		else
			out += "S -> C";

	out += ", Length: " + QString::number(_length);
	out += ", Description: " + _description + "\n";

	return out;
}

QString Packet::strFullDump()
{
	QString out;
	QString format;
	for(uint32 s = 0, x = 0, c = 0; s < _length; s+=16)
	{
		//Format line number
		out += format.sprintf("%08x     ", s);

		//Format hex
		for(x = s, c = 0; x < _length && c < 16; x++, c++)
			out += format.sprintf("%02X ", (uint8)_data->at(x));

		//Pad remaining space
		for(; c < 16; c++)
			out += "   ";
		out += "    "; 

		//Format ascii
		for(x = s, c = 0; x < _length && c < 16; x++, c++)
			out += (QChar(_data->at(x)).isLetterOrNumber() ? QChar(_data->at(x)) : '.');
		out += "\n";

		//Break if we are done
		if(s+c >= _length)
			break;
	}
	return out;
}

QVariant Packet::getField(int column)
{
	switch(column)
	{
		case 0:
			return getIcon();
		case 1:
			return getLength();
		case 2:
			return getBody();
		case 3:
			return _description;
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

QVariant Packet::getBody()
{
	return _summary;
}

PacketType Packet::getType()
{
	return _type;
}

uint32 Packet::getLength()
{
	return _length;
}

QByteArray *Packet::getData()
{
	return _data;
}