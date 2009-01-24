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

#include "CslGameConnection.h"
#include "CslDlgConnectPass.h"
#include "CslDlgConnectWait.h"
#include "CslStatusBar.h"
#include "CslSettings.h"
#include "CslGameProcess.h"
#include "engine/CslTools.h"


bool            CslGameConnection::m_playing=false;
wxInt32         CslGameConnection::m_waitTime=0;
wxInt32         CslGameConnection::m_connectMode=CslServerInfo::CSL_CONNECT_DEFAULT;
CslServerInfo*  CslGameConnection::m_info=NULL;


void CslGameConnection::Reset()
{
    if (m_info)
        m_info->SetWaiting(false);
    m_playing=false;
    m_waitTime=0;
    m_info=NULL;
    m_connectMode=CslServerInfo::CSL_CONNECT_DEFAULT;
    CslStatusBar::SetText(1,wxEmptyString);
}

void CslGameConnection::CountDown()
{
    if (--m_waitTime==0)
    {
        Reset();
        CslStatusBar::SetText(1,wxEmptyString);
    }
    else
        CslStatusBar::SetText(1,wxString::Format(_("Waiting %s for a free slot on \"%s\" " \
                              _L_"(press ESC to abort or join another server)"),
                              FormatSeconds(CslGameConnection::GetWaitTime(),true,true).c_str(),
                              m_info->GetBestDescription().c_str()));
}

bool CslGameConnection::Prepare(CslServerInfo *info,wxInt32 pass)
{
    if (CslGameConnection::IsPlaying())
    {
        wxMessageBox(_("You are currently playing, so quit the game and try again."),
                     _("Error"),wxOK|wxICON_ERROR,wxTheApp->GetTopWindow());
        return false;
    }

    if (pass>NO_PASS || CSL_SERVER_IS_PASSWORD(info->MM))
    {
        if (pass==ASK_PASS || info->Password.IsEmpty())
        {
            bool admin=CSL_CAP_CONNECT_ADMIN_PASS(info->GetGame().GetCapabilities());

            CslConnectPassInfo pi(info->Password,info->PasswordAdmin,admin);

            if (CslDlgConnectPass(wxTheApp->GetTopWindow(),&pi).ShowModal()!=wxID_OK)
                return false;

            info->Password=pi.Password;
            info->PasswordAdmin=pi.AdminPassword;
            m_connectMode=pi.Admin ? CslServerInfo::CSL_CONNECT_ADMIN_PASS:
                          CslServerInfo::CSL_CONNECT_PASS;
        }
        else
            m_connectMode=CslServerInfo::CSL_CONNECT_PASS;
    }
    else
        m_connectMode=CslServerInfo::CSL_CONNECT_DEFAULT;

    if (m_info && m_info->IsWaiting())
    {
        m_info->SetWaiting(false);
        CslStatusBar::SetText(1,wxEmptyString);
    }

    m_info=info;

    return true;
}

bool CslGameConnection::Connect()
{
    if (m_playing || !m_info)
        return false;

    wxWindow *window=wxTheApp->GetTopWindow();

    if (m_info->Players>0 && m_info->Players>=m_info->PlayersMax)
    {
        wxInt32 time=g_cslSettings->waitServerFull;

        CslDlgConnectWait *dlg=new CslDlgConnectWait(window,&time);

        switch (dlg->ShowModal())
        {
            case wxID_OK:
                m_waitTime=time;
                m_info->SetWaiting(true);
                return true;
            case wxID_CANCEL:
                return false;
            default:
                break;
        }
    }

    m_info->SetWaiting(false);

    wxString error;
    wxString cmd=m_info->GetGame().GameStart(m_info,m_connectMode,&error);

    if (cmd.IsEmpty())
    {
        if (!error.IsEmpty())
            wxMessageBox(error,_("Error"),wxICON_ERROR,window);
        return false;
    }

    if (!::wxSetWorkingDirectory(m_info->GetGame().GetClientSettings().GamePath))
        return false;

    m_info->Lock();

    CslGameProcess *process=new CslGameProcess(window,m_info,cmd);
    if (!(::wxExecute(cmd,wxEXEC_ASYNC,process)>0))
    {
        wxMessageBox(_("Failed to start: ")+cmd,_("Error"),wxICON_ERROR,window);
        m_info->Lock(false);
        return false;
    }

    m_playing=true;

    m_info->ConnectedTimes++;
    m_info->PlayLast=wxDateTime::Now().GetTicks();

    return true;
}
