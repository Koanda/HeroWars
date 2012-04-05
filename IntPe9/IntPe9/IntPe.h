#ifndef _INTPE_H_
#define _INTPE_H_

#include <QtGui/QApplication>
#include <QCleanlooksStyle>
#include <QThread>
#include "windows.h"

//Common
#include "Common.h"

//Models
#include "Models/PacketList.h"

//Include views for the PE
#include "Views/Main.h"
#include "Views/About.h"

/**
 * IntPe main class
 *
 * @author		Intline9 <Intline9@gmail.com>
 * @version		2010.0722
 */
class MessageQueue : public QThread
{
	public:
		void run();
};

class IntPe
{
	public:
		bool isRunning;
		IntPe();
		~IntPe();

		void run();


		void addPacket(RawPacket *data);

		//Property's
		PacketList *getPacketList()
		{
			return _packetList;
		}
		
	private:
		//Qt
		MessageQueue *messageQue;
		QApplication *application;
		MainGui *mainGui;
		AboutGui *aboutGui;

		void customQt();

		PacketList *_packetList;

		//Hook stuff
		void setUpHooks();

	protected:

};

extern IntPe *intPe;
#endif