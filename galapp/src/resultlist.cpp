#include "resultlist.h"
#include "mainbox.h"
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

namespace
{
	struct TempData
	{
		int i;
		Data d;
	};

	bool greaterThanCount(TempData a, TempData b)
	{
		return a.i > b.i;
	}
}

ResultListWidget::ResultListWidget(Mainbox* parent/*= Q_NULLPTR*/)
	: QListWidget(parent)
	, m_mainBox(parent)
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
	m_delayShowTimer = new QTimer(this);
	connect(m_delayShowTimer, &QTimer::timeout, this, &ResultListWidget::onDelayShow);
	m_delayLoadTimer = new QTimer(this);
	connect(m_delayLoadTimer, &QTimer::timeout, this, &ResultListWidget::onDelayLoad);
	m_backupTimer = new QTimer(this);
	m_backupTimer->setInterval(1000 * 60 * 30);
	connect(m_backupTimer, &QTimer::timeout, this, &ResultListWidget::saveDB);
}

ResultListWidget::~ResultListWidget()
{
	saveDB();
}

void ResultListWidget::next()
{
	int r = currentRow();
	++r;
	r = qMin(count() - 1, r);
	setCurrentRow(r);
}

void ResultListWidget::prev()
{
	int r = currentRow();
	--r;
	r = qMax(0, r);
	setCurrentRow(r);
}

void ResultListWidget::shot()
{
	ResultItem* pItem = dynamic_cast<ResultItem*>(currentItem());
	if (pItem)
		pItem->exec();
}

void ResultListWidget::extend()
{
	if (!isVisible())
		return;

	ResultItem* pItem = dynamic_cast<ResultItem*>(currentItem());
	if (pItem)
		pItem->extend();
}

void ResultListWidget::clear()
{
	hide();
	QListWidget::clear();
}

void ResultListWidget::setMainDataSet(MainDataSet* d)
{
	m_dataSet = d;
}

void ResultListWidget::delayShow()
{
	m_delayShowTimer->stop();
	m_delayShowTimer->start(6);
}

