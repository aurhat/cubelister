/***************************************************************************
 *   Copyright (C) 2007 by Glen Masgai                                     *
 *   mimosius@gmx.de                                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "CslConnectionState.h"
#include "CslStatusBar.h"


bool               CslConnectionState::m_playing=false;
wxInt32            CslConnectionState::m_waitTime=0;
wxInt32            CslConnectionState::m_connectMode=CslServerInfo::CSL_CONNECT_DEFAULT;
CslServerInfo*     CslConnectionState::m_activeInfo=NULL;


void CslConnectionState::Reset()
{
    if (m_activeInfo)
        m_activeInfo->SetWaiting(false);
    m_playing=false;
    m_waitTime=0;
    m_connectMode=CslServerInfo::CSL_CONNECT_DEFAULT;
    m_activeInfo=NULL;
    CslStatusBar::SetText(1,wxEmptyString);
}

void CslConnectionState::CreateWaitingState(CslServerInfo *info,wxInt32 mode,wxInt32 time)
{
    m_activeInfo=info;
    m_waitTime=time;
    m_connectMode=mode;
    info->SetWaiting(true);
}

bool CslConnectionState::CountDown()
{
    if (--m_waitTime==0)
    {
        Reset();
        return false;
    }
    return true;
}

void CslConnectionState::CreatePlayingState(CslServerInfo *info)
{
    m_activeInfo=info;
    m_playing=true;
}

bool CslConnectionState::CreateConnectState(CslServerInfo *info,wxInt32 mode)
{
    if (CslConnectionState::IsPlaying())
    {
        wxMessageBox(_("You are currently playing, so quit the game and try again."),
                     _("Error"),wxOK|wxICON_ERROR,wxTheApp->GetTopWindow());
        return false;
    }

    m_activeInfo=info;
    m_connectMode=mode;

    return true;
}
