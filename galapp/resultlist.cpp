#include "resultlist.h"
#include <QProcess>
#include <QPainter>
#include <QTimer>
#include <QFile>
#include <QScrollBar>
#include <QStandardItemModel>

ResultListWidget::ResultListWidget(QWidget* parent/*= Q_NULLPTR*/)
	: QListWidget(parent)
	, m_loadPoint(0)
{
	setMouseTracking(true);
	setFrameStyle(NoFrame);
	setMinimumHeight(0);
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	setResizeMode(QListView::Adjust);
	setVisible(false);
	setItemDelegate(new ResultItemDelegate(this));
	connect(this, &ResultListWidget::itemDoubleClicked, this, &ResultListWidget::onItemDoubleClicked);
	connect(this, &ResultListWidget::itemEntered, this, &ResultListWidget::onItemEntered);
	connect(this, &ResultListWidget::currentRowChanged, this, &ResultListWidget::onCurrentRowChanged);
	QTimer::singleShot(0, this, &ResultListWidget::firstInit);
	m_delayTimer = new QTimer(this);
	connect(m_delayTimer, &QTimer::timeout, this, &ResultListWidget::onDelayShow);
}

ResultListWidget::~ResultListWidget()
{
}

void ResultListWidget::next()
{
	int r = currentRow();
	++r;
	r = r == count() ? count() - 1: r;
	setCurrentRow(r);
}

void ResultListWidget::prev()
{
	int r = currentIndex().row();
	--r;
	r = r < 0 ? 0 : r;
	setCurrentRow(r);
}

void ResultListWidget::shot()
{
	ResultItem* pItem = dynamic_cast<ResultItem*>(currentItem());
	if (pItem)
		pItem->exec();
}

void ResultListWidget::clear()
{
	QListWidget::clear();
}

void ResultListWidget::setResult(const ResultSet& r)
{
	clear();
	m_tempResult = r;
	
	load();

	if (count() > 0)
		delayShow();
	else
		hide();

	updateGeometry();
	parentWidget()->adjustSize();
	update();
}

void ResultListWidget::delayShow()
{
	m_delayTimer->stop();
	m_delayTimer->start(6);
}

QSize ResultListWidget::sizeHint() const
{
	int c = count() > 6 ? 6 : count();
	int h = 1;
	if (item(0))
		h = c * item(0)->sizeHint().height();
	return QSize(width(), h);
}

void ResultListWidget::onItemDoubleClicked(QListWidgetItem* item)
{
    Q_UNUSED(item);
	ResultItem* pItem = dynamic_cast<ResultItem*>(currentItem());
	if (pItem)
	{
		pItem->exec();
	}
}

void ResultListWidget::onItemEntered(QListWidgetItem* item)
{
	setCurrentItem(item);
}

void ResultListWidget::onCurrentRowChanged(int currentRow)
{
	if (currentRow - m_loadPoint > 6)
	{
		load();
		m_loadPoint = currentRow;
	}
}

void ResultListWidget::onDelayShow()
{
	if (count() > 0)
	{
		m_delayTimer->stop();
		show();
		setCurrentRow(0);
		m_loadPoint = 0;
	}
	
	updateGeometry();
	parentWidget()->adjustSize();
	update();
}

void ResultListWidget::onSliderMove()
{
	load();
}

void ResultListWidget::firstInit()
{
	QFile f(":/scrollbarstyle.qss");
	f.open(QFile::ReadOnly);
	verticalScrollBar()->setStyleSheet(f.readAll());
	QScrollBar* pScrollbar = verticalScrollBar();
	connect(pScrollbar, &QScrollBar::sliderPressed, this, &ResultListWidget::onSliderMove);
	connect(pScrollbar, &QScrollBar::sliderMoved, this, &ResultListWidget::onSliderMove);
}

void ResultListWidget::load()
{
	int i = 0;
	if (m_tempResult.count() <= 12)
	{
		loadAll();
	}
	else
	{
		while (i++ < 6 * 2)
		{
			Data d = m_tempResult.takeFirst();
			addItem(new ResultItem(this, d));
		}
	}
}

void ResultListWidget::loadAll()
{
	foreach(const Data& result, m_tempResult)
	{
		addItem(new ResultItem(this, result));
 	}
	m_tempResult.clear();
}

ResultItem::ResultItem(QListWidget* parent, const Data& data)
	: QListWidgetItem(parent)
	, m_data(data)
{
	setText(m_data.name());
	setIcon(m_data.icon());
	setData(Qt::UserRole, QVariant::fromValue(m_data));
	setData(Qt::BackgroundRole, QColor(0x424242));
	setSizeHint(QSize(-1, 56));
}

ResultItem::~ResultItem()
{

}

void ResultItem::exec()
{
	QProcess process;
	process.setProgram(m_data.path());
	process.startDetached();
}

ResultItemDelegate::ResultItemDelegate(QObject* parent /*= nullptr*/)
	: QItemDelegate(parent)
{

}

void ResultItemDelegate::paint(QPainter* p, const QStyleOptionViewItem& option, const QModelIndex& index) const
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

	auto i = index.data(Qt::UserRole);
	auto d = i.value<Data>();
	QStyleOptionViewItem opt = option;
	opt.decorationSize = QSize(32, 32);
	QRect r = option.rect;
	int iconMargin = (r.height() - opt.decorationSize.height()) / 2;
	QPoint rIconP(r.left() + iconMargin, r.top() + iconMargin);
	auto value = index.data(Qt::DecorationRole);
	if (value.isValid()) {
		// ### we need the pixmap to call the virtual function
		auto pixmap = decoration(opt, value);
		p->drawPixmap(rIconP, pixmap);
	}

	QPen textPen(0xFFFFF8);
	QFont f("Microsoft YaHei");
	f.setBold(true);
	f.setPixelSize(20);
	p->setFont(f);
	p->setPen(textPen);
	p->drawText(r.adjusted(56, 0, 0, 0), d.name());
	f.setBold(false);
	f.setPixelSize(16);
	textPen.setColor(0xD9D9D4);
	p->setPen(textPen);
	p->setFont(f);
	p->drawText(r.adjusted(56, 28, 0, 0), d.path());

	p->restore();
}

QSize ResultItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	auto s = QItemDelegate::sizeHint(option, index);
	s.setHeight(56);
	return s;
}
