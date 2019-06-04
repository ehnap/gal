#include "gallistwidget.h"
#include "mainbox.h"
#include "omniobject.h"

#include <QProcess>
#include <QPainter>
#include <QTimer>
#include <QFile>
#include <QDir>
#include <QDesktopServices>
#include <QUrl>
#include <QScrollBar>
#include <QMenu>
#include <QApplication>
#include <QClipboard>
#include <QStandardItemModel>
#include <QTextStream>
#include <QKeyEvent>

GalListWidget::GalListWidget(QWidget* parent/*= Q_NULLPTR*/, int iVisibleItemCount/* = 6*/)
	: QListWidget(parent)
	, m_loadPoint(0)
	, m_visibleItemCount(iVisibleItemCount)
{
	setMouseTracking(true);
	setFrameStyle(NoFrame);
	setMinimumHeight(0);
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	setResizeMode(QListView::Adjust);
	setVisible(false);
	setItemDelegate(new GalListItemDelegate(this));
	connect(this, &GalListWidget::itemDoubleClicked, this, &GalListWidget::onItemDoubleClicked);
	connect(this, &GalListWidget::itemEntered, this, &GalListWidget::onItemEntered);
	connect(this, &GalListWidget::currentRowChanged, this, &GalListWidget::onCurrentRowChanged);
	QTimer::singleShot(0, this, &GalListWidget::firstInit);
}

GalListWidget::~GalListWidget()
{
}

int GalListWidget::visibleItemCount() const
{
	return m_visibleItemCount;
}

void GalListWidget::load()
{
	int i = 0;
	if (!m_itemQueue.empty())
	{
		m_queueMutex.lock();
		while (i++ < m_visibleItemCount + m_visibleItemCount / 2 && !m_itemQueue.empty())
		{
			GalListItem* pIt = m_itemQueue.takeFirst();
			QListWidget::addItem(pIt);
		}
		m_queueMutex.unlock();
	}

	updateGeometry();
	parentWidget()->adjustSize();
	update();
}

QSize GalListWidget::sizeHint() const
{
	int c = count() > m_visibleItemCount ? m_visibleItemCount : count();
	int h = 1;
	if (item(0))
		h = c * item(0)->sizeHint().height();
	return QSize(width(), h);
}

void GalListWidget::onItemDoubleClicked(QListWidgetItem* item)
{
	Q_UNUSED(item);
	GalListItem* pItem = dynamic_cast<GalListItem*>(currentItem());
	if (pItem)
	{
		pItem->exec();
	}
}

void GalListWidget::onItemEntered(QListWidgetItem* item)
{
	setCurrentItem(item);
}

void GalListWidget::onCurrentRowChanged(int currentRow)
{
	if (currentRow - m_loadPoint > m_visibleItemCount)
	{
		load();
		m_loadPoint = currentRow;
	}
}

void GalListWidget::onSliderMove()
{
	load();
}

void GalListWidget::firstInit()
{
	QFile f(":/scrollbarstyle.qss");

	if (f.open(QFile::ReadOnly))
	{
		verticalScrollBar()->setStyleSheet(f.readAll());
		QScrollBar* pScrollbar = verticalScrollBar();
		connect(pScrollbar, &QScrollBar::sliderPressed, this, &GalListWidget::onSliderMove);
		connect(pScrollbar, &QScrollBar::sliderMoved, this, &GalListWidget::onSliderMove);
		f.close();
	}
}

void GalListWidget::addItem(GalListItem* pItem)
{
	m_queueMutex.lock();
	m_itemQueue.append(pItem);
	m_queueMutex.unlock();
	load();
}

void GalListWidget::clear()
{
	QListWidget::clear();
	updateGeometry();
	parentWidget()->adjustSize();
	update();
}

GalListItem::GalListItem(GalListWidget* parent)
	: QListWidgetItem(parent)
{
	QListWidgetItem::setData(Qt::BackgroundRole, QColor(0x424242));
	setSizeHint(QSize(-1, 56));
}

GalListItem::~GalListItem()
{

}

void GalListItem::setTitle(const QString& t)
{
	QListWidgetItem::setData(GalListItem::Title, t);
}

QString GalListItem::title() const
{
	return QListWidgetItem::data(GalListItem::Title).toString();
}

void GalListItem::setContent(const QString& c)
{
	QListWidgetItem::setData(GalListItem::Content, c);
}

QString GalListItem::content() const
{
	return QListWidgetItem::data(GalListItem::Content).toString();
}

void GalListItem::setIcon(const QIcon& i)
{
	QListWidgetItem::setIcon(i);
}

QIcon GalListItem::icon() const
{
	return QListWidgetItem::icon();
}

GalListItemDelegate::GalListItemDelegate(QObject* parent /*= nullptr*/)
	: QItemDelegate(parent)
{

}

void GalListItemDelegate::paint(QPainter* p, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	p->save();
	p->setRenderHint(QPainter::HighQualityAntialiasing, true);

	if (option.showDecorationSelected && (option.state & QStyle::State_Selected))
	{
		p->fillRect(option.rect, 0x4f6180);
	}
	else if (option.state & QStyle::State_MouseOver)
	{
		p->fillRect(option.rect, 0x5f7190);
	}
	else {
		QVariant value = index.data(Qt::BackgroundRole);
		if (value.canConvert<QBrush>()) {
			QPointF oldBO = p->brushOrigin();
			p->setBrushOrigin(option.rect.topLeft());
			p->fillRect(option.rect, qvariant_cast<QBrush>(value));
			p->setBrushOrigin(oldBO);
		}
	}

	QStyleOptionViewItem opt = option;
	opt.decorationSize = QSize(32, 32);
	QRect r = option.rect;
	int iconMargin = (r.height() - opt.decorationSize.height()) / 2;
	QPoint rIconP(r.left() + iconMargin, r.top() + iconMargin);
	auto value = index.data(Qt::DecorationRole);
	if (value.isValid()) {
		auto pixmap = decoration(opt, value);
		p->drawPixmap(rIconP, pixmap);
	}

	auto d = index.data(GalListItem::Title).value<QString>();
	QPen textPen(0xFFFFF8);
	QFont f("Microsoft YaHei");
	f.setBold(true);
	f.setPixelSize(20);
	QFontMetrics fmName(f);
	p->setFont(f);
	p->setPen(textPen);
	QString dstring = fmName.elidedText(d, Qt::ElideRight, r.width() - 56);
	p->drawText(r.adjusted(56, 0, 0, 0), Qt::AlignLeft | Qt::TextSingleLine | Qt::TextJustificationForced, dstring);

	d = index.data(GalListItem::Content).value<QString>();
	f.setBold(false);
	f.setPixelSize(16);
	textPen.setColor(0xD9D9D4);
	p->setPen(textPen);
	p->setFont(f);
	QFontMetrics fmPath(f);
	dstring = fmPath.elidedText(d, Qt::ElideRight, r.width() - 56);
	p->drawText(r.adjusted(56, 28, 0, 0), Qt::AlignLeft | Qt::TextSingleLine | Qt::TextJustificationForced, dstring);

	p->restore();
}

QSize GalListItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	auto s = QItemDelegate::sizeHint(option, index);
	s.setHeight(56);
	return s;
}
