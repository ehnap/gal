#include "maintray.h"
#include <QMenu>
#include <QApplication>

MainTray::MainTray(QObject* parent /*= nullptr*/)
	: QSystemTrayIcon(parent)
{
	QMenu* pTrayMenu = new QMenu(Q_NULLPTR);
	
	QAction* pQuitAction = pTrayMenu->addAction("quit");
	pQuitAction->setIconVisibleInMenu(false);
	connect(pQuitAction, &QAction::triggered, this, &MainTray::onQuitActionTriggered);
	setIcon(QIcon(":/logo.png"));
	setContextMenu(pTrayMenu);
}

MainTray::~MainTray()
{
}

void MainTray::onQuitActionTriggered(bool checked)
{
    Q_UNUSED(checked);
	QApplication::quit();
}
