#include "Main.h"
#include "IntPe.h"

MainGui::MainGui(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags)
{
	_mainView.setupUi(this);

	//Create all sub views
	_aboutGui = new AboutGui(this);

	//
	_injector = new Injector();
	_packetList = new PacketList();
	_mainView.packetList->setModel(_packetList);

	//Custom GUI setup
	_mainView.packetList->horizontalHeader()->setResizeMode(QHeaderView::Fixed);
	_mainView.packetView->horizontalHeader()->setResizeMode(QHeaderView::Fixed);
	_mainView.packetView->verticalHeader()->setDefaultAlignment(Qt::AlignTop);

	_mainView.packetList->horizontalHeader()->resizeSection(0, 30);
	_mainView.packetList->horizontalHeader()->resizeSection(1, 50);
	//_mainView.packetList->horizontalHeader()->resizeSection(3, 100);

	connect(_mainView.actionAbout, SIGNAL(triggered()), _aboutGui, SLOT(slotShow()) );
	connect(_mainView.packetList->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(slotOnClickPacketList(const QModelIndex &, const QModelIndex &)) );
	//_mainView.packetList->setColumnWidth(0, 17);
	//_mainView.packetList->setColumnWidth(1, 40);
	//_mainView.packetList->setColumnWidth(2, 100);
}

MainGui::~MainGui()
{

}

void MainGui::slotOnClickPacketList(const QModelIndex &current, const QModelIndex &previous)
{
	_mainView.packetView->setModel(_packetList->getPacketAt(current.row()));
	_mainView.packetView->resizeRowsToContents();
	_mainView.packetView->horizontalHeader()->resizeSection(0, 60);
	_mainView.packetView->horizontalHeader()->resizeSection(1, 350);
	_mainView.packetView->horizontalHeader()->resizeSection(2, 120);
	//_mainView.packetView->resizeColumnsToContents();
	//
}

void MainGui::slotShow()
{
	show();
}

void MainGui::slotHide()
{
	hide();
}

Ui::mainView *MainGui::getView()
{
	return &_mainView;
}
