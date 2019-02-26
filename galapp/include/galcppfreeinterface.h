#pragma once

#include <QtPlugin>
#include <QString>
#include <QRect>

class QPainter;
class QMenu;

class CppFreeInterface
{
public:
    virtual ~CppFreeInterface() {}

	virtual void paint(QPainter* p, const QRect& r) = 0;
	virtual void exec(const QString& val) = 0;
    virtual QMenu* extentMenu() { return Q_NULLPTR; }
};

#define CppFreeInterface_iid "org.gal.galcolor.CppFreeInterface"

Q_DECLARE_INTERFACE(CppFreeInterface, CppFreeInterface_iid)
