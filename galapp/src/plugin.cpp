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
	if (str.compare("CPP_SIMPLELIST", Qt::CaseInsensitive) == 0)
		return PluginType::CPP_SIMPLELIST;

	return PluginType::UNKNOWN_TYPE;
}

JsSimplePlugin::JsSimplePlugin(QObject* parent, const QString& pluName, const QString& pluKey, const QString& pluVer, const QString& pluAuthor, const QString& pluDir)
	: Plugin(parent, pluName, pluKey, pluVer, pluAuthor, pluDir, Plugin::PluginType::JS_SIMPLE)
	, m_widget(Q_NULLPTR)
{
	m_jsEngine = new QJSEngine();
}

void JsSimplePlugin::query(const QString& content, QWidget* canvas)
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

LabelPluginWidget::LabelPluginWidget(PluginStackedWidget* parent)
	: m_stackedWidget(parent)
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
	m_stackedWidget->parentWidget()->adjustSize();
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

CppFreePlugin::CppFreePlugin(QObject* parent, const QString& pluName, const QString& pluKey, const QString& pluVer, const QString& pluAuthor, const QString& pluDir)
	: Plugin(parent, pluName, pluKey, pluVer, pluAuthor, pluDir, Plugin::PluginType::CPP_FREE)
	, m_widget(Q_NULLPTR)
{
	QTimer::singleShot(0, this, &CppFreePlugin::firstInit);
}

