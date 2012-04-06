#ifndef MAIN_H
#define MAIN_H

#include <QMessageBox>
#include <QtGui/QMainWindow>
#include <QStyledItemDelegate>
#include <QFile>


#include "Models/PacketList.h"
#include "ui_Main.h"
#include "About.h"
#include "Injector.h"
#include "QHexEdit/qhexedit.h"

class MainGui : public QMainWindow
{
	Q_OBJECT

public:
	MainGui(QWidget *parent = 0, Qt::WFlags flags = 0);
	~MainGui();

	Ui::mainView* getView();

	public slots:
		void saveAllAsText();
		void slotOnClickPacketList(const QModelIndex &current, const QModelIndex &previous);
		void slotShow();
		void slotHide();

private:
	//Views
	QHexEdit *_hexView;
	AboutGui *_aboutGui;
	Ui::mainView _mainView;
	Injector *_injector;
	PacketList *_packetList;
};

#endif // MAIN_H
