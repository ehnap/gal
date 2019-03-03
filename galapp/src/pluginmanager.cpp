#include "pluginmanager.h"
#include <QTimer>
#include <QApplication>
#include <QDir>
#include <QSettings>
#include <QThread>

PluginManager::PluginManager(QObject* parent /*= Q_NULLPTR*/)
	: QObject(parent)
{
	QTimer::singleShot(0, this, &PluginManager::firstInit);
}

PluginManager::~PluginManager()
{
}

void PluginManager::setStackedWidget(PluginStackedWidget* pWidget)
{
	m_stackedWidget = pWidget;
}

void PluginManager::onStartQuery(const QString& key, const QString& value)
{
	Plugin* p = m_plugins.value(key);
	if (!p)
		return;

	//根据类型选择对应widget
	m_stackedWidget->setCurrentWidget(p->type());
	m_stackedWidget->show();
	p->exec(value, m_stackedWidget->widget(p->type()));
}

void PluginManager::firstInit()
{
// 	m_workThread = new QThread(this);
// 	moveToThread(m_workThread);
// 	m_workThread->start();

	init();
}

void PluginManager::init()
{
	//scan the plugin
	QString pluginsDir = QDir::toNativeSeparators(qApp->applicationDirPath() + "\\plugin");
	QDir d(pluginsDir);
	QFileInfoList l = d.entryInfoList(QDir::Dirs);
	foreach(const QFileInfo& info, l)
	{
		if (info.isDir())
		{
			QString configFilePath = QDir::toNativeSeparators(info.absoluteFilePath() + "\\config.ini");
			QSettings configSetting(configFilePath, QSettings::IniFormat);
			if (configSetting.status() == QSettings::NoError)
			{
				QString strKey = configSetting.value("key").toString();
				QString strName = configSetting.value("name").toString();
				QString strVer = configSetting.value("version").toString();
				QString strAuthor = configSetting.value("author").toString();
				Plugin::PluginType t = Plugin::getTypeFromStr(configSetting.value("type").toString());
				switch (t)
				{
				case Plugin::PluginType::JS_SIMPLE:
				{
					JsSimplePlugin* p = new JsSimplePlugin(this, strName, strKey, strVer, strAuthor, info.filePath());
					m_plugins.insert(strKey, p);
					break;
				}
				case Plugin::PluginType::CPP_FREE:
				{
					CppFreePlugin* p = new CppFreePlugin(this, strName, strKey, strVer, strAuthor, info.filePath());
					m_plugins.insert(strKey, p);
					break;
				}
				}
			}
		}
	}
}

PluginStackedWidget::PluginStackedWidget(QWidget* parent)
	: QStackedWidget(parent)
{
	m_pLabelPWidget = new LabelPluginWidget(this);
	m_pFreeWidget = new FreeWidget(this);

	addWidget(m_pLabelPWidget);
	addWidget(m_pFreeWidget);
}

void PluginStackedWidget::setCurrentWidget(Plugin::PluginType t)
{
	switch (t)
	{
	case Plugin::PluginType::JS_SIMPLE:
		QStackedWidget::setCurrentWidget(m_pLabelPWidget);
		break;
	case Plugin::PluginType::CPP_FREE:
		QStackedWidget::setCurrentWidget(m_pFreeWidget);
		break;
	case Plugin::PluginType::UNKNOWN_TYPE:
		break;
	default:
		break;
	}
}

QWidget* PluginStackedWidget::widget(Plugin::PluginType t)
{
	QWidget* pWidget = currentWidget();
	switch (t)
	{
	case Plugin::PluginType::JS_SIMPLE:
		pWidget = m_pLabelPWidget;
		break;
	case Plugin::PluginType::CPP_FREE:
		pWidget = m_pFreeWidget;
		break;
	case Plugin::PluginType::UNKNOWN_TYPE:
		break;
	default:
		break;
	}
	return pWidget;
}

void PluginStackedWidget::extend()
{
	if (!isVisible())
		return;

	PluginWidget* pWidget = qobject_cast<PluginWidget*>(currentWidget());
	if (pWidget)
		pWidget->extend();
}
