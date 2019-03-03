/****************************************************************
 *
 *  RetroShare is distributed under the following license:
 *
 *  Copyright (C) 2006,  crypton
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

#include "PopupChatDialog.h"
#include "PopupChatWindow.h"

#include "gui/settings/rsharesettings.h"
#include "gui/settings/RsharePeerSettings.h"
#include "gui/notifyqt.h"

#include <retroshare/rspeers.h>
#include <retroshare/rsiface.h>
#include <retroshare/rsnotify.h>

#include <algorithm>

#include "gui/MainWindow.h"
#include "gui/ChatLobbyWidget.h"
#include "gui/common/AvatarDefs.h"

#define appDir QApplication::applicationDirPath()

#define WINDOW(This) dynamic_cast<PopupChatWindow*>(This->window())

/** Default constructor */
PopupChatDialog::PopupChatDialog(QWidget *parent, Qt::WindowFlags flags)
  : ChatDialog(parent, flags)
{
	/* Invoke Qt Designer generated QObject setup routine */
	ui.setupUi(this);

	manualDelete = false;

//    connect(ui.avatarFrameButton, SIGNAL(toggled(bool)), this, SLOT(showAvatarFrame(bool)));
	connect(ui.actionClearOfflineMessages, SIGNAL(triggered()), this, SLOT(clearOfflineMessages()));
	connect(NotifyQt::getInstance(), SIGNAL(chatStatusChanged(ChatId,QString)), this, SLOT(chatStatusChanged(ChatId,QString)));

}

void PopupChatDialog::init(const ChatId &chat_id, const QString &title)
{
        ChatDialog::init(chat_id, title);
        //22 Sep 2018 - meiyousixin - set one more cId
        cId = chat_id;

        /* Hide or show the avatar frames */
//        showAvatarFrame(false);         //   showAvatarFrame(PeerSettings->getShowAvatarFrame(chat_id));

//        ui.avatarWidget->setId(chat_id);
//        ui.avatarWidget->setFrameType(AvatarWidget::STATUS_FRAME);

//	ui.ownAvatarWidget->setFrameType(AvatarWidget::STATUS_FRAME);
//	ui.ownAvatarWidget->setOwnId();

	ui.chatWidget->addToolsAction(ui.actionClearOfflineMessages);

	// add to window
	PopupChatWindow *window = PopupChatWindow::getWindow(false);
	if (window) {
		window->addDialog(this);
	}

        ChatLobbyWidget *chatLobbyPage = dynamic_cast<ChatLobbyWidget*>(MainWindow::getPage(MainWindow::ChatLobby));
        if (chatLobbyPage) {
            chatLobbyPage->addOne2OneChatPage(this) ;
        }

	// load settings
	processSettings(true);
}

/** Destructor. */
PopupChatDialog::~PopupChatDialog()
{
	// save settings
	processSettings(false);
}

ChatWidget *PopupChatDialog::getChatWidget()
{
	return ui.chatWidget;
}

bool PopupChatDialog::notifyBlink()
{
	return (Settings->getChatFlags() & RS_CHAT_BLINK);
}

void PopupChatDialog::processSettings(bool load)
{
	Settings->beginGroup(QString("PopupChatDialog"));

	if (load) {
		// load settings
	} else {
		// save settings
	}

	Settings->endGroup();
}

void PopupChatDialog::showDialog(uint chatflags)
{
//	PopupChatWindow *window = WINDOW(this);
//	if (window) {
//		window->showDialog(this, chatflags);
//	}
	if (chatflags & RS_CHAT_FOCUS)
	{
		MainWindow::showWindow(MainWindow::ChatLobby);
		dynamic_cast<ChatLobbyWidget*>(MainWindow::getPage(MainWindow::ChatLobby))->setCurrentOne2OneChatPage(this) ;
	}
}

// Called by libretroshare through notifyQt to display the peer's status
//
void PopupChatDialog::chatStatusChanged(const ChatId &chat_id, const QString& statusString)
{
    if (mChatId.isSameEndpoint(chat_id)) {
        ui.chatWidget->updateStatusString(getPeerName(chat_id) + " %1", statusString);
	}
}

void PopupChatDialog::addChatMsg(const ChatMessage &msg)
{
	ChatWidget *cw = getChatWidget();
	if (cw)
	{
	    QDateTime sendTime = QDateTime::fromTime_t(msg.sendTime);
	    QDateTime recvTime = QDateTime::fromTime_t(msg.recvTime);
	    QString message = QString::fromUtf8(msg.msg.c_str());
        QString name = msg.incoming? QString::fromStdString(rsPeers->getGPGName(rsPeers->getGPGId(msg.chat_id.toPeerId()))): getOwnName();
        //QString name = msg.incoming? getPeerName(msg.chat_id): getOwnName();

	    cw->addChatMsg(msg.incoming, name, sendTime, recvTime, message, ChatWidget::MSGTYPE_NORMAL);

        //emit messageP2PReceived(msg.incoming, msg.chat_id,  sendTime, name, message) ;
        emit messageP2PReceived(msg) ;

        if (msg.incoming)
        {
            QTextEdit editor;
            editor.setHtml(message);
            QString notifyMsg =  editor.toPlainText();
            RsPgpId pgpId = rsPeers->getGPGId(msg.chat_id.toPeerId());
            std::string strname = rsPeers->getGPGName(pgpId);
            QPixmap avatar;
            AvatarDefs::getAvatarFromSslId(msg.chat_id.toPeerId(), avatar);
            //if (!avatar.isNull())
            MainWindow::displayContactSystrayMsg(QString::fromStdString(strname), notifyMsg, QIcon(avatar));

        }

	}
}

/**
 Toggles the ToolBox on and off, changes toggle button text
 */
//void PopupChatDialog::showAvatarFrame(bool show)
//{
//    ui.avatarframe->setVisible(show);
//    ui.avatarFrameButton->setChecked(show);

//	if (show) {
//		ui.avatarFrameButton->setToolTip(tr("Hide Avatar"));
//		ui.avatarFrameButton->setIcon(QIcon(":images/hide_toolbox_frame.png"));
//	} else {
//		ui.avatarFrameButton->setToolTip(tr("Show Avatar"));
//		ui.avatarFrameButton->setIcon(QIcon(":images/show_toolbox_frame.png"));
//	}

//    PeerSettings->setShowAvatarFrame(mChatId, show);
//}

void PopupChatDialog::clearOfflineMessages()
{
	manualDelete = true;
    // TODO
#ifdef REMOVE
	rsMsgs->clearPrivateChatQueue(false, peerId);
#endif
	manualDelete = false;
}
