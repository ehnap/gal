#include "plugin.h"
#include <QJSEngine>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QLabel>
#include <QVBoxLayout>

Plugin::Plugin(QObject* parent, const QString& pluName, const QString& pluKey, const QString& pluVer, const QString& pluAuthor, const QString& pluDir, PluginType t)
	: QObject(parent)
	, m_key(pluKey)
	, m_name(pluName)
	, m_version(pluVer)
	, m_author(pluAuthor)
	, m_dir(pluDir)
	, m_type(t)
{
}

Plugin::~Plugin()
{
}

Plugin::PluginType Plugin::type() const
{
	return m_type;
}

QString Plugin::name() const
{
	return m_name;
}

QString Plugin::version() const
{
	return m_version;
}

QString Plugin::key() const
{
	return m_key;
}

QString Plugin::dir() const
{
	return m_dir;
}

QString Plugin::author() const
{
	return m_author;
}

Plugin::PluginType Plugin::getTypeFromStr(const QString& str)
{
	if (str.compare("JS_SIMPLE",Qt::CaseInsensitive) == 0)
		return JS_SIMPLE;

	return UNKNOWN_TYPE;
}

JsSimplePlugin::JsSimplePlugin(QObject* parent, const QString& pluName, const QString& pluKey, const QString& pluVer, const QString& pluAuthor, const QString& pluDir)
	: Plugin(parent, pluName, pluKey, pluVer, pluAuthor, pluDir, Plugin::JS_SIMPLE)
	, m_widget(Q_NULLPTR)
{
	m_jsEngine = new QJSEngine();
}

void JsSimplePlugin::exec(const QString& content, QWidget* canvas)
{
	if (!canvas)
		return;

	QString fileName = QDir::toNativeSeparators(dir() + "\\" + "main.js");
	QFile scriptFile(fileName);
	if (!scriptFile.open(QIODevice::ReadOnly))
		return;
	QTextStream stream(&scriptFile);
	QString strFuncContents = stream.readAll();
	scriptFile.close();
	QJSValue fun = m_jsEngine->evaluate(strFuncContents);
	QJSValueList args;
	args << content;
	QJSValue funcResult = fun.call(args);
	QString strResult = funcResult.toString();
	JsSimplePluginWidget* pWidget = qobject_cast<JsSimplePluginWidget*>(canvas);
	if (pWidget)
	{
		pWidget->setText(strResult);
	}
}

QWidget* JsSimplePlugin::createWidget(QWidget* parent)
{
	if (!m_widget)
		return m_widget;

	m_widget = new JsSimplePluginWidget(parent);
	return m_widget;
}

QWidget* JsSimplePlugin::widget() const
{
	return m_widget;
}

JsSimplePluginWidget::JsSimplePluginWidget(QWidget* parent)
	: QWidget(parent)
{
	m_pResultLabel = new QLabel(this);
	m_pResultLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
	QVBoxLayout* pMainLayout = new QVBoxLayout(this);
	pMainLayout->addWidget(m_pResultLabel);
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
}

void JsSimplePluginWidget::setText(const QString& t)
{
	m_pResultLabel->setText(t);
}
