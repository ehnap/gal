#pragma once

#include <QString>
#include <QHash>
#include <QObject>

class PluginManager;

class OmniObject
{
public:
	enum class RespondType
	{
		Real,
		Delay,
	};

	enum class OmniType
	{
		File,
		Search,
		Plugin,
		Command,
	};

	explicit OmniObject(OmniType ot, RespondType rt);
	virtual ~OmniObject();

	void execEx(const QString& k);
	OmniType omniType() const;
	RespondType respondType() const;

	virtual bool filter(const QString& k) = 0;

protected:
	virtual void exec(const QString& k) = 0;

private:
	OmniType m_omniType;
	RespondType m_respondType;
};

class OmniCommand : public OmniObject
{
public:
	OmniCommand();
	~OmniCommand();

	bool filter(const QString& k) override;

protected:
	void exec(const QString& k) override;
};

class OmniSearchEngine : public QObject, public OmniObject
{

	Q_OBJECT

public:
	OmniSearchEngine();
	~OmniSearchEngine();

	bool filter(const QString& k) override;

protected:
	void exec(const QString& k) override;

private:
	void initSearchEngineTable();

private slots:
	void firstInit();

private:
	QHash<QString, QString> m_searchEngineTable;
};

class OmniPlugin : public QObject, public OmniObject
{

	Q_OBJECT

public:
	OmniPlugin();
	~OmniPlugin();

	virtual bool filter(const QString& k) override;

	PluginManager* pluginManager() const;

signals:
	void startPluginQuery(const QString& key, const QString& value);

protected:
	void exec(const QString& k) override;

private:
	PluginManager* m_pPluginManager;
};

class MainDataSet;

class OmniFile : public QObject, public OmniObject
{

	Q_OBJECT

public:
	OmniFile();
	~OmniFile();

	bool filter(const QString& k) override;

	MainDataSet* mainDataSet() const;

signals:
	void startSearchQuery(const QString& key);

protected:
	void exec(const QString& k) override;

private:
	MainDataSet* m_pMainDataSet;
};