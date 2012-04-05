#ifndef ABOUT_H
#define ABOUT_H

#include <QtGui/QDialog>
#include "ui_About.h"

class AboutGui : public QDialog
{
	Q_OBJECT

public:
	AboutGui(QWidget *parent = 0, Qt::WFlags flags = 0);
	~AboutGui();

	Ui::aboutView *getView();

public slots:
	void slotShow();
	void slotHide();

private:
	Ui::aboutView _aboutView;
};

#endif // ABOUT_H
