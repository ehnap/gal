#include "galcolor.h"
#include <QPainter>
#include <QStringList>
#include <QClipboard>
#include <QApplication>
#include <QMenu>
#include <QAction>

GalColorPlugin::GalColorPlugin()
{
	m_menu = new QMenu(Q_NULLPTR);
	m_cpHexrgb = new QAction(QObject::tr("Copy:#RRGGBB"), m_menu);
	QObject::connect(m_cpHexrgb, &QAction::triggered, this, &GalColorPlugin::cpHexRgb);
	m_menu->addAction(m_cpHexrgb);
	m_cpHexargb = new QAction(QObject::tr("Copy:#AARRGGBB"), m_menu);
	QObject::connect(m_cpHexargb, &QAction::triggered, this, &GalColorPlugin::cpHexArgb);
	m_menu->addAction(m_cpHexargb);
	m_cpIntargb = new QAction(QObject::tr("Copy:A,R,G,B"), m_menu);
	QObject::connect(m_cpIntargb, &QAction::triggered, this, &GalColorPlugin::cpIntArgb);
	m_menu->addAction(m_cpIntargb);
	m_cpIntrgb = new QAction(QObject::tr("Copy:R,G,B"), m_menu);
	QObject::connect(m_cpIntrgb, &QAction::triggered, this, &GalColorPlugin::cpIntRgb);
	m_menu->addAction(m_cpIntrgb);
}

GalColorPlugin::~GalColorPlugin()
{
	delete m_menu;
}

void GalColorPlugin::paint(QPainter* p, const QRect& r)
{
    p->save();

    p->fillRect(r, 0x424242);
    if (m_color.isValid())
        p->fillRect(r, m_color);

    QPen pen(0xd9d9d4);
    QFont f("Microsoft YaHei");
    f.setBold(false);
    f.setPixelSize(16);
    p->setFont(f);
    p->setPen(pen);
    p->drawText(r, Qt::AlignCenter, m_text);

    p->restore();
}

void GalColorPlugin::query(const QString& val)
{
	m_text = "Invalid Color";

	QColor tc = processColorString(val, ' ');
	if (tc.isValid())
	{
		m_color = tc;
		m_text = m_color.name(QColor::HexArgb);
		return;
	}
	tc = processColorString(val, '.');
	if (tc.isValid())
	{
		m_color = tc;
		m_text = m_color.name(QColor::HexArgb);
		return;
	}
	tc = processColorString(val, ',');
	if (tc.isValid())
	{
		m_color = tc;
		m_text = m_color.name(QColor::HexArgb);
		return;
	}

    QString colorName;
    if (val.at(0) != "#")
        colorName = "#" + val;
    else
        colorName = val;

    if (m_color.isValidColor(colorName))
    {
        m_color.setNamedColor(colorName);
        m_text = colorName;
    }
    else
        m_text = "Invalid Color";

}

QMenu* GalColorPlugin::extentMenu()
{
	if (!m_color.isValid())
		return Q_NULLPTR;

	return m_menu;
}

void GalColorPlugin::cpHexRgb()
{
	QClipboard* pClipboard = QApplication::clipboard();
	pClipboard->setText(m_color.name(QColor::HexRgb));
}

void GalColorPlugin::cpHexArgb()
{
	QClipboard* pClipboard = QApplication::clipboard();
	pClipboard->setText(m_color.name(QColor::HexArgb));
}

void GalColorPlugin::cpIntRgb()
{
	QString a = QString::number(m_color.alpha());
	QString r = QString::number(m_color.red());
	QString g = QString::number(m_color.green());
	QString b = QString::number(m_color.blue());

	QClipboard* pClipboard = QApplication::clipboard();
	pClipboard->setText(r + "," + g + "," + b);
}

void GalColorPlugin::cpIntArgb()
{
	QString a = QString::number(m_color.alpha());
	QString r = QString::number(m_color.red());
	QString g = QString::number(m_color.green());
	QString b = QString::number(m_color.blue());

	QClipboard* pClipboard = QApplication::clipboard();
	pClipboard->setText(a + "," + r + "," + g + "," + b);
}

QColor GalColorPlugin::processColorString(const QString& str, QChar c)
{
	if (str.contains(c))
	{
		QStringList l = str.split(c);
		if (l.count() != 3 && l.count() != 4)
			return QColor(QColor::Invalid);

		bool bOk = false;
		int r = l.at(0).toInt(&bOk);
		if (!bOk)
			return QColor(QColor::Invalid);
		bOk = false;
		int g = l.at(1).toInt(&bOk);
		if (!bOk)
			return QColor(QColor::Invalid);
		bOk = false;
		int b = l.at(2).toInt(&bOk);
		if (!bOk)
			return QColor(QColor::Invalid);
		bOk = false;
		if (l.count() == 4)
		{
			int a = r;
			r = g;
			g = b;
			b = l.at(3).toInt(&bOk);
			if (!bOk)
				return QColor(QColor::Invalid);
			else
				return QColor(r, g, b, a);
		}

		return QColor(r, g, b);
	}

	return QColor(QColor::Invalid);
}
