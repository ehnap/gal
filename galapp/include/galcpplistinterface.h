#pragma once

#include <QtPlugin>
#include <QString>
#include <QIcon>

class QMenu;

struct ListItem
{
	QString title;
	QString content;
	QIcon icon;
};

class CppSimpleListInterface
{
public:
	virtual ~CppSimpleListInterface() {}

	virtual QVector<ListItem> query(const QString& val) = 0;
	virtual void exec(ListItem item) = 0;
	virtual QMenu* extentMenu(ListItem item) { Q_UNUSED(item); return Q_NULLPTR; }
};

#define CppListInterface_iid "org.gal.CppSimpleList.Interface"

Q_DECLARE_INTERFACE(CppSimpleListInterface, CppListInterface_iid)