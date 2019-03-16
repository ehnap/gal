#pragma once

#include <QWidget>
#include <QHash>

class QLineEdit;
class ResultListWidget;
class MainDataSet;
class PluginManager;
class PluginStackedWidget;

class Mainbox : public QWidget
{

	Q_OBJECT

public:
	enum class OmniState
	{
		File,
		Search,
		Plugin,
		Command,
	};

	Mainbox(QWidget *parent = Q_NULLPTR);
	~Mainbox();

	QString queryKey() const;
	void popUp();

	bool searchEngineKeyFilter(const QString& key);
	bool execSearchEngine(const QString& key, const QString& value);

protected slots:
	void textEdited(const QString& t);
	void firstInit();

signals:
	void startSearchQuery(const QString& key);
	void startPluginQuery(const QString& key, const QString& value);

protected:
	bool event(QEvent* e) override;
	void keyPressEvent(QKeyEvent* e) override;
	void mousePressEvent(QMouseEvent* e) override;
	void mouseMoveEvent(QMouseEvent* e) override;
	void mouseReleaseEvent(QMouseEvent* e) override;
	bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;

private:
	void initSearchEngineTable();
	void processInputWord(const QString& t);
	void executeCommand();

	bool searchEngineFilter(const QString& k);
	bool pluginFilter(const QString& k);
	bool fileFilter(const QString& k);

private:
	QLineEdit* m_pInputEdit;
	ResultListWidget* m_pItemList;
	MainDataSet* m_pMainDataSet;
	PluginManager* m_pPluginManager;
	PluginStackedWidget* m_pPluginWidget;

private:
	QHash<QString, QString> m_searchEngineTable;
	QString m_searchKey;
	QString m_searchContent;
	OmniState m_currentState;
	bool m_bSearchEngineState;

private:
	//drag
	QPoint m_movablePoint;
	QPoint m_lastTopLeft;
	bool m_bDrag;
}