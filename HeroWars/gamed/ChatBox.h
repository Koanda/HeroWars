#ifndef _CHATBOX_H
#define _CHATBOX_H
#include "common.h"
//this is where we'll filtre messages 
enum ChatMessageType : uint32
{
	CMT_ALL = 0,
	CMT_TEAM = 1,
};
#endif