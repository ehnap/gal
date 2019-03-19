#include "mainbox.h"
#include "data.h"
#include "resultlist.h"
#include "pluginmanager.h"
#include "omniobject.h"

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
	connect(m_pInputEdit, &QLineEdit::textEdited, this, &Mainbox::textEdited);

	QFont f = font();
	f.setFamily("Microsoft YaHei");
	f.setPixelSize(36);
	m_pInputEdit->setFont(f);
	pMainlayout->addWidget(m_pInputEdit);

	m_pPluginWidget = new PluginStackedWidget(this);
	pMainlayout->addWidget(m_pPluginWidget);
	installEventFilter(m_pPluginWidget);
	m_pPluginWidget->hide();
	
	m_pItemList = new ResultListWidget(this);
	installEventFilter(m_pItemList);
	pMainlayout->addWidget(m_pItemList);

	QTimer::singleShot(0, this, &Mainbox::firstInit);
}

Mainbox::~Mainbox()
{
	::UnregisterHotKey((HWND)winId(), 0x0923);
}

QString Mainbox::queryKey() const
{
	return m_pInputEdit->text().trimmed();
}

void Mainbox::popUp()
{
	show();
	HWND hWnd = (HWND)winId();
	::SetForegroundWindow(hWnd);
}

void Mainbox::textEdited(const QString& t)
{
	Q_UNUSED(t);
	m_pItemList->clear();
	m_pPluginWidget->hide();
	adjustSize();

	processInputWord(queryKey());
}

void Mainbox::firstInit()
{
	auto pcScreen = QGuiApplication::primaryScreen();
	QRect r = pcScreen->availableGeometry();
	QSize s = size();
	move((r.width() - s.width()) / 2, (r.height() - s.height()) / 4);
	
	::RegisterHotKey((HWND)winId(), 0x0923, MOD_ALT, VK_SPACE);

	initOmniObjectList();
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
	if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter)
	{
		if (m_omniObjects[m_currentObjectIndex]->respondType() == OmniObject::RespondType::Delay)
		{
			m_omniObjects[m_currentObjectIndex]->execEx(queryKey());
		}
		m_pInputEdit->clear();
		adjustSize();
		hide();	
	}

	if (e->key() == Qt::Key_Escape)
	{
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

void Mainbox::initOmniObjectList()
{
	m_omniObjects.push_back(QSharedPointer<OmniCommand>(new OmniCommand()));
	m_omniObjects.push_back(QSharedPointer<OmniSearchEngine>(new OmniSearchEngine()));
	m_omniObjects.push_back(m_pPluginWidget->getOmniPlugin());
	m_omniObjects.push_back(m_pItemList->getOmniFile());
}

void Mainbox::processInputWord(const QString& t)
{
	QString k = t.trimmed();
	for (int i = 0; i < m_omniObjects.size(); i++)
	{
		if (m_omniObjects[i]->filter(t))
		{
			m_currentObjectIndex = i;
			if (m_omniObjects[i]->respondType() == OmniObject::RespondType::Real)
				m_omniObjects[i]->execEx(k);
			break;
		}
	}
}