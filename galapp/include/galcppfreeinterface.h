#pragma once

#include <QtPlugin>
#include <QString>
#include <QRect>
#include <QVector>

class QPainter;
class QMenu;

class CppFreeInterface
{
public:
    virtual ~CppFreeInterface() {}

	virtual void paint(QPainter* p, const QRect& r) = 0;
	virtual void query(const QString& val) = 0;
    virtual QMenu* extentMenu() { return Q_NULLPTR; }
};

#define CppFreeInterface_iid "org.gal.CppFree.Interface"

Q_DECLARE_INTERFACE(CppFreeInterface, CppFreeInterface_iid)
