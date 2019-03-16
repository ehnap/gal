#include "mainbox.h"
#include "data.h"
#include "resultlist.h"
#include "pluginmanager.h"
#include <QKeyEvent>
#include <QLineEdit>
#include <QListWidget>
#include <QVBoxLayout>
#include <QTimer>
#include <QScreen>
#include <QGuiApplication>
#include <QDir>
#include <QUrl>
#include <QDesktopServices>

#include <windows.h>

Mainbox::Mainbox(QWidget* parent/*= Q_NULLPTR*/)
	: QWidget(parent, Qt::FramelessWindowHint)
	, m_bDrag(false)
	, m_currentState(OmniState::File)
{
	setAutoFillBackground(true);
	QPalette p = palette();
	p.setColor(QPalette::Background, 0x424242);
	setPalette(p);

	QVBoxLayout* pMainlayout = new QVBoxLayout(this);
	m_pInputEdit = new QLineEdit(this);
	setFocusProxy(m_pInputEdit);
	m_pInputEdit->setFixedSize(1024, 72);
	QPalette editorPalette = m_pInputEdit->palette();
	editorPalette.setColor(QPalette::Base, 0x616161);
	editorPalette.setColor(QPalette::Text, 0xe3e0e3);
	m_pInputEdit->setPalette(editorPalette);
	m_pInputEdit->setFrame(false);

	QFont f = font();
	f.setFamily("Microsoft YaHei");
	f.setPixelSize(36);
	m_pInputEdit->setFont(f);
	pMainlayout->addWidget(m_pInputEdit);

	m_pPluginWidget = new PluginStackedWidget(this);
	pMainlayout->addWidget(m_pPluginWidget);
	m_pPluginWidget->hide();

	m_pPluginManager = new PluginManager(this);
	m_pPluginManager->setStackedWidget(m_pPluginWidget);
	connect(this, &Mainbox::startPluginQuery, m_pPluginManager, &PluginManager::onStartQuery);
	
	m_pItemList = new ResultListWidget(this);
	pMainlayout->addWidget(m_pItemList);

	m_pMainDataSet = new MainDataSet();
	connect(m_pMainDataSet, &MainDataSet::dataChanged, m_pItemList, &ResultListWidget::onDataChanged);
	connect(this, &Mainbox::startSearchQuery, m_pMainDataSet, &MainDataSet::onStartQuery);

	m_pItemList->setMainDataSet(m_pMainDataSet);
	connect(m_pInputEdit, &QLineEdit::textEdited, this, &Mainbox::textEdited);

	QTimer::singleShot(0, this, &Mainbox::firstInit);
}

Mainbox::~Mainbox()
{
	::UnregisterHotKey((HWND)winId(), 0x0923);
}

QString Mainbox::queryKey() const
{
	return m_pInputEdit->text();
}

void Mainbox::popUp()
{
	show();
	HWND hWnd = (HWND)winId();
	::SetForegroundWindow(hWnd);
}

bool Mainbox::searchEngineKeyFilter(const QString& key)
{
	auto it = m_searchEngineTable.find(key);
	return it != m_searchEngineTable.end();
}

bool Mainbox::execSearchEngine(const QString& key, const QString& value)
{
	auto it = m_searchEngineTable.find(key);
	if (it != m_searchEngineTable.end())
	{
		QString strContent = it.value();
		QUrl u(strContent.replace("@", value));
		QString strUrl = u.url(QUrl::EncodeSpaces);
		QDesktopServices::openUrl(strUrl);
		return true;
	}
	return false;
}

void Mainbox::textEdited(const QString& t)
{
	m_pItemList->clear();
	m_pPluginWidget->hide();
	adjustSize();
	m_currentState = OmniState::File;

	processInputWord(t);
}

void Mainbox::firstInit()
{
	auto pcScreen = QGuiApplication::primaryScreen();
	QRect r = pcScreen->availableGeometry();
	QSize s = size();
	move((r.width() - s.width()) / 2, (r.height() - s.height()) / 4);
	
	::RegisterHotKey((HWND)winId(), 0x0923, MOD_ALT, VK_SPACE);

	initSearchEngineTable();
}

bool Mainbox::event(QEvent* e)
{
	if (e->type() == QEvent::WindowDeactivate)
	{
		m_pItemList->clear();
		m_pInputEdit->clear();
		m_pPluginWidget->hide();
		adjustSize();
		hide();
	}

	if (e->type() == QEvent::ApplicationStateChange)
	{
		if (qApp->applicationState() != Qt::ApplicationActive)
		{
			m_pItemList->clear();
			m_pInputEdit->clear();
			m_pPluginWidget->hide();
			adjustSize();
			hide();
		}
	}

	return QWidget::event(e);
}

