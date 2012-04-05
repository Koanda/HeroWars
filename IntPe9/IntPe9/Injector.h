#include <QObject>
#include <QThread>
#include <QMetaObject>

class Injector : QObject
{
	Q_OBJECT

public:
	Injector();
	~Injector();
	bool inject();

public slots:
	void listenerLoop();

private:
	bool _isAlive;
	QThread *_thread;


};