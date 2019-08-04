#pragma once

#include "data.h"
#include "gallistwidget.h"
#include <QWidget>
#include <QListWidget>
#include <QItemDelegate>

class MainDataSet;
class Mainbox;
class OmniFile;
class QMenu;

class ResultItem : public GalListItem
{
public:
	ResultItem(GalListWidget* parent, const Data& d);
	~ResultItem();

	void exec() override;
	Data data() const;

private:
	Data m_data;
};

class ResultListWidget : public GalListWidget
{
	Q_OBJECT

public:
	ResultListWidget(Mainbox* parent = Q_NULLPTR);
	~ResultListWidget();

	void next();
	void prev();
	void shot();
	void extend();
	void clear() override;
	void addHitCount(const QString& key);

	QSharedPointer<OmniFile> getOmniFile() const;

public slots:
	void onDataChanged(const QString& key);

protected slots:
	void firstInit();
	void saveDB();

protected:
	bool eventFilter(QObject* o, QEvent* e) override;

private slots:
	void copyPath();
	void copyDir();
	void openDir();

private:
	void initMenu();

private:
	QTimer* m_backupTimer;
	Mainbox* m_mainBox;
	QMenu* m_menu;
	QMap<QString, int> m_countTable;
	bool m_bCountTableDirty;
	QSharedPointer<OmniFile> m_omniFile;
};