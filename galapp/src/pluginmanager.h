#pragma once

#include "plugin.h"
#include <QObject>
#include <QMap>
#include <QStackedWidget>

class Plugin;


class PluginStackedWidget : public QStackedWidget
{
public:
	PluginStackedWidget(QWidget* parent);

	void setCurrentWidget(Plugin::PluginType t);
	QWidget* widget(Plugin::PluginType t);

	void extend();

private:
	LabelPluginWidget* m_pLabelPWidget;
	FreeWidget* m_pFreeWidget;
};


class PluginManager : public QObject
{

	Q_OBJECT

public:
	PluginManager(QObject* parent = Q_NULLPTR);
	~PluginManager();

	void setStackedWidget(PluginStackedWidget* pWidget);
	bool isPluginExist(const QString& key);

public slots:
	void onStartQuery(const QString& key, const QString& value);

private slots:
	void firstInit();

private:
	void init();

private:
	QMap<QString, Plugin*> m_plugins;
	PluginStackedWidget* m_stackedWidget;
	//QThread* m_workThread;
};
