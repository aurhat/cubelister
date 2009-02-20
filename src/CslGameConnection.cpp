/***************************************************************************
 *   Copyright (C) 2007-2009 by Glen Masgai                                *
 *   mimosius@users.sourceforge.net                                        *
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
#include "CslDlgOutput.h"
#include "CslStatusBar.h"
#include "CslSettings.h"
#include "CslGameProcess.h"
#include "engine/CslTools.h"


bool            CslGameConnection::m_locked=false;
bool            CslGameConnection::m_playing=false;
CslServerInfo*  CslGameConnection::m_info=NULL;
wxString        CslGameConnection::m_cmd=wxEmptyString;


void CslGameConnection::Reset(CslServerInfo *info)
{
    if (m_info)
    {
        wxString s;

        m_info->GetGame().GameEnd(s);
        m_info->ConnectWait=0;

        if (m_locked)
            m_info->Lock(false);

        s.Empty();

        if (m_playing)
        {
            s=wxString::Format(_("Last played on: '%s (%s)'"),
                               m_info->GetBestDescription().c_str(),
                               m_info->GetGame().GetName().c_str());

            if (m_info->PlayTimeLast>0)
                s<<wxT(" - ")<<_("Play time:")<<wxT(" ")<<FormatSeconds(m_info->PlayTimeLast).c_str();
        }

        CslStatusBar::SetText(1,s);
    }

    m_locked=false;
    m_playing=false;
    m_info=info;
    m_cmd.Empty();
}

void CslGameConnection::CountDown()
{
    if (--m_info->ConnectWait>0)
        CslStatusBar::SetText(1,wxString::Format(_("Waiting %s for a free slot on '%s (%s)' " \
                              _L_"(press ESC to abort or join another server)"),
                              FormatSeconds(m_info->ConnectWait,true,true).c_str(),
                              m_info->GetBestDescription().c_str(),m_info->GetGame().GetName().c_str()));
    else
        Reset();
}

bool CslGameConnection::Prepare(CslServerInfo *info,wxInt32 pass)
{
    wxString error;
    wxInt32 mode=CslServerInfo::CSL_CONNECT_DEFAULT;

    if (IsPlaying())
    {
        wxMessageBox(_("You are currently playing, so quit the game and try again."),
                     _("Error"),wxOK|wxICON_ERROR,wxTheApp->GetTopWindow());
        return false;
    }

    Reset(info);

    if (pass>NO_PASS || CSL_SERVER_IS_PASSWORD(m_info->MM))
    {
        if (pass==ASK_PASS || m_info->Password.IsEmpty())
        {
            bool admin=CSL_CAP_CONNECT_ADMIN_PASS(m_info->GetGame().GetCapabilities());

            CslConnectPassInfo pi(m_info->Password,m_info->PasswordAdmin,admin);

            if (CslDlgConnectPass(wxTheApp->GetTopWindow(),&pi).ShowModal()!=wxID_OK)
            {
                Reset();
                return false;
            }

            m_info->Password=pi.Password;
            m_info->PasswordAdmin=pi.AdminPassword;
            mode=pi.Admin ? CslServerInfo::CSL_CONNECT_ADMIN_PASS:
                 CslServerInfo::CSL_CONNECT_PASS;
        }
        else
            mode=CslServerInfo::CSL_CONNECT_PASS;
    }
    else
        mode=CslServerInfo::CSL_CONNECT_DEFAULT;

    m_cmd=m_info->GetGame().GameStart(m_info,mode,error);

    if (m_cmd.IsEmpty())
    {
        if (!error.IsEmpty())
            wxMessageBox(error,_("Error"),wxICON_ERROR,wxTheApp->GetTopWindow());
        Reset();
        return false;
    }

    if (!::wxSetWorkingDirectory(m_info->GetGame().GetClientSettings().GamePath))
    {
        Reset();
        return false;
    }

    return true;
}

bool CslGameConnection::Connect()
{
    if (m_info->Players>0 && m_info->PlayersMax>0 && m_info->Players>=m_info->PlayersMax)
    {
        if (IsWaiting())
            return true;

        wxInt32 time=g_cslSettings->waitServerFull;

        CslDlgConnectWait *dlg=new CslDlgConnectWait(wxTheApp->GetTopWindow(),&time);

        switch (dlg->ShowModal())
        {
            case wxID_OK:
                m_locked=true;
                m_info->Lock();
                m_info->ConnectWait=time;
                return true;

            case wxID_CANCEL:
            default:
                Reset();
                return false;
        }
    }
    else if (!m_locked)
    {
        m_locked=true;
        m_info->Lock();
    }

    m_info->ConnectWait=0;

    CslDlgOutput::Reset(m_info->GetBestDescription());

    CslGameProcess *process=new CslGameProcess(m_info,m_cmd);
    if (!::wxExecute(m_cmd,wxEXEC_ASYNC,process))
    {
        wxMessageBox(_("Failed to start: ")+m_cmd,_("Error"),wxICON_ERROR,wxTheApp->GetTopWindow());
        Reset();
        return false;
    }

    m_playing=true;

    CslStatusBar::SetText(1,wxString::Format(_("Connected to: '%s (%s)'"),
                          m_info->GetBestDescription().c_str(),
                          m_info->GetGame().GetName().c_str()));

    m_info->ConnectedTimes++;
    m_info->PlayedLast=wxDateTime::Now().GetTicks();

    return true;
}
