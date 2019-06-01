#pragma once

#include <QMap>

class PyData
{
public:
	static PyData& GetInstance()
	{
		static PyData instance;
		return instance;
	}
	~PyData();


	void init();
	QStringList queryPy(const QChar& key) const;
	QString getPy(const QString& key) const;
	bool isEqual(const QString& a, const QString& b) const;

private:
	PyData() {}
	PyData(const PyData&);
	PyData& operator = (const PyData&);

private:
	QMap<QString, QStringList> m_data;
};

