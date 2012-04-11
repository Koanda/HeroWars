#ifndef _CLIENT_H
#define _CLIENT_H

#include "common.h"

typedef struct _ClientInfo
{
	_ClientInfo()
	{
		keyChecked = false;
		name = type = NULL;
	}

	~_ClientInfo()
	{
		if(name != NULL)
			delete[] name;
		if(type != NULL)
			delete[] type;
	}

	void setName(char *name)
	{
		if(this->name != NULL)
			delete[] this->name;

		nameLen = strlen(name);
		this->name = new int8[nameLen+1];
		memcpy(this->name, name, nameLen+1);
	}

	void setType(char *type)
	{
		if(this->type != NULL)
			delete[] this->type;

		typeLen = strlen(type);
		this->type = new int8[typeLen+1];
		memcpy(this->type, type, typeLen+1);
	}

	bool keyChecked;
	uint64 userId;

	uint32 nameLen;
	uint32 typeLen;
	int8 *name;
	int8 *type;

} ClientInfo;
#define peerInfo(p) ((ClientInfo*)p->data)

#endif