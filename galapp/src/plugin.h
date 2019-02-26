#pragma once

#include "include/galcppfreeinterface.h"

#include <QObject>
#include <QWidget>

class QJSEngine;
class QLabel;
class QMenu;
class PluginStackedWidget;

class Plugin : public QObject
{
public:
	enum PluginType
	{
		JS_SIMPLE,
		CPP_FREE,

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
	virtual QWidget* widget() const = 0; //查询异步支持时会用得上
	static PluginType getTypeFromStr(const QString& str);

private:
	QString m_name;
	QString m_version;
	QString m_key;
	QString m_author;
	QString m_dir;
	PluginType m_type;
};

class PluginWidget : public QWidget
{

	Q_OBJECT

public:
	PluginWidget(PluginStackedWidget* parent);
	virtual ~PluginWidget();

	PluginStackedWidget* stackedWidget() const;
	virtual void extend() = 0;

private:
	PluginStackedWidget* m_stackedWidget;
};

class LabelPluginWidget : public PluginWidget
{

	Q_OBJECT

public:
	LabelPluginWidget(PluginStackedWidget* parent);

	void setText(const QString& t);
	void extend() override;

private slots:
	void copyContent();

private:
	QLabel* m_pResultLabel;
	PluginStackedWidget* m_stackedWidget;
	QMenu* m_pMenu;
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
	QWidget* widget() const override;

private:
	LabelPluginWidget* m_widget;
	QJSEngine* m_jsEngine;
};

class CppFreePlugin : public Plugin
{

	Q_OBJECT

public:
	CppFreePlugin(QObject* parent,
		const QString& pluName,
		const QString& pluKey,
		const QString& pluVer,
		const QString& pluAuthor,
		const QString& pluDir);

	void exec(const QString& content, QWidget* canvas) override;
	QWidget* widget() const override;

private slots:
	void firstInit();

private:
	QWidget* m_widget;
	QSharedPointer<CppFreeInterface> m_interface;
};

class FreeWidget : public PluginWidget
{

	Q_OBJECT

public:
	FreeWidget(PluginStackedWidget* parent);

	void setHost(QWeakPointer<CppFreeInterface> cp);

	void extend() override;
protected:
	void paintEvent(QPaintEvent* e) override;

private:
	QWeakPointer<CppFreeInterface> m_host;
};