void ResultListWidget::addHitCount(const QString& key)
{
	m_countHashTable[key]++;
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
		m_delayShowTimer->stop();
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

void ResultListWidget::onDelayLoad()
{
	m_delayLoadTimer->stop();

	load();

	if (count() > 0)
		delayShow();
	else
		hide();

	updateGeometry();
	parentWidget()->adjustSize();
	update();
}

void ResultListWidget::onDataChanged(const QString& key)
{
	Q_UNUSED(key);
	m_delayLoadTimer->stop();
	m_delayLoadTimer->start(6);
}

void ResultListWidget::firstInit()
{
	QFile f(":/scrollbarstyle.qss");

	if (f.open(QFile::ReadOnly))
	{
		verticalScrollBar()->setStyleSheet(f.readAll());
		QScrollBar* pScrollbar = verticalScrollBar();
		connect(pScrollbar, &QScrollBar::sliderPressed, this, &ResultListWidget::onSliderMove);
		connect(pScrollbar, &QScrollBar::sliderMoved, this, &ResultListWidget::onSliderMove);
		f.close();
	}

	QDir d(qApp->applicationDirPath());
	bool bOk = d.cd("config");
	if (!bOk)
		d.mkdir("config");

	QFile fData(qApp->applicationDirPath() + "\\config\\data.db");
	if (fData.open(QFile::ReadOnly))
	{
		while (true)
		{
			QString lineContent = fData.readLine();
			if (lineContent.split(" ").count() < 2)
				break;

			QString strCount = lineContent.split(" ").at(0);
			QString strPath = lineContent.split(" ").at(1);
			bool bOk = false;
			int iCount = strCount.toInt(&bOk);
			if (bOk && !strPath.isEmpty())
				m_countHashTable.insert(strPath, iCount);
			else
			{
				fData.close();
				break;
			}
		}
		
	}

	m_backupTimer->start();
}

void ResultListWidget::load()
{
	int i = 0;
	Data d;
	QList<TempData> tempList;
	while (i++ < 6 + 3 && m_dataSet->takeData(d, m_mainBox->queryKey()))
	{
		TempData td;
		td.i = m_countHashTable.value(QDir::toNativeSeparators(d.path()));
		td.d = d;
		tempList.append(td);
	}
	
	qSort(tempList.begin(), tempList.end(), greaterThanCount);
	for (auto i = 0; i < tempList.count(); i++)
		addItem(new ResultItem(this, tempList[i].d));
}

void ResultListWidget::loadAll()
{
	Data d;
	while (m_dataSet->takeData(d, m_mainBox->queryKey()))
	{
		addItem(new ResultItem(this, d));
	}
}

void ResultListWidget::saveDB()
{
	QFile fData(qApp->applicationDirPath() + "\\config\\data.db");
	if (fData.open(QFile::WriteOnly | QIODevice::Text))
	{
		QTextStream out(&fData);
		QHash<QString, int>::const_iterator i = m_countHashTable.constBegin();
		QString lineContent;
		while (i != m_countHashTable.constEnd()) {
			lineContent = QString::number(i.value()) + " " + i.key();
			out << lineContent << "\n";
			i++;
		}
		fData.close();
	}
}

ResultItem::ResultItem(QListWidget* parent, const Data& data)
	: QListWidgetItem(parent)
	, m_data(data)
{
	setIcon(m_data.icon());
	setData(Qt::UserRole, QVariant::fromValue(m_data));
	setData(Qt::BackgroundRole, QColor(0x424242));
	setSizeHint(QSize(-1, 56));

	initMenu();
}

ResultItem::~ResultItem()
{

}

void ResultItem::exec()
{
	if (m_data.type() == Data::Normal)
	{
		ResultListWidget* pWidget = qobject_cast<ResultListWidget*>(listWidget());
		QString fullPath = QDir::toNativeSeparators(m_data.path());
		if (pWidget)
			pWidget->addHitCount(fullPath);
		
		QFileInfo fi(fullPath);
		if (fi.isFile())
		{
			QProcess process;
			process.setProgram(fullPath);
			process.startDetached();
		}
		else if (fi.isDir())
		{
			QDesktopServices::openUrl(QUrl::fromLocalFile(fullPath));
		}
	}
	else
	{
		m_data.exec();
	}
}

void ResultItem::extend()
{
	QListWidget* pWidget = listWidget();
	if (!pWidget)
		return;

	QRect rc = pWidget->visualItemRect(this);
	QPoint lt = pWidget->mapToGlobal(QPoint(0, 0));
	m_menu->move(lt.x() + rc.x() + rc.width() / 2, lt.y() + rc.y() + rc.height() / 2);
	m_menu->show();
}

void ResultItem::copyPath()
{
	QClipboard* pClipboard = QApplication::clipboard();
	pClipboard->setText(m_data.path());
}

void ResultItem::copyDir()
{
	QClipboard* pClipboard = QApplication::clipboard();
	pClipboard->setText(m_data.dirPath());
}

void ResultItem::openDir()
{
	QProcess process;
	process.startDetached("explorer /select," + QDir::toNativeSeparators(m_data.path()));
}

void ResultItem::initMenu()
{
	m_menu = new QMenu(listWidget());
	auto pOd = new QAction(QObject::tr("&Open Dir"), m_menu);
	QObject::connect(pOd, &QAction::triggered, this, &ResultItem::openDir);
	m_menu->addAction(pOd);
	auto pCp = new QAction(QObject::tr("&Copy Path"), m_menu);
	QObject::connect(pCp, &QAction::triggered, this, &ResultItem::copyPath);
	m_menu->addAction(pCp);
	auto pCd = new QAction(QObject::tr("&Copy Dir"), m_menu);
	QObject::connect(pCd, &QAction::triggered, this, &ResultItem::copyDir);
	m_menu->addAction(pCd);
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
		auto pixmap = decoration(opt, value);
		p->drawPixmap(rIconP, pixmap);
	}

	QPen textPen(0xFFFFF8);
	QFont f("Microsoft YaHei");
	f.setBold(true);
	f.setPixelSize(20);
	QFontMetrics fmName(f);
	p->setFont(f);
	p->setPen(textPen);
	QString dstring = fmName.elidedText(d.displayName(), Qt::ElideRight, r.width() - 56);
	p->drawText(r.adjusted(56, 0, 0, 0), Qt::AlignLeft|Qt::TextSingleLine|Qt::TextJustificationForced, dstring);

	f.setBold(false);
	f.setPixelSize(16);
	textPen.setColor(0xD9D9D4);
	p->setPen(textPen);
	p->setFont(f);
	QFontMetrics fmPath(f);
	dstring = fmPath.elidedText(d.path(), Qt::ElideRight, r.width() - 56);
	p->drawText(r.adjusted(56, 28, 0, 0), Qt::AlignLeft | Qt::TextSingleLine | Qt::TextJustificationForced, dstring);

	p->restore();
}

QSize ResultItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	auto s = QItemDelegate::sizeHint(option, index);
	s.setHeight(56);
	return s;
}
