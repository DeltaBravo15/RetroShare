/*******************************************************************************
 * gui/statusbar/peerstatus.cpp                                                *
 *                                                                             *
 * Copyright (c) 2008 Retroshare Team <retroshare.project@gmail.com>           *
 *                                                                             *
 * This program is free software: you can redistribute it and/or modify        *
 * it under the terms of the GNU Affero General Public License as              *
 * published by the Free Software Foundation, either version 3 of the          *
 * License, or (at your option) any later version.                             *
 *                                                                             *
 * This program is distributed in the hope that it will be useful,             *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                *
 * GNU Affero General Public License for more details.                         *
 *                                                                             *
 * You should have received a copy of the GNU Affero General Public License    *
 * along with this program. If not, see <https://www.gnu.org/licenses/>.       *
 *                                                                             *
 *******************************************************************************/

#include <QHBoxLayout>
#include <QLabel>

#include "peerstatus.h"

PeerStatus::PeerStatus(QWidget *parent)
 : QWidget(parent)
{
    QHBoxLayout *hbox = new QHBoxLayout(this);
    hbox->setMargin(0);
    hbox->setSpacing(6);
    

    iconLabel = new QLabel( this );
    int S = QFontMetricsF(iconLabel->font()).height();
     iconLabel->setPixmap(QPixmap(":/chat/img/friends_off_un_128.png").scaledToHeight(S,Qt::SmoothTransformation));
    hbox->addWidget(iconLabel);
    
    statusPeers = new QLabel( tr("Trusted nodes: 0/0"), this );
    hbox->addWidget(statusPeers);

    _compactMode = false;

    setLayout(hbox);

}

void PeerStatus::getPeerStatus(unsigned int nFriendCount, unsigned int nOnlineCount)
{
	/* set users/friends/network */

	if (statusPeers){
		statusPeers->setToolTip(tr("Online Trusted nodes/Total trusted nodes") );
		QString text;
		if (_compactMode) text = QString("%1/%2").arg(nOnlineCount).arg(nFriendCount);
		else text = QString("<strong>%1:</strong> %2/%3 ").arg(tr("Trusted nodes")).arg(nOnlineCount).arg(nFriendCount);
		statusPeers -> setText(text);
	}
	int S = QFontMetricsF(iconLabel->font()).height();

	if (nOnlineCount > 0)
        iconLabel->setPixmap(QPixmap(":/chat/img/friends_on_un_128.png").scaledToHeight(S,Qt::SmoothTransformation));
	else
        iconLabel->setPixmap(QPixmap(":/chat/img/friends_off_un_128.png").scaledToHeight(S,Qt::SmoothTransformation));
}
