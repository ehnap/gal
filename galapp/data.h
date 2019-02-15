#pragma once

#include <QObject>
#include <QString>
#include <QMap>
#include <QIcon>
#include <QMetaType>
#include <QQueue>

class QDir;

class Data
{
public:
	Data();
	Data(const QString& id, const QString& displayname, const QString& name, const QString& path, const QIcon& icon);
	Data(const Data& other);
	Data& operator = (const Data& other);
	~Data();

	QString name() const;
	QString displayName() const;
	QString path() const;
	QIcon icon() const;
	QString id() const;
	QString dirPath() const;

private:
    QString m_id;
	QString m_name;
	QString m_displayName;
	QString m_path;
	QIcon m_icon;
};

Q_DECLARE_METATYPE(Data);

typedef QQueue<Data> ResultQueue;

class QuickLaunchTable
{
public:
	QuickLaunchTable();
	~QuickLaunchTable();

	void init();

	ResultQueue queryResult(const QString& key);

private:
	void walkThroughDirHelper(QDir* d);

private:
	QMap<QString, Data> m_items;
};

class ResultListWidget;

class MainDataSet : public QObject
{

	Q_OBJECT

public:
	MainDataSet();
	~MainDataSet();

	void queryResult(const QString& key);
	bool takeData(Data& d, const QString& key);

public slots:
	void onStartQuery(const QString& content);

signals:
	void dataChanged(const QString& key);

protected slots:
	void firstInit();

private:
	QuickLaunchTable* m_pQLTable;
	QThread* m_workThread;
	ResultQueue m_resultQueue;
	QString m_resultKey;
};
