#pragma once

#include "data.h"
#include <QWidget>
#include <QListWidget>
#include <QItemDelegate>

class MainDataSet;
class Mainbox;
class OmniFile;
class QMenu;

class ResultItem : public QObject, public QListWidgetItem
{

	Q_OBJECT

public:
	ResultItem(QListWidget* parent, const Data& data);
	~ResultItem();

	void exec();
	void extend();

private slots:
	void copyPath();
	void copyDir();
	void openDir();

private:
	void initMenu();

private:
	Data m_data;
	QMenu* m_menu;
};

class ResultItemDelegate : public QItemDelegate
{
public:
	explicit ResultItemDelegate(QObject* parent = nullptr);

protected:
	void paint(QPainter *painter,
		const QStyleOptionViewItem &option,
		const QModelIndex& index) const override;
	QSize sizeHint(const QStyleOptionViewItem &option,
		const QModelIndex &index) const override;
};

class ResultListWidget : public QListWidget
{
	Q_OBJECT

public:
	ResultListWidget(Mainbox* parent = Q_NULLPTR);
	~ResultListWidget();

	void next();
	void prev();
	void shot();
	void extend();

	void clear();
	void delayShow();
	void addHitCount(const QString& key);

	QSharedPointer<OmniFile> getOmniFile() const;

protected:
	QSize sizeHint() const override;
	bool eventFilter(QObject* o, QEvent* e) override;

public slots:
	void onDataChanged(const QString& key);

protected slots:
	void onItemDoubleClicked(QListWidgetItem* item);
	void onItemEntered(QListWidgetItem* item);
	void onCurrentRowChanged(int currentRow);
	void onDelayShow();
	void onSliderMove();
	void onDelayLoad();
	void firstInit();
	void saveDB();

private:
	void load();
	void loadAll();

private:
	int m_loadPoint;
	QTimer* m_delayShowTimer;
	QTimer* m_delayLoadTimer;
	QTimer* m_backupTimer;
	Mainbox* m_mainBox;

	QMap<QString, int> m_countTable;
	bool m_bCountTableDirty;

	QSharedPointer<OmniFile> m_omniFile;
};