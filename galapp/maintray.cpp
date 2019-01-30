#include "maintray.h"
#include "mainbox.h"
#include <QMenu>
#include <QApplication>
#include <QSettings>
#include <QDir>

MainTray::MainTray(Mainbox* box, QObject* parent /*= nullptr*/)
	: QSystemTrayIcon(parent)
	, m_box(box)
{
	m_pTrayMenu = new QMenu(Q_NULLPTR);
	
	m_pStartAction = m_pTrayMenu->addAction("start with bootup");
	m_pStartAction->setIconVisibleInMenu(false);
	m_pStartAction->setCheckable(true);
	connect(m_pStartAction, &QAction::triggered, this, &MainTray::onStartActionTriggered);

	QAction* pQuitAction = m_pTrayMenu->addAction("quit");
	pQuitAction->setIconVisibleInMenu(false);
	connect(pQuitAction, &QAction::triggered, this, &MainTray::onQuitActionTriggered);

	connect(this, &MainTray::activated, this, &MainTray::onActivated);
	setIcon(QIcon(":/logo.png"));
	setContextMenu(m_pTrayMenu);

	m_pTrayMenu->installEventFilter(this);
}

MainTray::~MainTray()
{
}

void MainTray::onQuitActionTriggered(bool checked)
{
    Q_UNUSED(checked);
	QApplication::quit();
}

void MainTray::onStartActionTriggered(bool checked)
{
	QSettings* pReg = new QSettings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
	if (checked)
		pReg->setValue("Gal", QDir::toNativeSeparators(QApplication::applicationFilePath()));
	else
		pReg->remove("Gal");
}

void MainTray::onActivated(QSystemTrayIcon::ActivationReason reason)
{
	switch (reason)
	{
	case QSystemTrayIcon::DoubleClick:
		m_box->popUp();
		break;
	}
}

bool MainTray::eventFilter(QObject* o, QEvent* e)
{
	if (o == m_pTrayMenu && e->type() == QEvent::Show)
	{
		QSettings* pReg = new QSettings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
		m_pStartAction->setChecked(pReg->contains("Gal") && pReg->value("Gal").toString() == QDir::toNativeSeparators(QApplication::applicationFilePath()));
	}

	return QSystemTrayIcon::eventFilter(o, e);
}
