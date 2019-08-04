#pragma once

#include "include/galcppfreeinterface.h"
#include "include/galcpplistinterface.h"
#include "gallistwidget.h"
#include "omniobject.h"

#include <QObject>
#include <QWidget>

class QJSEngine;
class QLabel;
class QMenu;
class PluginStackedWidget;

class Plugin : public QObject
{
public:
	enum class PluginType
	{
		JS_SIMPLE,
		CPP_FREE,
		CPP_SIMPLELIST,

		UNKNOWN_TYPE,
	};

	Plugin(QObject* parent, 
		const QString& pluName, 
		const QString& pluKey, 
		const QString& pluVer, 
		const QString& pluAuthor,
		const QString& pluDir, 
		PluginType t,
		OmniObject::RespondType rt = OmniObject::RespondType::Real);
	virtual ~Plugin();

	OmniObject::RespondType respondType() const;
	PluginType type() const;
	QString name() const;
	QString version() const;
	QString key() const;
	QString dir() const;
	QString author() const;
	virtual void query(const QString& content, QWidget* canvas) = 0; //查询动作
	virtual void exec(ListItem it) { Q_UNUSED(it); }
	virtual QWidget* widget() const { return Q_NULLPTR; } //查询异步支持时会用得上
	static PluginType getTypeFromStr(const QString& str);

private:
	QString m_name;
	QString m_version;
	QString m_key;
	QString m_author;
	QString m_dir;
	PluginType m_type;
	OmniObject::RespondType m_respondType;
};

class PluginWidgetInterface
{
public:
	virtual void next() {}
	virtual void prev() {}
	virtual void shot() {}
	virtual void clear() {}
	virtual void extend() {}
};

class LabelPluginWidget : public QWidget, public PluginWidgetInterface
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

class FreeWidget : public QWidget, public PluginWidgetInterface
{

	Q_OBJECT

public:
	FreeWidget(PluginStackedWidget* parent);

	void setHost(QWeakPointer<CppFreeInterface> cp);
	void extend() override;

protected:
	void paintEvent(QPaintEvent* e) override;
	QSize sizeHint() const override;

private:
	QWeakPointer<CppFreeInterface> m_host;
	int m_heightHint;
};

class CppSimpleListWidget : public GalListWidget, public PluginWidgetInterface
{

	Q_OBJECT

public:
	CppSimpleListWidget(PluginStackedWidget* parent);

	void setHost(QWeakPointer<CppSimpleListInterface> cp);

	virtual void next() override;
	virtual void prev() override;
	virtual void shot() override;
	virtual void clear() override;

private:
	QWeakPointer<CppSimpleListInterface> m_host;
};

class JsSimplePlugin : public Plugin
{
public:
	JsSimplePlugin(QObject* parent,
		const QString& pluName,
		const QString& pluKey,
		const QString& pluVer,
		const QString& pluAuthor,
		const QString& pluDir,
		OmniObject::RespondType rt = OmniObject::RespondType::Real);

	void query(const QString& content, QWidget* canvas) override;

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
		const QString& pluDir,
		OmniObject::RespondType rt = OmniObject::RespondType::Real);

	void query(const QString& content, QWidget* canvas) override;

private slots:
	void firstInit();

private:
	QWidget* m_widget;
	QSharedPointer<CppFreeInterface> m_interface;
};

class CppSimpleListPlugin : public Plugin
{

	Q_OBJECT

public:
	CppSimpleListPlugin(QObject* parent,
		const QString& pluName,
		const QString& pluKey,
		const QString& pluVer,
		const QString& pluAuthor,
		const QString& pluDir,
		OmniObject::RespondType rt = OmniObject::RespondType::Real);

	void query(const QString& content, QWidget* canvas) override;

private slots:
	void firstInit();

private:
	QSharedPointer<CppSimpleListInterface> m_interface;
};