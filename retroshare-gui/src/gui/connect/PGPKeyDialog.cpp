/****************************************************************
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

#include "PGPKeyDialog.h"

#include <QMessageBox>
#include <QDateTime>
#include <QMenu>
#include <QClipboard>
#include <QMap>

#include <iostream>

#include <retroshare/rspeers.h>
#include <retroshare/rsdisc.h>
#include <retroshare/rsmsgs.h>

#include "gui/help/browser/helpbrowser.h"
#include "gui/common/PeerDefs.h"
#include "gui/common/StatusDefs.h"
#include "gui/RetroShareLink.h"
#include "gui/notifyqt.h"
#include "gui/common/AvatarDefs.h"
#include "gui/MainWindow.h"
#include "mainpage.h"
#include "util/DateTime.h"
#include "util/misc.h"

static QMap<RsPgpId, PGPKeyDialog*> instances_pgp;

PGPKeyDialog *PGPKeyDialog::instance(const RsPgpId& pgp_id)
{

    PGPKeyDialog *d = instances_pgp[pgp_id];
    if (d) {
        return d;
    }

    d = new PGPKeyDialog(RsPeerId(),pgp_id);
    instances_pgp[pgp_id] = d;

    return d;
}
/** Default constructor */
PGPKeyDialog::PGPKeyDialog(const RsPeerId& id, const RsPgpId &pgp_id, QWidget *parent, Qt::WindowFlags flags)
  : QDialog(parent, flags), peerId(id), pgpId(pgp_id)
{
    /* Invoke Qt Designer generated QObject setup routine */
    ui.setupUi(this);
	Settings->loadWidgetInformation(this);
//	 if(id.isNull())
//		 ui._useOldFormat_CB->setChecked(true) ;
//	 else
//	 {
//		 ui._useOldFormat_CB->setChecked(false) ;
//		 ui._useOldFormat_CB->setEnabled(false) ;
//	 }

    ui.headerFrame->setHeaderImage(QPixmap(":/home/img/User/information-button_64.png"));
    ui.headerFrame->setHeaderText(tr("UnseenP2P profile"));

    //ui._chat_CB->hide() ;

	setAttribute(Qt::WA_DeleteOnClose, true);

    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(applyDialog()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(close()));
    connect(ui.make_friend_button, SIGNAL(clicked()), this, SLOT(makeFriend()));
    connect(ui.denyFriendButton, SIGNAL(clicked()), this, SLOT(denyFriend()));
    connect(ui.signKeyButton, SIGNAL(clicked()), this, SLOT(signGPGKey()));
    //connect(ui.trusthelpButton, SIGNAL(clicked()), this, SLOT(showHelpDialog()));
    //connect(ui._shouldAddSignatures_CB, SIGNAL(toggled(bool)), this, SLOT(loadInvitePage()));
    connect(ui._shouldAddSignatures_CB_2, SIGNAL(toggled(bool)), this, SLOT(loadKeyPage()));

    //ui.avatar->setFrameType(AvatarWidget::NORMAL_FRAME);

    MainWindow *w = MainWindow::getInstance();
    if (w) {
        connect(this, SIGNAL(configChanged()), w->getPage(MainWindow::Network), SLOT(insertConnect()));
    }
}

PGPKeyDialog::~PGPKeyDialog()
{
	Settings->saveWidgetInformation(this);
        QMap<RsPgpId, PGPKeyDialog*>::iterator it = instances_pgp.find(pgpId);
        if (it != instances_pgp.end())
            instances_pgp.erase(it);
}


//void PGPKeyDialog::setServiceFlags()
//{
//    ServicePermissionFlags flags(0) ;
//
//    if(  ui._direct_transfer_CB->isChecked()) flags = flags | RS_NODE_PERM_DIRECT_DL ;
//    if(  ui._allow_push_CB->isChecked()) flags = flags | RS_NODE_PERM_ALLOW_PUSH ;
//
//    rsPeers->setServicePermissionFlags(pgpId,flags) ;
//}

void PGPKeyDialog::loadAll()
{
    for(QMap<RsPgpId , PGPKeyDialog*>::iterator it = instances_pgp.begin(); it != instances_pgp.end(); ++it)  it.value()->load();
}

