#include "About.h"

AboutGui::AboutGui(QWidget *parent, Qt::WFlags flags)
: QDialog(parent, flags)
{
	_aboutView.setupUi(this);
}

AboutGui::~AboutGui()
{

}

void AboutGui::slotShow()
{
	this->show();
}

void AboutGui::slotHide()
{
	this->hide();
}

Ui::aboutView *AboutGui::getView()
{
	return &_aboutView;
}