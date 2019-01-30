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

QString PyData::getPy(const QString& key) const
{
	QString nameID = key;
	nameID.replace(" ", QString());
	QString resultId;
	for (int i = 0; i < nameID.length(); ++i)
	{
		ushort uni = nameID[i].unicode();
		if (uni >= 0x4E00 && uni <= 0x9FA5)
		{
			QStringList l = PyData::GetInstance().queryPy(nameID[i]);
			QStringList resultList;
			for (int i = 0; i < l.count(); i++)
			{
				QStringList tempList = resultId.split("|");
				for (int j = 0; j < tempList.count(); j++)
				{
					tempList[j] += l[i];
				}
				resultList += tempList;
			}
			resultId = resultList.join("|");
		}
		else
		{
			if (resultId.contains("|"))
				resultId = resultId.replace("|", nameID[i] + QString("|"));
			else
				resultId += nameID[i];
		}
	}
	return resultId;
}