void PGPKeyDialog::load()
{
    RsPeerDetails detail;

    if(!rsPeers->getGPGDetails(pgpId, detail))
    {
        QMessageBox::information(this,
                                 tr("UnseenP2P"),
                                 tr("Error : cannot get peer details."));
        close();
        return;
    }

    if(!rsPeers->isKeySupported(pgpId))
    {
        ui.make_friend_button->setEnabled(false) ;
        ui.make_friend_button->setToolTip(tr("The supplied key algorithm is not supported by UnseenP2P\n(Only RSA keys are supported at the moment)")) ;
    }
    else
    {
        ui.make_friend_button->setEnabled(true) ;
        ui.make_friend_button->setToolTip("") ;
    }

    ui.name->setText(QString::fromUtf8(detail.name.c_str()));
    ui.pgpfingerprint->setText(misc::fingerPrintStyleSplit(QString::fromStdString(detail.fpr.toStdString())));

    ui.pgpfingerprint->show();
    ui.pgpfingerprint_label->show();

    ui._direct_transfer_CB->setChecked(  detail.service_perm_flags & RS_NODE_PERM_DIRECT_DL ) ;
	//Add warning to direct source checkbox depends general setting.
	switch (rsFiles->filePermDirectDL())
	{
		case RS_FILE_PERM_DIRECT_DL_YES:
			ui._direct_transfer_CB->setIcon(QIcon(":/icons/warning_yellow_128.png"));
			ui._direct_transfer_CB->setToolTip(ui._direct_transfer_CB->toolTip().append(tr("\nWarning: In your File-Transfer option, you select allow direct download to Yes.")));
			break ;
		case RS_FILE_PERM_DIRECT_DL_NO:
			ui._direct_transfer_CB->setIcon(QIcon(":/icons/warning_yellow_128.png"));
			ui._direct_transfer_CB->setToolTip(ui._direct_transfer_CB->toolTip().append(tr("\nWarning: In your File-Transfer option, you select allow direct download to No.")));
			break ;

		default: break ;
	}
    ui._allow_push_CB->setChecked(  detail.service_perm_flags & RS_NODE_PERM_ALLOW_PUSH) ;
    ui._require_WL_CB->setChecked(  detail.service_perm_flags & RS_NODE_PERM_REQUIRE_WL) ;

    uint32_t max_upload_speed = 0 ;
    uint32_t max_download_speed = 0 ;
    
    rsPeers->getPeerMaximumRates(pgpId,max_upload_speed,max_download_speed);
    
    ui.maxUploadSpeed_SB->setValue(max_upload_speed) ;
    ui.maxDownloadSpeed_SB->setValue(max_download_speed) ;
    
    if (detail.gpg_id == rsPeers->getGPGOwnId())
    {
        ui.make_friend_button->hide();
        ui.signGPGKeyCheckBox->hide();
        ui.signKeyButton->hide();
        ui.denyFriendButton->hide();

        ui.web_of_trust_label->hide();
        ui.trustlevel_CB->hide();

        ui.is_signing_me->hide();
        ui.signersBox->setTitle(tr("This is your own PGP key, and it is signed by :")+" ");

    }
    else
    {
        ui.web_of_trust_label->show();
        ui.trustlevel_CB->show();
        ui.is_signing_me->show();
        ui.signersBox->setTitle(tr("This key is signed by :")+" ");

        if (detail.accept_connection)
        {
            ui.make_friend_button->hide();
            ui.denyFriendButton->show();
            ui.signGPGKeyCheckBox->hide();
            //connection already accepted, propose to sign gpg key
            if (!detail.ownsign) {
                ui.signKeyButton->show();
            } else {
                ui.signKeyButton->hide();
            }
        }
        else
        {
            ui.make_friend_button->show();
            ui.denyFriendButton->hide();
            ui.signKeyButton->hide();
            if (!detail.ownsign) {
                ui.signGPGKeyCheckBox->show();
                ui.signGPGKeyCheckBox->setChecked(false);
            } else {
                ui.signGPGKeyCheckBox->hide();
            }
        }

        //web of trust

            ui.trustlevel_CB->setCurrentIndex(detail.trustLvl) ;


    QString truststring = "<p>" ;
    truststring += tr("The trust level is a way to express your own trust in this key. It is not used by the software nor shared, but can be useful to you in order to remember good/bad keys.") ;
    truststring += "</p>" ;
    truststring += "<p>" ;
        switch(detail.trustLvl)
    {
    case  RS_TRUST_LVL_ULTIMATE:
        //trust is ultimate, it means it's one of our own keys
        truststring += tr("Your trust in this peer is ultimate");
        break ;
    case RS_TRUST_LVL_FULL:
        truststring += tr("Your trust in this peer is full.");
        break ;
    case RS_TRUST_LVL_MARGINAL:
        truststring += tr("Your trust in this peer is marginal.");
        break ;
    case RS_TRUST_LVL_NEVER:
        truststring += tr("Your trust in this peer is none.");
        break ;

    default:
        truststring += tr("You haven't set a trust level for this key.");
        break ;
    }
    truststring += "</p>" ;
        ui.trustlevel_CB->setToolTip(truststring) ;

        if (detail.hasSignedMe) {
            ui.is_signing_me->setText(tr("This key has signed your own PGP key"));
            ui.is_signing_me->show() ;
        } else {
            ui.is_signing_me->hide() ;
        }
    }

    QString text;
    RetroShareLink link ;

    for(std::list<RsPgpId>::const_iterator it(detail.gpgSigners.begin());it!=detail.gpgSigners.end();++it) {
        link = RetroShareLink::createPerson(*it);
        if (link.valid()) {
            text += link.toHtml() + "<BR>";
        }
    }
    ui.signers->setHtml(text);

    loadKeyPage() ;
}

