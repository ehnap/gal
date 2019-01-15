#pragma once

#include "data.h"
#include <QWidget>
#include <QListWidget>
#include <QItemDelegate>

class ResultItem : public QListWidgetItem
{
public:
	ResultItem(QListWidget* parent, const Data& data);
	~ResultItem();

	void exec();

private:
	Data m_data;
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
	ResultListWidget(QWidget* parent = Q_NULLPTR);
	~ResultListWidget();

	void next();
	void prev();
	void shot();

	void clear();
	void setResult(const ResultSet& r);
	void delayShow();

protected:
	QSize sizeHint() const override;

protected slots:
	void onItemDoubleClicked(QListWidgetItem* item);
	void onItemEntered(QListWidgetItem* item);
	void onCurrentRowChanged(int currentRow);
	void onDelayShow();
	void onSliderMove();
	void firstInit();

private:
	void load();
	void loadAll();

private:
	ResultSet m_tempResult;
	int m_loadPoint;
	QTimer* m_delayTimer;
};