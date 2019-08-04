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
			resultId += l.count() > 1 ? "{" : "";
			for (int i = 0; i < l.count() - 1; i++)
			{
				resultId += l[i] + "|";
			}
			resultId += l[l.count() - 1];
			resultId += l.count() > 1 ? "}" : "";
		}
		else
		{
			resultId += nameID[i];
		}
	}
	return resultId;
}

bool PyData::isEqual(const QString& key, const QString& data) const
{
	int iStart = 0;
	int j = 0;
	int k = 0;
	int iKeyGuard = -1;
	bool bStartMatch = false;
	bool bTarget = false;
	do
	{
		if (k >= key.length())
			break;

		if (bStartMatch)
		{
			if (key[k] == data[j])
			{
				k++;
				if (k >= key.length())
				{
					bTarget = true;
					break;
				}
			}
			else if (data[j] == '{')
			{
				iKeyGuard = k;
			}
			else if (data[j] == '|')
			{
				do
				{
					j++;
				} while (data[j] != '}');
			}
			else if (data[j] == '}')
			{
				iKeyGuard = -1;
			}
			else
			{
				// key[k] != data[j]
				if (iKeyGuard == -1)
				{
					bStartMatch = false;
					j = iStart;
					k = 0;
				}
				else
				{
					k = iKeyGuard;
					do
					{
						j++;
						if (data[j] == '|')
							break;

						if (data[j] == '}')
						{
							bStartMatch = false;
							j = iStart;
							k = 0;
							break;
						}
					} while (true);
				}
			}
		}
		else
		{
			k = 0;
			if (key[k] == data[j])
			{
				iStart = j;
				bStartMatch = true;
				k++;
			}
		}
		j++;
	} while (j < data.count());

	return bTarget;
}

