#include "mainbox.h"
#include "pydata.h"
#include "maintray.h"
#include <QtWidgets/QApplication>
#include <QTranslator>

int main(int argc, char* argv[])
{
	QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QApplication a(argc, argv);
	QTranslator qTranslator;
	qTranslator.load(qApp->applicationDirPath() + "\\i18n\\galapp_zh_CN.qm");
	a.installTranslator(&qTranslator);
	a.setWindowIcon(QIcon(":/logo.png"));
	PyData::GetInstance().init();
	Mainbox w;
	MainTray t(&w);
	t.show();
	w.show();
	return a.exec();
}
