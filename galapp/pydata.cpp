#include "pydata.h"
#include <QFile>

void PyData::init()
{
	QFile f(":/py.data");

	if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
		return;

	while (!f.atEnd())
	{
		QString content = QString::fromUtf8(f.readLine());
		QStringList l = content.split(" ", QString::SkipEmptyParts);
		for (int i = 1; i < l.count(); i++)
		{
			l[i] = l[i].simplified();
			l[i] = l[i].left(l[i].count() - 1);
		}
		QString ucode = l.takeFirst();
		l.removeDuplicates();
		m_data.insert(ucode, l);
	}
}

PyData::~PyData()
{
}

QStringList PyData::queryPy(const QChar& key) const
{
	QString unicode = QString::number(key.unicode(), 16);
	return m_data.value(unicode.toUpper());
}
