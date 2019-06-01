#include "resultlist.h"
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

namespace
{
	struct TempData
	{
		int i;
		Data d;
	};

	bool greaterThanCount(const TempData& a,const TempData& b)
	{
		return a.i > b.i;
	}
}

ResultListWidget::ResultListWidget(Mainbox* parent/*= Q_NULLPTR*/)
	: GalListWidget(parent)
	, m_mainBox(parent)
	, m_bCountTableDirty(false)
	, m_omniFile(new OmniFile())
{
	QTimer::singleShot(0, this, &ResultListWidget::firstInit);
	m_backupTimer = new QTimer(this);
	m_backupTimer->setInterval(1000 * 60);
	connect(m_backupTimer, &QTimer::timeout, this, &ResultListWidget::saveDB);
	connect(m_omniFile->mainDataSet(), &MainDataSet::dataChanged, this, &ResultListWidget::onDataChanged);
	initMenu();
}

ResultListWidget::~ResultListWidget()
{
}

void ResultListWidget::initMenu()
{
	m_menu = new QMenu(this);
	auto pOd = new QAction(QObject::tr("&Open Dir"), m_menu);
	QObject::connect(pOd, &QAction::triggered, this, &ResultListWidget::openDir);
	m_menu->addAction(pOd);
	auto pCp = new QAction(QObject::tr("&Copy Path"), m_menu);
	QObject::connect(pCp, &QAction::triggered, this, &ResultListWidget::copyPath);
	m_menu->addAction(pCp);
	auto pCd = new QAction(QObject::tr("&Copy Dir"), m_menu);
	QObject::connect(pCd, &QAction::triggered, this, &ResultListWidget::copyDir);
	m_menu->addAction(pCd);
}

void ResultListWidget::shot()
{
	ResultItem* pItem = dynamic_cast<ResultItem*>(currentItem());
	if (pItem)
		pItem->exec();
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

void ResultListWidget::clear()
{
	hide();
	QListWidget::clear();
}

void ResultListWidget::extend()
{
	if (!isVisible())
		return;

	ResultItem* pItem = dynamic_cast<ResultItem*>(currentItem());
	if (!pItem)
		return;

	QRect rc = visualItemRect(pItem);
	QPoint lt = mapToGlobal(QPoint(0, 0));
	m_menu->move(lt.x() + rc.x() + rc.width() / 2, lt.y() + rc.y() + rc.height() / 2);
	m_menu->show();
}

void ResultListWidget::addHitCount(const QString& key)
{
	if (m_countTable.find(key) == m_countTable.end())
		m_countTable.insert(key, 1);
	else
		m_countTable[key]++;

	m_bCountTableDirty = true;
}

bool ResultListWidget::eventFilter(QObject* o, QEvent* e)
{
	if (isVisible() && e->type() == QEvent::KeyPress)
	{
		QKeyEvent* ev = dynamic_cast<QKeyEvent*>(e);
		if (ev)
		{
			if (ev->key() == Qt::Key_Down)
				next();

			if (ev->key() == Qt::Key_Up)
				prev();

			if (ev->key() == Qt::Key_Right && ev->modifiers() & Qt::AltModifier)
				extend();

			if (ev->key() == Qt::Key_Escape)
				clear();

			if (ev->key() == Qt::Key_Return || ev->key() == Qt::Key_Enter)
			{
				shot();
				clear();
			}
		}
	}

	return QListWidget::eventFilter(o, e);
}

QSharedPointer<OmniFile> ResultListWidget::getOmniFile() const
{
	return m_omniFile;
}

void ResultListWidget::onDataChanged(const QString& key)
{
	Q_UNUSED(key);
	int i = 0;
	Data d;
	QList<TempData> tempList;
	while (m_omniFile->mainDataSet()->takeData(d, m_mainBox->queryKey()))
	{
		TempData td;
		td.i = m_countTable.value(QDir::toNativeSeparators(d.path()), -i);
		td.d = d;
		tempList.append(td);
		QCoreApplication::processEvents();
	}
	
	qSort(tempList.begin(), tempList.end(), greaterThanCount);
	for (auto i = 0; i < tempList.count(); i++)
		addItem(new ResultItem(this, tempList[i].d));

	if (!tempList.empty())
		show();
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
				m_countTable.insert(strPath, iCount);
			else
			{
				fData.close();
				break;
			}
		}
		
	}

	m_backupTimer->start();
}

void ResultListWidget::copyPath()
{
	QClipboard* pClipboard = QApplication::clipboard();
	ResultItem* pIt = dynamic_cast<ResultItem*>(currentItem());
	pClipboard->setText(pIt->data().path());
}

void ResultListWidget::copyDir()
{
	QClipboard* pClipboard = QApplication::clipboard();
	ResultItem* pIt = dynamic_cast<ResultItem*>(currentItem());
	pClipboard->setText(pIt->data().dirPath());
}

void ResultListWidget::openDir()
{
	QProcess process;
	ResultItem* pIt = dynamic_cast<ResultItem*>(currentItem());
	process.startDetached("explorer /select," + QDir::toNativeSeparators(pIt->data().path()));
}

void ResultListWidget::saveDB()
{
	if (!m_bCountTableDirty)
		return;

	QFile fData(qApp->applicationDirPath() + "\\config\\data.db");
	if (fData.open(QFile::WriteOnly | QFile::Text | QFile::Truncate))
	{
		QTextStream out(&fData);
		QMap<QString, int>::const_iterator i = m_countTable.constBegin();
		QString lineContent;
		while (i != m_countTable.constEnd()) {
			QString hitCount = QString::number(i.value());
			QString strPath = i.key();
			if (!hitCount.isEmpty() && !strPath.isEmpty())
			{
				lineContent = QString::number(i.value()) + " " + i.key();
				out << lineContent << "\n";
			}
			i++;
		}
		fData.close();
	}

	m_bCountTableDirty = false;
}

ResultItem::ResultItem(GalListWidget* parent, const Data& d)
	: GalListItem(parent)
	, m_data(d)
{
	setIcon(m_data.icon());
	setTitle(m_data.displayName());
	setContent(m_data.path());
}

ResultItem::~ResultItem()
{

}

void ResultItem::exec()
{
	if (m_data.type() == Data::Type::Normal)
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

Data ResultItem::data() const
{
	return m_data;
}

