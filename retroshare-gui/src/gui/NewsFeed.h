/****************************************************************
 *  RetroShare is distributed under the following license:
 *
 *  Copyright (C) 2008 Robert Fernie
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, 
 *  Boston, MA  02110-1301, USA.
 ****************************************************************/

#ifndef _NEWS_FEED_DIALOG_H
#define _NEWS_FEED_DIALOG_H

#include "mainpage.h"

#include "gui/feeds/FeedHolder.h"
#include "util/TokenQueue.h"
#include <retroshare-gui/RsAutoUpdatePage.h>

#define IMAGE_NEWSFEED ":/home/img/face_icon/newsfeed_128.png"

const uint32_t NEWSFEED_PEERLIST =       0x0001;

const uint32_t NEWSFEED_FORUMNEWLIST =   0x0002;
const uint32_t NEWSFEED_FORUMMSGLIST =   0x0003;
const uint32_t NEWSFEED_CHANNELNEWLIST = 0x0004;
const uint32_t NEWSFEED_CHATNEWLIST     = 0x0005;
#if 0
const uint32_t NEWSFEED_BLOGNEWLIST =    0x0006;
const uint32_t NEWSFEED_BLOGMSGLIST =    0x0007;
#endif

const uint32_t NEWSFEED_MESSAGELIST =      0x0008;
const uint32_t NEWSFEED_CHATMSGLIST =      0x0009;
const uint32_t NEWSFEED_SECLIST =          0x000a;
const uint32_t NEWSFEED_POSTEDNEWLIST =    0x000b;
const uint32_t NEWSFEED_POSTEDMSGLIST =    0x000c;
const uint32_t NEWSFEED_CIRCLELIST    =    0x000d;
const uint32_t NEWSFEED_CHANNELPUBKEYLIST= 0x000e;
const uint32_t NEWSFEED_CHATPUBKEYLIST   = 0x000f;


namespace Ui {
class NewsFeed;
}

class RsFeedItem;
class FeedNotify;
class FeedItem;

class NewsFeed : public RsAutoUpdatePage, public FeedHolder, public TokenResponse
{
	Q_OBJECT

public:
	/** Default Constructor */
	NewsFeed(QWidget *parent = 0);
	/** Default Destructor */
	virtual ~NewsFeed();

	virtual QIcon iconPixmap() const { return QIcon(IMAGE_NEWSFEED) ; } //MainPage
    virtual QString pageName() const { return tr("Activities") ; } //MainPage
	virtual QString helpText() const { return ""; } //MainPage

	virtual UserNotify *getUserNotify(QObject *parent);

	/* FeedHolder Functions (for FeedItem functionality) */
	virtual QScrollArea *getScrollArea();
	virtual void deleteFeedItem(QWidget *item, uint32_t type);
	virtual void openChat(const RsPeerId& peerId);
	virtual void openComments(uint32_t type, const RsGxsGroupId &groupId, const QVector<RsGxsMessageId> &versions, const RsGxsMessageId &msgId, const QString &title);

	static void testFeeds(uint notifyFlags);
	static void testFeed(FeedNotify *feedNotify);

	virtual void updateDisplay();

signals:
	void newsFeedChanged(int count);

protected:
	void processSettings(bool load);

	/* TokenResponse */
	virtual void loadRequest(const TokenQueue *queue, const TokenRequest &req);

private slots:
//	void toggleChanMsgItems(bool on);
	void feedoptions();
	void sortChanged(int index);

	void sendNewsFeedChanged();

private:
	void addFeedItem(FeedItem *item);
	void addFeedItemIfUnique(FeedItem *item, int itemType, const std::string& id1, const std::string& id2, const std::string& id3, const std::string& id4, bool replace);
	void remUniqueFeedItem(FeedItem *item, int itemType, const std::string& id1, const std::string& id2, const std::string& id3, const std::string& id4);

	void addFeedItemPeerConnect(const RsFeedItem &fi);
	void addFeedItemPeerDisconnect(const RsFeedItem &fi);
	void addFeedItemPeerHello(const RsFeedItem &fi);
	void addFeedItemPeerNew(const RsFeedItem &fi);
	void addFeedItemPeerOffset(const RsFeedItem &fi);

	void addFeedItemSecurityConnectAttempt(const RsFeedItem &fi);
	void addFeedItemSecurityAuthDenied(const RsFeedItem &fi);
	void addFeedItemSecurityUnknownIn(const RsFeedItem &fi);
	void addFeedItemSecurityUnknownOut(const RsFeedItem &fi);
	void addFeedItemSecurityIpBlacklisted(const RsFeedItem &fi, bool isTest);
	void addFeedItemSecurityWrongExternalIpReported(const RsFeedItem &fi, bool isTest);

	void addFeedItemChannelNew(const RsFeedItem &fi);
//	void addFeedItemChannelUpdate(const RsFeedItem &fi);
	void addFeedItemChannelMsg(const RsFeedItem &fi);
	void addFeedItemChannelPublishKey(const RsFeedItem &fi);

    void addFeedItemChatPublishKey(const RsFeedItem &fi);
    void addFeedItemChatNew(const RsFeedItem &fi);
    void addFeedItemChatMsg(const RsFeedItem &fi);


	void addFeedItemForumNew(const RsFeedItem &fi);
//	void addFeedItemForumUpdate(const RsFeedItem &fi);
	void addFeedItemForumMsg(const RsFeedItem &fi);

	void addFeedItemPostedNew(const RsFeedItem &fi);
//	void addFeedItemPostedUpdate(const RsFeedItem &fi);
	void addFeedItemPostedMsg(const RsFeedItem &fi);

#if 0
	void addFeedItemBlogNew(const RsFeedItem &fi);
	void addFeedItemBlogMsg(const RsFeedItem &fi);
#endif

	void addFeedItemChatNew(const RsFeedItem &fi, bool addWithoutCheck);
	void addFeedItemMessage(const RsFeedItem &fi);
	void addFeedItemFilesNew(const RsFeedItem &fi);

	void addFeedItemCircleMembReq(const RsFeedItem &fi);
	void remFeedItemCircleMembReq(const RsFeedItem &fi);
	void addFeedItemCircleInvitRec(const RsFeedItem &fi);

	virtual void loadChannelGroup(const uint32_t &token);
	virtual void loadChannelPost(const uint32_t &token);
	virtual void loadChannelPublishKey(const uint32_t &token);

	virtual void loadForumGroup(const uint32_t &token);
	virtual void loadForumMessage(const uint32_t &token);
	virtual void loadForumPublishKey(const uint32_t &token);

	virtual void loadPostedGroup(const uint32_t &token);
	virtual void loadPostedMessage(const uint32_t &token);

	virtual void loadCircleGroup(const uint32_t &token);
	virtual void loadCircleMessage(const uint32_t &token);

private:
	TokenQueue *mTokenQueueChannel;
	TokenQueue *mTokenQueueCircle;
	TokenQueue *mTokenQueueForum;
	TokenQueue *mTokenQueuePosted;
    TokenQueue *mTokenQueueChat;

	/* UI - from Designer */
	Ui::NewsFeed *ui;
};

#endif
