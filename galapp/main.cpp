#include "mainbox.h"
#include "pydata.h"
#include "maintray.h"
#include <QtWidgets/QApplication>

int main(int argc, char* argv[])
{
	QApplication a(argc, argv);
	a.setWindowIcon(QIcon(":/logo.png"));
	PyData::GetInstance().init();
	MainTray t;
	t.show();
	Mainbox w;
	w.show();
	return a.exec();
}
