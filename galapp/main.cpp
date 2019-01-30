#include "mainbox.h"
#include "pydata.h"
#include "maintray.h"
#include <QtWidgets/QApplication>

int main(int argc, char* argv[])
{
	QApplication a(argc, argv);
	a.setWindowIcon(QIcon(":/logo.png"));
	PyData::GetInstance().init();
	Mainbox w;
	MainTray t(&w);
	t.show();
	w.show();
	return a.exec();
}