void PGPKeyDialog::loadKeyPage()
{
    RsPeerDetails detail;

    if (!rsPeers->getGPGDetails(pgpId,detail))
    {
        QMessageBox::information(this,
                                 tr("UnseenP2P"),
                                 tr("Error : cannot get peer details."));
        close();
        return;
    }
    ui._shouldAddSignatures_CB_2->setEnabled(detail.gpgSigners.size() > 1) ;

    //if (!rsPeers->getGPGDetails(pgpId, detail))
    std::list<RsPeerId> sslIds;
    RsPeerId sslId;
    //try to replace getAssociatedSSLIds by another function to get sslId

    //rsPeers->getAssociatedSSLIds(detail.gpg_id, sslIds);
    if (sslIds.size() >= 1) {

             sslId  = sslIds.front();
             std::cerr << "DEBUG: sslId of peer id: " << sslId << std::endl ;
    }
    peerId = sslId;
     std::string pgp_key = rsPeers->getPGPKey(detail.gpg_id,ui._shouldAddSignatures_CB_2->isChecked()) ; // this needs to be a SSL id

    ui.userCertificateText_2->setReadOnly(true);
    ui.userCertificateText_2->setMinimumHeight(200);
    ui.userCertificateText_2->setMinimumWidth(530);
    QFont font("Courier New",10,50,false);
    font.setStyleHint(QFont::TypeWriter,QFont::PreferMatch);
    font.setStyle(QFont::StyleNormal);
    ui.userCertificateText_2->setFont(font);
    ui.userCertificateText_2->setText(QString::fromUtf8(pgp_key.c_str()));

    QString helptext ;
    helptext += tr("<p>This PGP key (ID=")+detail.gpg_id.toStdString().c_str()+")" + " authenticates one or more UnseenP2P nodes.</p> ";

    helptext += "<p>" ;
    if(detail.accept_connection)
        helptext += tr("You have chosen to add friend (accept connections) from UnseenP2P nodes signed by this key.") ;
    else
        helptext += tr("You are currently not removing friend (allowing connections) from UnseenP2P nodes signed by this key.") ;

    helptext += "</p>" ;

    ui.userCertificateText_2->setToolTip(helptext) ;
}

void PGPKeyDialog::applyDialog()
{
    std::cerr << "PGPKeyDialog::applyDialog() called" << std::endl ;
    RsPeerDetails detail;
    if (!rsPeers->getPeerDetails(peerId, detail))
    {
        if (!rsPeers->getGPGDetails(pgpId, detail)) {
            QMessageBox::information(this,
                                     tr("UnseenP2P"),
                                     tr("Error : cannot get peer details."));
            close();
            return;
        }
    }

    //check the GPG trustlvl
    if(ui.trustlevel_CB->currentIndex() != (int)detail.trustLvl)
        rsPeers->trustGPGCertificate(pgpId, ui.trustlevel_CB->currentIndex());

    uint32_t max_upload_speed = ui.maxUploadSpeed_SB->value() ;
    uint32_t max_download_speed = ui.maxDownloadSpeed_SB->value();

    rsPeers->setPeerMaximumRates(pgpId,max_upload_speed,max_download_speed);

    ServicePermissionFlags flags(0) ;

    if(  ui._direct_transfer_CB->isChecked()) flags = flags | RS_NODE_PERM_DIRECT_DL ;
    if(  ui._allow_push_CB->isChecked()) flags = flags | RS_NODE_PERM_ALLOW_PUSH ;
    if(  ui._require_WL_CB->isChecked()) flags = flags | RS_NODE_PERM_REQUIRE_WL ;

    rsPeers->setServicePermissionFlags(pgpId,flags) ;
    
     //setServiceFlags() ;

    loadAll();
    close();
}

