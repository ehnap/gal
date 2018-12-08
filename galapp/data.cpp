#include "data.h"
#include "pydata.h"
#include <QTimer>
#include <QDir>
#include <QFileInfo>
#include <QtWidgets/QFileIconProvider>
#include "QProcess"

Data::Data(const QString& id, 
	const QString& name, 
	const QString& path, 
	const QIcon& icon)
	: m_id(id)
	, m_name(name)
	, m_path(path)
	, m_icon(icon)
{

}

Data::Data()
	: m_id("")
	, m_name("")
	, m_path("")
	, m_icon("")
{

}

Data::~Data()
{
}

QString Data::name() const
{
	return m_name;
}

QString Data::path() const
{
	return m_path;
}

QIcon Data::icon() const
{
	return m_icon;
}

QString Data::id() const
{
	return m_id;
}

QuickLaunchTable::QuickLaunchTable()
{

}

QuickLaunchTable::~QuickLaunchTable()
{

}

void QuickLaunchTable::init()
{
	// C:\ProgramData\Microsoft\Windows\Start Menu\Programs
	QString userStartMenuDir = QDir::toNativeSeparators(QProcessEnvironment::systemEnvironment().value("APPDATA") + "/" + "Microsoft\\Windows\\Start Menu\\Programs");
	QString AllStartMenuDir = QDir::toNativeSeparators(("C:\\ProgramData\\Microsoft\\Windows\\Start Menu\\Programs"));
	QStringList dirList;
	dirList << userStartMenuDir << AllStartMenuDir;
	QDir curDir;
	foreach (const QString& d, dirList)
	{
		curDir.setPath(d);
		walkThroughDirHelper(&curDir);
	}
	
}

ResultSet QuickLaunchTable::queryResult(const QString& key)
{
	ResultSet s;
	for (auto it = m_items.begin(); it != m_items.end(); it++)
	{
		QString k = it.key();
		Data v = it.value();
		if (k.contains(key, Qt::CaseInsensitive))
			s << v;
	}
	
	return s;
}

void QuickLaunchTable::walkThroughDirHelper(QDir* d)
{
	QFileInfoList l = d->entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
	QFileIconProvider p;
	foreach(const QFileInfo& info, l)
	{
		if (info.isDir())
		{
			QDir newDir(info.filePath());
			walkThroughDirHelper(&newDir);
		}
		else
		{
			QString path = info.filePath();
			if (info.isSymLink())
				path = info.symLinkTarget();
			QFileInfo f(path);
			QString id = getId(info.baseName());
			Data newData(id, info.baseName(), path, p.icon(f));
			m_items.insert(id, newData);
		}
	}
}

QString QuickLaunchTable::getId(const QString& name)
{
	QString nameID = name;
	nameID.replace(" ", QString());
	QString resultId;
	for (int i = 0; i < nameID.length(); ++i)
	{
		ushort uni = nameID[i].unicode();
		if (uni >= 0x4E00 && uni <= 0x9FA5)
		{
			QStringList l = PyData::GetInstance().queryPy(nameID[i]);
			QStringList resultList;
			for (int i = 0; i < l.count(); i++)
			{
				QStringList tempList = resultId.split("|");
				for (int j = 0; j < tempList.count(); j++)
				{
					tempList[j] += l[i];
				}
				resultList += tempList;
			}
			resultId = resultList.join("|");
		}
		else
		{
			if (resultId.contains("|"))
				resultId = resultId.replace("|", nameID[i] + QString("|"));
			else
				resultId += nameID[i];
		}
	}
	return resultId;
}

MainDataSet::MainDataSet()
	: m_pQLTable(new QuickLaunchTable())
{
	QTimer::singleShot(0, this, &MainDataSet::init);
}

MainDataSet::~MainDataSet()
{

}

ResultSet MainDataSet::queryResult(const QString& key)
{
	if (key.isEmpty())
		return ResultSet();
	return m_pQLTable->queryResult(key);
}

void MainDataSet::init()
{
	m_pQLTable->init();
}
