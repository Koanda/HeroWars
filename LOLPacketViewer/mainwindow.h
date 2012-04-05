#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QModelIndex>
#include <QItemSelection>
#include "logfile.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionOpen_activated();
    void on_packetsTreeView_selectionChanged ( const QItemSelection & selected, const QItemSelection & deselected);
    void on_actionAbout_us_activated();

private:
    Ui::MainWindow *ui;
    LogFile m_file;
};

#endif // MAINWINDOW_H
