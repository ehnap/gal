#pragma once

#include <QWidget>
#include <QHash>
#include "omniobject.h"

class QLineEdit;
class ResultListWidget;
class MainDataSet;
class PluginManager;
class PluginStackedWidget;

class Mainbox : public QWidget
{

	Q_OBJECT

public:
	Mainbox(QWidget *parent = Q_NULLPTR);
	~Mainbox();

	QString queryKey() const;
	void popUp();
	OmniObject::OmniType currentOmniType() const;

protected slots:
	void textEdited(const QString& t);
	void firstInit();

protected:
	bool event(QEvent* e) override;
	void keyPressEvent(QKeyEvent* e) override;
	void mousePressEvent(QMouseEvent* e) override;
	void mouseMoveEvent(QMouseEvent* e) override;
	void mouseReleaseEvent(QMouseEvent* e) override;
	bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;

private:
	void initOmniObjectList();
	void processInputWord(const QString& t);

private:
	QLineEdit* m_pInputEdit;
	ResultListWidget* m_pItemList;
	PluginStackedWidget* m_pPluginWidget;

private:
	QVector<QSharedPointer<OmniObject>> m_omniObjects;
	int m_currentObjectIndex;

private:
	//drag
	QPoint m_movablePoint;
	QPoint m_lastTopLeft;
	bool m_bDrag;
};