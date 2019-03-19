#include "omniobject.h"
#include "pluginmanager.h"
#include "data.h"

#include <QGuiApplication>
#include <QDir>
#include <QFile>
#include <QDesktopServices>
#include <QUrl>

OmniObject::OmniObject(OmniType ot, RespondType rt)
	: m_omniType(ot)
	, m_respondType(rt)
{

}

OmniObject::~OmniObject()
{

}

void OmniObject::execEx(const QString& k)
{
	if (!filter(k))
		return;

	exec(k);
}


OmniObject::RespondType OmniObject::respondType() const
{
	return m_respondType;
}

OmniObject::OmniType OmniObject::omniType() const
{
	return m_omniType;
}

OmniCommand::OmniCommand()
	: OmniObject(OmniObject::OmniType::Command, OmniObject::RespondType::Delay)
{

}

OmniCommand::~OmniCommand()
{

}

bool OmniCommand::filter(const QString& k)
{
	QString strKey = k.left(k.indexOf(" "));
	return strKey == ">";
}

void OmniCommand::exec(const QString& k)
{
	QString strContent = k.mid(k.indexOf(" ") + 1);
	QString s = "start cmd /k " + strContent;
	system(s.toStdString().c_str());
}

OmniSearchEngine::OmniSearchEngine()
	: QObject(Q_NULLPTR)
	, OmniObject(OmniObject::OmniType::Search, OmniObject::RespondType::Delay)
{

}

OmniSearchEngine::~OmniSearchEngine()
{

}

bool OmniSearchEngine::filter(const QString& k)
{
	auto it = m_searchEngineTable.find(k);
	return it != m_searchEngineTable.end();
}

void OmniSearchEngine::exec(const QString& k)
{
	QString strKey = k.left(k.indexOf(" "));
	QString strValue = k.mid(k.indexOf(" ") + 1);
	auto it = m_searchEngineTable.find(strKey);
	if (it != m_searchEngineTable.end())
	{
		QString strContent = it.value();
		QUrl u(strContent.replace("@", strValue));
		QString strUrl = u.url(QUrl::EncodeSpaces);
		QDesktopServices::openUrl(strUrl);
	}
}

void OmniSearchEngine::initSearchEngineTable()
{
	m_searchEngineTable.insert("bing", "https://www.bing.com/search?FORM=BDT1DF&PC=BDT1&q=@");
	m_searchEngineTable.insert("blbl", "http://www.bilibili.tv/search?keyword=@");
	m_searchEngineTable.insert("tb", "https://s.taobao.com/search?q=@");
	m_searchEngineTable.insert("db", "https://www.douban.com/search?q=@");
	m_searchEngineTable.insert("epub", "http://cn.epubee.com/books/?s=@");

	QDir d(qApp->applicationDirPath());
	bool bOk = d.cd("config");
	if (!bOk)
		d.mkdir("config");

	QFile fData(qApp->applicationDirPath() + "\\config\\searchenginemap.db");
	if (fData.open(QFile::ReadOnly))
	{
		while (true)
		{
			QString lineContent = fData.readLine();
			if (lineContent.split(" ").count() < 2)
				break;
			QString strKey = lineContent.split(" ").at(0);
			QString strUrl = lineContent.split(" ").at(1);
			if (!strKey.isEmpty() && !strUrl.isEmpty())
				m_searchEngineTable[strKey] = strUrl;
			else
			{
				fData.close();
				break;
			}
		}
	}
}

void OmniSearchEngine::firstInit()
{
	initSearchEngineTable();
}

OmniPlugin::OmniPlugin()
	: QObject(Q_NULLPTR)
	, OmniObject(OmniObject::OmniType::Plugin, OmniObject::RespondType::Real)
{
	m_pPluginManager = new PluginManager(this);
}

OmniPlugin::~OmniPlugin()
{

}

bool OmniPlugin::filter(const QString& k)
{
	QString strKey = k.left(k.indexOf(" "));
	QString strContent = k.mid(k.indexOf(" ") + 1);
	return m_pPluginManager->isPluginExist(strKey);
}

PluginManager* OmniPlugin::pluginManager() const
{
	return m_pPluginManager;
}

void OmniPlugin::exec(const QString& k)
{
	QString strKey = k.left(k.indexOf(" "));
	QString strContent = k.mid(k.indexOf(" ") + 1);
 	emit startPluginQuery(strKey, strContent);
}

OmniFile::OmniFile()
	: QObject(Q_NULLPTR)
	, OmniObject(OmniObject::OmniType::File, OmniObject::RespondType::Real)
{
	m_pMainDataSet = new MainDataSet(this);
	connect(this, &OmniFile::startSearchQuery, m_pMainDataSet, &MainDataSet::onStartQuery);
}

OmniFile::~OmniFile()
{

}

bool OmniFile::filter(const QString& k)
{
	Q_UNUSED(k);

	return true;
}

MainDataSet* OmniFile::mainDataSet() const
{
	return m_pMainDataSet;
}

void OmniFile::exec(const QString& k)
{
	emit startSearchQuery(k);
}
