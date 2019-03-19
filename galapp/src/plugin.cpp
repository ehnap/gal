#include "plugin.h"
#include "pluginmanager.h"

#include <QJSEngine>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QLabel>
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QMenu>
#include <QApplication>
#include <QClipboard>
#include <QPainter>
#include <QTimer>
#include <QPluginLoader>

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
		return PluginType::JS_SIMPLE;
	if (str.compare("CPP_FREE", Qt::CaseInsensitive) == 0)
		return PluginType::CPP_FREE;

	return PluginType::UNKNOWN_TYPE;
}

JsSimplePlugin::JsSimplePlugin(QObject* parent, const QString& pluName, const QString& pluKey, const QString& pluVer, const QString& pluAuthor, const QString& pluDir)
	: Plugin(parent, pluName, pluKey, pluVer, pluAuthor, pluDir, Plugin::PluginType::JS_SIMPLE)
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
	LabelPluginWidget* pWidget = qobject_cast<LabelPluginWidget*>(canvas);
	if (pWidget)
	{
		pWidget->setText(strResult);
	}
}

QWidget* JsSimplePlugin::widget() const
{
	return m_widget;
}

LabelPluginWidget::LabelPluginWidget(PluginStackedWidget* parent)
	: PluginWidget(parent)
	, m_stackedWidget(parent)
{
	m_pResultLabel = new QLabel(this);
	QFont f = m_pResultLabel->font();
	f.setBold(false);
	f.setPixelSize(16);
	QPalette labelPalette = m_pResultLabel->palette();
	labelPalette.setColor(QPalette::Foreground, 0xd9d9d4);
	m_pResultLabel->setPalette(labelPalette);
	m_pResultLabel->setFont(f);
	m_pResultLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
	QVBoxLayout* pMainLayout = new QVBoxLayout(this);
	pMainLayout->setSizeConstraint(QLayout::SetMinimumSize);
	pMainLayout->addWidget(m_pResultLabel);
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	m_pResultLabel->setAlignment(Qt::AlignTop);

	m_pMenu = new QMenu(this);
	auto pCp = new QAction(QObject::tr("&Copy Content"), m_pMenu);
	QObject::connect(pCp, &QAction::triggered, this, &LabelPluginWidget::copyContent);
	m_pMenu->addAction(pCp);
}

void LabelPluginWidget::setText(const QString& t)
{
	m_pResultLabel->setText(t);
	stackedWidget()->parentWidget()->adjustSize();
}

void LabelPluginWidget::extend()
{
	QRect rc = rect();
	QPoint lt = mapToGlobal(QPoint(0, 0));
	m_pMenu->move(lt.x() + rc.x() + rc.width() / 2, lt.y() + rc.y() + rc.height() / 2);
	m_pMenu->show();
}

void LabelPluginWidget::copyContent()
{
	QClipboard* pClipboard = QApplication::clipboard();
	pClipboard->setText(m_pResultLabel->text());
}

PluginWidget::PluginWidget(PluginStackedWidget* parent)
	: QWidget(parent)
	, m_stackedWidget(parent)
{
}

PluginWidget::~PluginWidget()
{

}

PluginStackedWidget* PluginWidget::stackedWidget() const
{
	return m_stackedWidget;
}

CppFreePlugin::CppFreePlugin(QObject* parent, const QString& pluName, const QString& pluKey, const QString& pluVer, const QString& pluAuthor, const QString& pluDir)
	: Plugin(parent, pluName, pluKey, pluVer, pluAuthor, pluDir, Plugin::PluginType::CPP_FREE)
	, m_widget(Q_NULLPTR)
{
	QTimer::singleShot(0, this, &CppFreePlugin::firstInit);
}

void CppFreePlugin::exec(const QString& content, QWidget* canvas)
{
	if (!canvas)
		return;

	FreeWidget* pWidget = qobject_cast<FreeWidget*>(canvas); 
	if (pWidget)
		pWidget->setHost(m_interface);

	if (!m_interface.isNull())
		m_interface->exec(content);

	pWidget->update();
}

QWidget* CppFreePlugin::widget() const
{
	return m_widget;
}

void CppFreePlugin::firstInit()
{
	QString fileName = QDir::toNativeSeparators(dir() + "\\" + name() + ".dll");
	QPluginLoader pluginLoader(fileName);
	QObject* pObject = pluginLoader.instance();
	if (pObject) {
		CppFreeInterface* pInterface = qobject_cast<CppFreeInterface*>(pObject);
		if (pInterface)
			m_interface = QSharedPointer<CppFreeInterface>(pInterface);
	}
}

FreeWidget::FreeWidget(PluginStackedWidget* parent)
	: PluginWidget(parent)
	, m_host(Q_NULLPTR)
{

}

void FreeWidget::setHost(QWeakPointer<CppFreeInterface> cp)
{
	m_host = cp;
}

void FreeWidget::extend()
{
	if (m_host.isNull())
		return;

	CppFreeInterface* pInterface = m_host.data();
	QMenu* pMenu = pInterface->extentMenu();
	if (pMenu)
	{
		QRect rc = rect();
		QPoint lt = mapToGlobal(QPoint(0, 0));
		pMenu->move(lt.x() + rc.x() + rc.width() / 2, lt.y() + rc.y() + rc.height() / 2);
		pMenu->show();
	}
}

void FreeWidget::paintEvent(QPaintEvent* e)
{
	QPainter p(this);

	if (!m_host.isNull())
	{
		CppFreeInterface* pInterface = m_host.data();
		if (pInterface)
			pInterface->paint(&p, rect());
	}

	QWidget::paintEvent(e);
}