void Mainbox::keyPressEvent(QKeyEvent* e)
{
	if (e->key() == Qt::Key_Down)
	{
		m_pItemList->next();
	}

	if (e->key() == Qt::Key_Up)
	{
		m_pItemList->prev();
	}

	if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter)
	{
		switch (m_currentState)
		{
		case Mainbox::OmniState::Search:
			execSearchEngine(m_searchKey, m_searchContent);
			break;
		case Mainbox::OmniState::File:
			m_pItemList->shot();
			m_pItemList->clear();
			m_pInputEdit->clear();
			adjustSize();
			hide();
			break;
		case Mainbox::OmniState::Command:
			executeCommand();
			break;
		case Mainbox::OmniState::Plugin:
			break;
		default:
			break;
		}	
	}

	if (e->key() == Qt::Key_Right && e->modifiers() & Qt::AltModifier)
	{
		m_pItemList->extend();
		m_pPluginWidget->extend();
	}

	if (e->key() == Qt::Key_Escape)
	{
		m_pItemList->clear();
		m_pInputEdit->clear();
		m_pPluginWidget->hide();
		adjustSize();
		hide();
	}

	QWidget::keyPressEvent(e);
}

void Mainbox::mousePressEvent(QMouseEvent* e)
{
	m_movablePoint = e->globalPos();
	m_lastTopLeft = geometry().topLeft();
	m_bDrag = true;

	QWidget::mousePressEvent(e);
}

void Mainbox::mouseMoveEvent(QMouseEvent* e)
{
	if (m_bDrag)
	{	
		auto p = e->globalPos();
		int p_x = m_lastTopLeft.x() + p.x() - m_movablePoint.x();
		int p_y = m_lastTopLeft.y() + p.y() - m_movablePoint.y();
		move(p_x, p_y);
	}

	QWidget::mouseMoveEvent(e);
}

void Mainbox::mouseReleaseEvent(QMouseEvent* e)
{
	m_bDrag = false;

	QWidget::mouseReleaseEvent(e);
}

bool Mainbox::nativeEvent(const QByteArray& eventType, void* message, long* result)
{
	MSG* msg = (MSG*)message;
	if (msg->message == WM_HOTKEY)
		popUp();

	return QWidget::nativeEvent(eventType, message, result);
}

void Mainbox::initSearchEngineTable()
{
	m_searchEngineTable.insert("bing","https://www.bing.com/search?FORM=BDT1DF&PC=BDT1&q=@");
	m_searchEngineTable.insert("blbl", "http://www.bilibili.tv/search?keyword=@");
	m_searchEngineTable.insert("tb", "https://s.taobao.com/search?q=@");
	m_searchEngineTable.insert("db", "https://www.douban.com/search?q=@");
	m_searchEngineTable.insert("epub", "http://cn.epubee.com/books/?s=@");
	
	QDir d(qApp->applicationDirPath());
	bool bOk = d.cd("config");
	if (!bOk)
		d.mkdir("config");

	QFile fData(qApp->applicationDirPath() + "\\config\\searchenginemap.db");
	if (fData.open(QFile::ReadOnly))
	{
		while (true)
		{
			QString lineContent = fData.readLine();
			if (lineContent.split(" ").count() < 2)
				break;
			QString strKey = lineContent.split(" ").at(0);
			QString strUrl = lineContent.split(" ").at(1);
			if (!strKey.isEmpty() && !strUrl.isEmpty())
				m_searchEngineTable[strKey] = strUrl;
			else
			{
				fData.close();
				break;
			}
		}
	}
}

bool Mainbox::searchEngineFilter(const QString& k)
{
	QString strKey = k.left(k.indexOf(" "));
	QString strContent = k.mid(k.indexOf(" ") + 1);
	if (searchEngineKeyFilter(strKey))
	{
		m_currentState = OmniState::Search;
		m_searchKey = strKey;
		m_searchContent = strContent;
		return true;
	}
	
	return false;
}

bool Mainbox::pluginFilter(const QString& k)
{
	QString strKey = k.left(k.indexOf(" "));
	QString strContent = k.mid(k.indexOf(" ") + 1);
	if (m_pPluginManager->isPluginExist(strKey))
	{
		m_currentState = OmniState::Plugin;
		m_pPluginWidget->show();
		emit startPluginQuery(strKey, strContent);
		return true;
	}
	return false;
}

bool Mainbox::fileFilter(const QString& k)
{
	m_currentState = OmniState::File;
	emit startSearchQuery(k);
	return true;
}

void Mainbox::processInputWord(const QString& t)
{
	QString k = t.trimmed();
	commandFilter(k)
		|| searchEngineFilter(k)
		|| pluginFilter(k)
		|| fileFilter(k);
}

void Mainbox::executeCommand()
{
	QString s = "start cmd /k " + m_searchContent;
	system(s.toStdString().c_str());
}

bool Mainbox::commandFilter(const QString& k)
{
	QString strKey = k.left(k.indexOf(" "));
	QString strContent = k.mid(k.indexOf(" ") + 1);
	if (strKey == ">")
	{
		m_currentState = OmniState::Command;
		m_searchContent = strContent;
		return true;
	}
	return false;
}
