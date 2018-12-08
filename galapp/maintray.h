#pragma once

#include <QSystemTrayIcon>

class MainTray : public QSystemTrayIcon
{
public:
	MainTray(QObject* parent = nullptr);
	~MainTray();

public slots:
	void onQuitActionTriggered(bool checked);
};

