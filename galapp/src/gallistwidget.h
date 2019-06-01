#pragma once

#include <QWidget>
#include <QListWidget>
#include <QItemDelegate>
#include <QPointer>
#include <QQueue>
#include <QMutex>

class QMenu;
class GalListWidget;

class GalListItem : public QListWidgetItem
{
public:
	enum GalDataRole
	{
		Title = Qt::UserRole + 1,
		Content,
		Icon,

		GalUserRole = Title + 10,
	};

	GalListItem(GalListWidget* parent);
	virtual ~GalListItem();

	void setTitle(const QString& t);
	QString title() const;

	void setContent(const QString& c);
	QString content() const;

	void setIcon(const QIcon& i);
	QIcon icon() const;

	virtual void exec() {}
};

class GalListItemDelegate : public QItemDelegate
{
public:
	explicit GalListItemDelegate(QObject* parent = nullptr);

protected:
	void paint(QPainter *painter,
		const QStyleOptionViewItem &option,
		const QModelIndex& index) const override;
	QSize sizeHint(const QStyleOptionViewItem &option,
		const QModelIndex &index) const override;
};

class GalListWidget : public QListWidget
{
	Q_OBJECT

public:
	GalListWidget(QWidget* parent = Q_NULLPTR, int iVisibleItemCount = 6);
	~GalListWidget();

	int visibleItemCount() const;

	void addItem(GalListItem* item);
	

protected:
	QSize sizeHint() const override;

protected slots:
	void onItemDoubleClicked(QListWidgetItem* item);
	void onItemEntered(QListWidgetItem* item);
	void onCurrentRowChanged(int currentRow);
	void onSliderMove();
	void firstInit();

private:
	void load();

private:
	int m_loadPoint;
	int m_visibleItemCount;
	QMutex m_queueMutex;
	QQueue<GalListItem*> m_itemQueue;
};