#pragma once

#include <QObject>
#include <QString>
#include <QMap>
#include <QIcon>
#include <QMetaType>

class QDir;

class Data
{
public:
	Data();
	Data(const QString& id, const QString& name, const QString& path, const QIcon& icon);
	~Data();

	QString name() const;
	QString path() const;
	QIcon icon() const;
	QString id() const;

private:
    QString m_id;
	QString m_name;
	QString m_path;
	QIcon m_icon;
};

Q_DECLARE_METATYPE(Data);

typedef QList<Data> ResultSet;

class QuickLaunchTable
{
public:
	QuickLaunchTable();
	~QuickLaunchTable();

	void init();

	ResultSet queryResult(const QString& key);

private:
	void walkThroughDirHelper(QDir* d);
	QString getId(const QString& name);

private:
	QMap<QString, Data> m_items;
};

class MainDataSet : public QObject
{

	Q_OBJECT

public:
	MainDataSet();
	~MainDataSet();

	ResultSet queryResult(const QString& key);

protected slots:
	void init();

private:
	QuickLaunchTable* m_pQLTable;
};
