#ifndef GALLIVE_H
#define GALLIVE_H

#include "galyd_global.h"
#include "galcpplistinterface.h"

#include <QObject>
#include <QColor>
#include <QNetworkAccessManager>

class QMenu;
class QAction;

class GalYDPlugin : public QObject, CppSimpleListInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.gal.galyd.Plugin" FILE "galydplugin.json")
    Q_INTERFACES(CppSimpleListInterface)

public:
	GalYDPlugin();
    ~GalYDPlugin();

    virtual QVector<ListItem> query(const QString& val) override;
    virtual void exec(ListItem item);

private slots:
	void firstInit();

private:
	QNetworkAccessManager* m_pNetManager;
	QString m_ydkeyfrom;
	QString m_ydkey;
	
	QString m_cacheKey;
	QVector<ListItem> m_cacheResult;
};


#endif // GALLIVE_H
