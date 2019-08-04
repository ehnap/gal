#pragma once

#include "plugin.h"
#include <QObject>
#include <QMap>
#include <QStackedWidget>

class Plugin;
class OmniPlugin;

class PluginStackedWidget : public QStackedWidget
{
public:
	PluginStackedWidget(QWidget* parent);

	void setCurrentWidget(Plugin::PluginType t);
	QWidget* widget(Plugin::PluginType t);

	void extend();
	

	QSharedPointer<OmniPlugin> getOmniPlugin() const;

protected:
	bool eventFilter(QObject* o, QEvent* e) override;

private:
	void next();
	void prev();
	void shot();
	void clear();

private:
	LabelPluginWidget* m_pLabelPWidget;
	FreeWidget* m_pFreeWidget;
	CppSimpleListWidget* m_pListWidget;

private:
	QSharedPointer<OmniPlugin> m_omniPlugin;
};


class PluginManager : public QObject
{

	Q_OBJECT

public:
	PluginManager(QObject* parent = Q_NULLPTR);
	~PluginManager();

	void setStackedWidget(PluginStackedWidget* pWidget);
	bool isPluginExist(const QString& key);
	Plugin* queryPlugin(const QString& key) const;

public slots:
	void onStartQuery(const QString& key, const QString& value);

private slots:
	void firstInit();

private:
	void init();
	void execQuery(Plugin* pPlugin, const QString& value);

private:
	QMap<QString, Plugin*> m_plugins;
	PluginStackedWidget* m_stackedWidget;
	//QThread* m_workThread;
};
