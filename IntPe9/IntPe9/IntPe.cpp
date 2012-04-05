#include "IntPe.h"

//Definition of our pointer
IntPe *intPe;

/**
 * Constructor
 * 
 * Creates the Qt environment
 */
IntPe::IntPe()
{
	isRunning = true;

	//Other stuff
	int argNo = 0;
	application = new QApplication(argNo, NULL);
	application->setStyle(new QCleanlooksStyle);

	//Create the views
	mainGui = new MainGui();
	aboutGui = new AboutGui();

	//Models
	_packetList = new PacketList();

	//Set up the links
	customQt();	
}

/**
 * Destructor
 *
 * Quits the QtApp, deletes gui and app and calls ExitThread
 */
IntPe::~IntPe()
{
	isRunning = false;

	application->quit();


	delete mainGui;
	delete aboutGui;
	delete _packetList;
}

void IntPe::addPacket(RawPacket *data)
{
	_packetList->addPacket(data);
	mainGui->getView()->packetList->scrollToBottom();
}

void IntPe::customQt()
{
	//Models
	mainGui->getView()->packetList->setModel(_packetList);

	//Some custom stuff not able to do from Designer
	//Connects
	QObject::connect(mainGui->getView()->actionAbout, SIGNAL(triggered()), aboutGui, SLOT(slotShow()) );

	
	//QObject::connect(mainGui->getView()->packetList, SIGNAL(clicked(QModelIndex)), mainGui, SLOT(slotOnClickPacketList(QModelIndex)) );
	//QObject::connect(mainGui->getView()->packetList, SIGNAL(pressed(QModelIndex)), mainGui, SLOT(slotOnClickPacketList(QModelIndex)) );
	//mainGui->getView()->packetList->selectionModel()->
	QObject::connect(mainGui->getView()->packetList->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex &, const QModelIndex &)), mainGui, SLOT(slotOnClickPacketList(const QModelIndex &, const QModelIndex &)) );

	//Some initial stuff
}