void CppFreePlugin::query(const QString& content, QWidget* canvas)
{
	if (!canvas)
		return;

	FreeWidget* pWidget = qobject_cast<FreeWidget*>(canvas); 
	if (pWidget)
		pWidget->setHost(m_interface);

	if (!m_interface.isNull())
		m_interface->query(content);

	pWidget->update();
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
	: QWidget(parent)
	, m_host(Q_NULLPTR)
	, m_heightHint(-1)
{

}

void FreeWidget::setHost(QWeakPointer<CppFreeInterface> cp)
{
	m_host = cp;
	CppFreeInterface* pInterface = m_host.data();
	if (!pInterface)
		m_heightHint = -1;
	else
		m_heightHint = pInterface->heightHint();
}

void FreeWidget::extend()
{
	if (m_host.isNull())
		return;

	CppFreeInterface* pInterface = m_host.data();
	if (!pInterface)
		return;

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


QSize FreeWidget::sizeHint() const
{
	QSize sz = QWidget::sizeHint();
	if (m_heightHint > -1)
		sz.setHeight(m_heightHint);

	return sz;
}

CppSimpleListWidget::CppSimpleListWidget(PluginStackedWidget* parent)
	: GalListWidget(parent)
{

}


void CppSimpleListWidget::setHost(QWeakPointer<CppSimpleListInterface> cp)
{
	m_host = cp;
}


void CppSimpleListWidget::next()
{
	int r = currentRow();
	++r;
	r = qMin(count() - 1, r);
	setCurrentRow(r);
}


void CppSimpleListWidget::prev()
{
	int r = currentRow();
	--r;
	r = qMax(0, r);
	setCurrentRow(r);
}

void CppSimpleListWidget::shot()
{
	if (m_host.isNull())
		return;
	
	CppSimpleListInterface* pInterface = m_host.data();
	if (!pInterface)
		return;

	ListItem it;
	GalListItem* pItem = dynamic_cast<GalListItem*>(currentItem());
	if (!pItem)
		return;

	it.content = pItem->content();
	it.icon = pItem->icon();
	it.title = pItem->title();
	pInterface->exec(it);
}


void CppSimpleListWidget::clear()
{
	hide();
	GalListWidget::clear();
}

CppSimpleListPlugin::CppSimpleListPlugin(QObject* parent, const QString& pluName, const QString& pluKey, const QString& pluVer, const QString& pluAuthor, const QString& pluDir)
	: Plugin(parent, pluName, pluKey, pluVer, pluAuthor, pluDir, Plugin::PluginType::CPP_SIMPLELIST)
{
	QTimer::singleShot(0, this, &CppSimpleListPlugin::firstInit);
}

void CppSimpleListPlugin::query(const QString& content, QWidget* canvas)
{
	if (!canvas)
		return;

	CppSimpleListWidget* pWidget = qobject_cast<CppSimpleListWidget*>(canvas);
	if (!pWidget)
		return;

	if (m_interface.isNull())
		return;

	pWidget->setHost(m_interface);

	auto itemList = m_interface->query(content);
	pWidget->clear();
	foreach(auto item, itemList)
	{
		GalListItem* pItem = new GalListItem(pWidget);
		pItem->setTitle(item.title);
		pItem->setIcon(item.icon);
		pItem->setContent(item.content);
		pWidget->addItem(pItem);
	}
	if (!itemList.isEmpty())
		pWidget->show();
}

void CppSimpleListPlugin::firstInit()
{
	QString fileName = QDir::toNativeSeparators(dir() + "\\" + name() + ".dll");
	QPluginLoader pluginLoader(fileName);
	QObject* pObject = pluginLoader.instance();
	if (pObject) {
		CppSimpleListInterface* pInterface = qobject_cast<CppSimpleListInterface*>(pObject);
		if (pInterface)
			m_interface = QSharedPointer<CppSimpleListInterface>(pInterface);
	}
}

JsSimpleListPlugin::JsSimpleListPlugin(QObject* parent, const QString& pluName, const QString& pluKey, const QString& pluVer, const QString& pluAuthor, const QString& pluDir)
	: Plugin(parent, pluName, pluKey, pluVer, pluAuthor, pluDir, Plugin::PluginType::CPP_SIMPLELIST)
{
	m_jsEngine = new QJSEngine();
	QJSValue jsMetaObject = m_jsEngine->newQMetaObject(&JsGalObject::staticMetaObject);
	m_jsEngine->globalObject().setProperty("GalApp", jsMetaObject);
}

void JsSimpleListPlugin::query(const QString& content, QWidget* canvas)
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
	m_jsEngine->evaluate(strFuncContents);
}


JsGalObject::JsGalObject(QObject* parent)
	: QObject(parent)
{
	m_pNetManager = new QNetworkAccessManager(this);
	connect(m_pNetManager, &QNetworkAccessManager::finished,
		this, &JsGalObject::replyFinished);

}

JsGalObject::~JsGalObject()
{

}

Q_INVOKABLE int JsGalObject::httpGet(const QString& strUrl, const QString& jsCallBack)
{
	QNetworkReply* pReply = m_pNetManager->get(QNetworkRequest(QUrl(strUrl)));
	GalNetReply* pGalReply = new GalNetReply(this);
	pGalReply->setNetReply(pReply);

	return 0;
}

void JsGalObject::replyFinished(QNetworkReply* reply)
{
	if (reply->isFinished())
	{
		QByteArray ba = reply->readAll();

	}
}

GalNetReply::GalNetReply(QObject* parent)
	: QObject(parent)
{

}

GalNetReply::~GalNetReply()
{

}

void GalNetReply::setNetReply(QNetworkReply* pReply)
{
	m_pReply = pReply;
	connect(m_pReply, &QIODevice::readyRead, this, &GalNetReply::slotReadyRead);
	connect(m_pReply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
		this, &GalNetReply::slotError);
	connect(m_pReply, &QNetworkReply::sslErrors,
		this, &GalNetReply::slotSslErrors);
}


void GalNetReply::setJsCallback(const QString& jsFunc)
{
	m_jsCallback = jsFunc;
}

void GalNetReply::slotReadyRead()
{

}

void GalNetReply::slotError(QNetworkReply::NetworkError code)
{

}

void GalNetReply::slotSslErrors(const QList<QSslError> &errors)
{

}
