#include "galyd.h"
#include <QDesktopServices>
#include <QUrl>
#include <QEventLoop>
#include <QNetworkReply>
#include <QTimer>
#include <QSettings>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDir>
#include <QApplication>

GalYDPlugin::GalYDPlugin()
{
	m_pNetManager = new QNetworkAccessManager(this);
	QTimer::singleShot(0, this, &GalYDPlugin::firstInit);
}

GalYDPlugin::~GalYDPlugin()
{
}

QVector<ListItem> GalYDPlugin::query(const QString& val)
{
	if (m_ydkey.isEmpty() || m_ydkeyfrom.isEmpty())
		return QVector<ListItem>();

	if (val == m_cacheKey)
		return m_cacheResult;

	QString strKey = "key=" + m_ydkey;
	QString strKeyFrom = "keyfrom=" + m_ydkeyfrom;
	QString strUrl = "http://fanyi.youdao.com/openapi.do?" + 
		strKeyFrom + "&" + strKey + "&type=data&doctype=json&version=1.1&q=" + val;
	QNetworkRequest requestInfo;
 	requestInfo.setUrl(QUrl(strUrl));

	QEventLoop eventLoop;
	QNetworkReply* pReply = m_pNetManager->get(requestInfo);
	connect(pReply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
	eventLoop.exec();

	m_cacheResult.clear();
	m_cacheKey = val;

	if (pReply->error() == QNetworkReply::NoError)
	{
		QByteArray responseByte = pReply->readAll();
		auto jsonDoc = QJsonDocument::fromJson(responseByte);
		if (jsonDoc.isObject())
		{
			auto jsonObj = jsonDoc.object();
			auto translationArr = jsonObj.value("translation").toArray();
			ListItem transItem;
			transItem.title = "Translation";
			foreach(const QJsonValue& v, translationArr)
			{
				transItem.content += v.toString();
			}
			if (!transItem.title.isEmpty() && !transItem.content.isEmpty())
				m_cacheResult.append(transItem);

			auto basicValue = jsonObj.value("basic");
			ListItem basicItem;
			if (basicValue.isObject())
			{
				auto basicObj = basicValue.toObject();
				auto strPhonetic = basicObj.value("phonetic").toString();
				auto strUsPhonetic = basicObj.value("us-phonetic").toString();
				auto strUkPhonetic = basicObj.value("uk-phonetic").toString();
				if (!strPhonetic.isEmpty())
					basicItem.title += "[" + strPhonetic + "]; ";
				if (!strUsPhonetic.isEmpty())
					basicItem.title += "(us)[" + strUsPhonetic + "]; ";
				if (!strUkPhonetic.isEmpty())
					basicItem.title += "(uk)[" + strUkPhonetic + "]; ";
				auto explainArr = basicObj.value("explains").toArray();
				foreach(const QJsonValue& v, explainArr)
				{
					basicItem.content += v.toString() + "; ";
				}
			}
			if (!basicItem.title.isEmpty() && !basicItem.content.isEmpty())
				m_cacheResult.append(basicItem);

			auto webValue = jsonObj.value("web");
			if (webValue.isArray())
			{
				auto webArr = webValue.toArray();
				foreach(const QJsonValue& v, webArr)
				{
					if (v.isObject())
					{
						ListItem item;
						auto vObj = v.toObject();
						item.title = vObj.value("key").toString();
						auto vArrValue = vObj.value("value");
						if (vArrValue.isArray())
						{
							auto vArr = vArrValue.toArray();
							foreach(const QJsonValue& v, vArr)
							{
								item.content += v.toString() + "; ";
							}
							if (!item.title.isEmpty() && !item.content.isEmpty())
								m_cacheResult.append(item);
						}
					}
				}
			}
		}
		
	}

	delete pReply;

	return m_cacheResult;
}

void GalYDPlugin::exec(ListItem item)
{
	QDesktopServices::openUrl(QUrl(item.content));
}

void GalYDPlugin::firstInit()
{
	QString pluginsDir = QDir::toNativeSeparators(qApp->applicationDirPath() 
		+ "\\plugin\\galyd\\");
	QString configFilePath = pluginsDir + "ydconfig.ini";
	QSettings configSetting(configFilePath, QSettings::IniFormat);
	if (configSetting.status() == QSettings::NoError)
	{
		auto testlist = configSetting.allKeys();
		m_ydkey = configSetting.value("ydkey").toString();
		m_ydkeyfrom = configSetting.value("ydkeyfrom").toString();
	}
}
