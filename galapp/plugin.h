#pragma once

#include <QObject>
#include <QWidget>

class QJSEngine;
class QLabel;

class Plugin : public QObject
{
public:
	enum PluginType
	{
		JS_SIMPLE,


		UNKNOWN_TYPE,
	};

	Plugin(QObject* parent, 
		const QString& pluName, 
		const QString& pluKey, 
		const QString& pluVer, 
		const QString& pluAuthor,
		const QString& pluDir, 
		PluginType t);
	virtual ~Plugin();

	PluginType type() const;
	QString name() const;
	QString version() const;
	QString key() const;
	QString dir() const;
	QString author() const;
	virtual void exec(const QString& content, QWidget* canvas) = 0;

	virtual QWidget* createWidget(QWidget* parent) = 0;
	virtual QWidget* widget() const = 0;

	static PluginType getTypeFromStr(const QString& str);

private:
	QString m_name;
	QString m_version;
	QString m_key;
	QString m_author;
	QString m_dir;
	PluginType m_type;
};

class JsSimplePluginWidget : public QWidget
{

	Q_OBJECT

public:
	JsSimplePluginWidget(QWidget* parent);

	void setText(const QString& t);

private:
	QLabel* m_pResultLabel;
};

class JsSimplePlugin : public Plugin
{
public:
	JsSimplePlugin(QObject* parent, 
		const QString& pluName, 
		const QString& pluKey, 
		const QString& pluVer, 
		const QString& pluAuthor,
		const QString& pluDir);

	void exec(const QString& content, QWidget* canvas) override;
	QWidget* createWidget(QWidget* parent) override;
	QWidget* widget() const override;

private:
	JsSimplePluginWidget* m_widget;
	QJSEngine* m_jsEngine;
};
