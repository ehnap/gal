#include "data.h"
#include "pydata.h"
#include "resultlist.h"
#include "Everything.h"
#include <QTimer>
#include <QDir>
#include <QThread>
#include <QFileInfo>
#include <QtWidgets/QFileIconProvider>
#include <QProcess>
#include <QMutex>

namespace {
	QMutex queryMutex;
}

Data::Data(const QString& id, 
	const QString& displayName,
	const QString& name,
	const QString& path, 
	const QIcon& icon)
	: m_id(id)
	, m_displayName(displayName)
	, m_name(name)
	, m_path(path)
	, m_icon(icon)
{

}

Data::Data()
	: m_id("")
	, m_displayName("")
	, m_name("")
	, m_path("")
	, m_icon(QIcon())
{

}

Data::Data(const Data& other)
{
	m_icon = other.m_icon;
	m_id = other.m_id;
	m_name = other.m_name;
	m_path = other.m_path;
	m_displayName = other.m_displayName;
}

Data& Data::operator=(const Data& other)
{
	m_path = other.m_path;
	m_id = other.m_id;
	m_name = other.m_name;
	m_icon = other.m_icon;
	m_displayName = other.m_displayName;
	return *this;
}

Data::~Data()
{
}

QString Data::name() const
{
	return m_name;
}

QString Data::displayName() const
{
	return m_displayName;
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

QString Data::dirPath() const
{
	return "";
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

ResultQueue QuickLaunchTable::queryResult(const QString& key)
{
	ResultQueue s;
	for (auto it = m_items.begin(); it != m_items.end(); it++)
	{
		QString k = it.key();
		Data v = it.value();
		if (k.contains(key, Qt::CaseInsensitive) || 
			v.displayName().contains(key, Qt::CaseInsensitive) ||
			v.name().contains(key, Qt::CaseInsensitive))
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
			QString id = PyData::GetInstance().getPy(info.baseName());
			Data newData(id, info.baseName(), f.fileName(), f.absolutePath(), p.icon(f));
			m_items.insert(id, newData);
		}
	}
}

MainDataSet::MainDataSet()
	: m_pQLTable(new QuickLaunchTable())
{
	QTimer::singleShot(0, this, &MainDataSet::firstInit);
}

MainDataSet::~MainDataSet()
{

}

void MainDataSet::queryResult(const QString& key)
{
	queryMutex.lock();
	m_resultKey = key;
	m_resultQueue.clear();
	if (!key.isEmpty())
		m_resultQueue.append(m_pQLTable->queryResult(key));
	queryMutex.unlock();

	emit dataChanged(key);

	if (key.isEmpty())
		return;

	DWORD i;
	std::wstring s(key.toStdWString());
	Everything_SetSearch(s.c_str());
	Everything_SetMax(6 * 15);
	Everything_Query(TRUE);

	QFileIconProvider p;
	for (i = 0; i < Everything_GetNumResults(); i++)
	{
		QString strPath = QString::fromStdWString(Everything_GetResultPath(i));
		QString strName = QString::fromStdWString(Everything_GetResultFileName(i));
		QString strFullPath = QDir::toNativeSeparators(strPath + "/" + strName);
		QString strId = PyData::GetInstance().getPy(strName);
		QFileInfo f(strFullPath);
		Data newData(strId, strName, strName, strPath, p.icon(f));
		queryMutex.lock();
		m_resultQueue.append(newData);
		queryMutex.unlock();
		if (i % 6 == 0)
			emit dataChanged(key);
	}

	emit dataChanged(key);
}

bool MainDataSet::takeData(Data& d, const QString& key)
{
	bool r = false;
	if (key != m_resultKey)
		return false;

	queryMutex.lock();
	if (!m_resultQueue.isEmpty())
	{
		d = m_resultQueue.dequeue();
		r = true;
	}
	queryMutex.unlock();

	return r;
}

void MainDataSet::onStartQuery(const QString& content)
{
	queryResult(content);
}

void MainDataSet::firstInit()
{
	m_workThread = new QThread(this);
	moveToThread(m_workThread);
	m_workThread->start();

	m_pQLTable->init();
}