void PGPKeyDialog::makeFriend()
{
    if (ui.signGPGKeyCheckBox->isChecked()) {
        rsPeers->signGPGCertificate(pgpId);
    } 

    //Add friend first, then set location, ip go after
    //we can get peerId here for add friend!!!
    //std::map<RsPgpId, RsPeerId> list = rsPeers->friendListOfContact();
    //std::map<RsPgpId, RsPeerId>::iterator it;

    std::map<RsPgpId, UnseenNetworkContactsItem> rsDetailsList = rsPeers->networkContacts();
    std::map<RsPgpId, UnseenNetworkContactsItem>::iterator itDetails;

    itDetails = rsDetailsList.find(pgpId);
    if (itDetails != rsDetailsList.end() )
    {
        peerId = itDetails->second.id;
        std::cerr << "we can find you sslId from RsPeerDetails network contact map: " << peerId << std::endl;;
    }
    else
    {
        std::cerr << "we CAN NOT find you in RsPeerDetails network contact map " << std::endl; ;
    }
//    it = list.find(pgpId);
//    if (it != list.end())
//    {
//        peerId = it->second;
//        std::cerr << "we can find you sslId: " << peerId << ") is Friend of contact  ";
//        std::cerr << std::endl;

//    }
//    else
//    {
//        std::cerr << "we CAN NOT find you is Friend of contact  ";
//        std::cerr << std::endl;
//    }

    //unseenp2p - set option of addFriend when user click on Make Friend button, do not broadcast this anymore
    rsPeers->setAddFriendOption(ADDFRIEND_PGPKEY_DIALOG_MAKE_FRIEND);

    rsPeers->addFriend(peerId, pgpId);

    //at first we parse the cert str of this peer
    std::map<RsPgpId, std::string> certlist = rsPeers->certListOfContact();
    std::map<RsPgpId, std::string>::iterator cert_it;
    std::string certstr;
     cert_it = certlist.find(pgpId);
     if (cert_it != certlist.end())
     {
        certstr = cert_it->second;
        uint32_t cert_error_code;
        RsPeerDetails peerDetails;
        if (rsPeers->loadDetailsFromStringCert(certstr, peerDetails, cert_error_code))
        {
           if (peerDetails.isHiddenNode)
           {
               std::cerr << "PGPKeyDialog::makeFriend() : setting hidden node." << std::endl;
               rsPeers->setHiddenNode(peerDetails.id, peerDetails.hiddenNodeAddress, peerDetails.hiddenNodePort);
           }
           else
           {
               if (!peerDetails.location.empty()) {
                    std::cerr << "PGPKeyDialog::makeFriend() : setting location." << std::endl;
                   rsPeers->setLocation(peerDetails.id, peerDetails.location);
               }
               //let's check if there is ip adresses in the wizard.
               if (!peerDetails.extAddr.empty() && peerDetails.extPort) {
                   std::cerr << "PGPKeyDialog::makeFriend() : setting ip ext address." << std::endl;
                   rsPeers->setExtAddress(peerDetails.id, peerDetails.extAddr, peerDetails.extPort);
               }
               if (!peerDetails.localAddr.empty() && peerDetails.localPort) {
                   std::cerr << "PGPKeyDialog::makeFriend() : setting ip local address." << std::endl;
                   rsPeers->setLocalAddress(peerDetails.id, peerDetails.localAddr, peerDetails.localPort);
               }
               if (!peerDetails.dyndns.empty()) {
                   std::cerr << "PGPKeyDialog::makeFriend() : setting DynDNS." << std::endl;
                   rsPeers->setDynDNS(peerDetails.id, peerDetails.dyndns);
               }
               for(auto&& ipr : peerDetails.ipAddressList)
                   rsPeers->addPeerLocator(
                               peerDetails.id,
                               RsUrl(ipr.substr(0, ipr.find(' '))) );
           }
        }
     }



//	 setServiceFlags() ;
    loadAll();

    emit configChanged();
}

void PGPKeyDialog::denyFriend()
{
    rsPeers->removeFriend(pgpId);
    loadAll();

    emit configChanged();
}

void PGPKeyDialog::signGPGKey()
{
    if (!rsPeers->signGPGCertificate(pgpId)) {
                 QMessageBox::warning ( NULL,
                                tr("Signature Failure"),
                                tr("Maybe password is wrong"),
                                QMessageBox::Ok);
    }
    loadAll();

    emit configChanged();
}

/** Displays the help browser and displays the most recently viewed help
 * topic. */
void PGPKeyDialog::showHelpDialog()
{
    showHelpDialog("trust");
}

/**< Shows the help browser and displays the given help <b>topic</b>. */
void PGPKeyDialog::showHelpDialog(const QString &topic)
{
    HelpBrowser::showWindow(topic);
}
