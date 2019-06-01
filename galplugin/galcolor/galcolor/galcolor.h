#ifndef GALCOLOR_H
#define GALCOLOR_H

#include "galcolor_global.h"
#include "galcppfreeinterface.h"

#include <QObject>
#include <QColor>

class QMenu;
class QAction;

class GalColorPlugin : public QObject, CppFreeInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.gal.galcolor.Plugin" FILE "galcolorplugin.json")
    Q_INTERFACES(CppFreeInterface)

public:
    GalColorPlugin();
	~GalColorPlugin();

    void paint(QPainter* p, const QRect& r) override;
    void query(const QString& val) override;
    QMenu* extentMenu() override;

private slots:
	void cpHexRgb();
	void cpHexArgb();
	void cpIntRgb();
	void cpIntArgb();

private:
	QColor processColorString(const QString& str, QChar c);

private:
    QString m_text;
    QColor m_color;
	QMenu* m_menu;
	QAction* m_cpHexrgb;
	QAction* m_cpHexargb;
	QAction* m_cpIntrgb;
	QAction* m_cpIntargb;
};


#endif // GALCOLOR_H
