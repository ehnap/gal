#pragma once

#include <QSystemTrayIcon>

class Mainbox;
class QAction;
class QMenu;

class MainTray : public QSystemTrayIcon
{
public:
	MainTray(Mainbox* box, QObject* parent = nullptr);
	~MainTray();

public slots:
	void onQuitActionTriggered(bool checked);
	void onStartActionTriggered(bool checked);
	void onActivated(QSystemTrayIcon::ActivationReason reason);

protected:
	bool eventFilter(QObject* o, QEvent* e) override;

private:
	Mainbox* m_box;
	QAction* m_pStartAction;
	QMenu* m_pTrayMenu;
};

