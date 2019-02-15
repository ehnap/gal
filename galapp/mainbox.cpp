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
#include <QPainter>

#include <windows.h>

Mainbox::Mainbox(QWidget* parent/*= Q_NULLPTR*/)
	: QWidget(parent, Qt::FramelessWindowHint)
	, m_bDrag(false)
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
	connect(m_pInputEdit, &QLineEdit::textEdited, m_pItemList, &ResultListWidget::onKeyChanged);

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

void Mainbox::textEdited(const QString& t)
{
	QString k = t.trimmed();
	if (k.contains(" "))
	{
		QString strKey = k.left(k.indexOf(" "));
		QString strContent = k.mid(k.indexOf(" "));
		emit startPluginQuery(strKey, strContent);
	}
	else
	{
		m_pPluginWidget->hide();
		emit startSearchQuery(t);
	}
}

void Mainbox::firstInit()
{
	auto pcScreen = QGuiApplication::primaryScreen();
	QRect r = pcScreen->availableGeometry();
	QSize s = size();
	move((r.width() - s.width()) / 2, (r.height() - s.height()) / 4);
	::RegisterHotKey((HWND)winId(), 0x0923, MOD_ALT, VK_SPACE);
}

bool Mainbox::event(QEvent* e)
{
	if (e->type() == QEvent::WindowDeactivate)
	{
		m_pItemList->clear();
		m_pInputEdit->clear();
		adjustSize();
		hide();
	}

	if (e->type() == QEvent::ApplicationStateChange)
	{
		if (qApp->applicationState() != Qt::ApplicationActive)
		{
			m_pItemList->clear();
			m_pInputEdit->clear();
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
		m_pItemList->shot();
		m_pItemList->clear();
		m_pInputEdit->clear();
		adjustSize();
		hide();
	}

	if (e->key() == Qt::Key_Escape)
	{
		m_pItemList->clear();
		m_pInputEdit->clear();
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
		move(m_lastTopLeft.x() + p.x() - m_movablePoint.x(), m_lastTopLeft.y() + p.y() - m_movablePoint.y());
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
