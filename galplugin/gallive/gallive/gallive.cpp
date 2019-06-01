#include "gallive.h"
#include <QDesktopServices>
#include <QUrl>

GalLivePlugin::GalLivePlugin()
{
}

GalLivePlugin::~GalLivePlugin()
{
}

QVector<ListItem> GalLivePlugin::query(const QString& val)
{
	QVector<ListItem> resultList;

	ListItem hy;
	hy.title = "HuYa";
	hy.content = "https://www.huya.com/" + val;
	hy.icon = QIcon(":/huya.ico");
	resultList.append(hy);

	ListItem dy;
	dy.title = "DouYu";
	dy.content = "https://www.douyu.com/" + val;
	dy.icon = QIcon(":/douyu.png");
	resultList.append(dy);

	ListItem bz;
	bz.title = "BiliBili";
	bz.content = "https://live.bilibili.com/" + val;
	bz.icon = QIcon(":/bilibili.ico");
	resultList.append(bz);

	ListItem twitch;
	twitch.title = "Twitch";
	twitch.content = "https://www.twitch.tv/" + val;
	twitch.icon = QIcon(":/twitch.png");
	resultList.append(twitch);

	return resultList;
}

void GalLivePlugin::exec(ListItem item)
{
	QDesktopServices::openUrl(QUrl(item.content));
}
