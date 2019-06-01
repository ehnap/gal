#ifndef GALLIVE_H
#define GALLIVE_H

#include "gallive_global.h"
#include "galcpplistinterface.h"

#include <QObject>
#include <QColor>

class QMenu;
class QAction;

class GalLivePlugin : public QObject, CppSimpleListInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.gal.gallive.Plugin" FILE "galliveplugin.json")
    Q_INTERFACES(CppSimpleListInterface)

public:
    GalLivePlugin();
    ~GalLivePlugin();

    virtual QVector<ListItem> query(const QString& val);
    virtual void exec(ListItem item);

};


#endif // GALLIVE_H
