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
#include "CslStatusBar.h"
#include "CslSettings.h"
#include "CslGameProcess.h"
#include "engine/CslTools.h"


CslGameConnection::CslGameConnection()
{
    m_locked=false;
    m_playing=false;
    m_info=NULL;
}

CslGameConnection::~CslGameConnection()
{
    Reset();
}

CslGameConnection& CslGameConnection::GetInstance()
{
    static CslGameConnection connection;

    return connection;
}

void CslGameConnection::Reset(CslServerInfo *info)
{
    CslGameConnection& self=GetInstance();

    if (self.m_info)
    {
        wxString s;

        self.m_info->GetGame().GameEnd(s);
        self.m_info->ConnectWait=0;

        if (self.m_locked)
            self.m_info->Lock(false);

        s.Empty();

        if (self.m_playing)
        {
            s=wxString::Format(_("Last played on: '%s (%s)'"),
                               self.m_info->GetBestDescription().c_str(),
                               self.m_info->GetGame().GetName().c_str());

            if (self.m_info->PlayTimeLast>0)
            {
                s<<wxT(" - ")<<_("Play time:")<<wxT(" ");
                s<<FormatSeconds(self.m_info->PlayTimeLast).c_str();
            }
        }

        CslStatusBar::SetText(1,s);
    }

    self.m_locked=false;
    self.m_playing=false;
    self.m_info=info;
    self.m_cmd.Empty();
}

bool CslGameConnection::CountDown()
{
    CslGameConnection& self=GetInstance();

    if (--self.m_info->ConnectWait>0)
    {
        CslStatusBar::SetText(1,wxString::Format(_("Waiting %s for a free slot on '%s (%s)' " \
                              _L_"(press ESC to abort or join another server)"),
                              FormatSeconds(self.m_info->ConnectWait,true,true).c_str(),
                              self.m_info->GetBestDescription().c_str(),
                              self.m_info->GetGame().GetName().c_str()));
        return true;
    }
    else
        Reset();

    return false;
}

bool CslGameConnection::Prepare(CslServerInfo *info,wxInt32 pass)
{
    wxString error;
    wxInt32 mode=CslServerInfo::CSL_CONNECT_DEFAULT;
    CslGameConnection& self=GetInstance();

    if (IsPlaying())
    {
        wxMessageBox(_("You are currently playing, so quit the game and try again."),
                     _("Error"),wxOK|wxICON_ERROR,wxTheApp->GetTopWindow());
        return false;
    }

    Reset(info);

    if (pass>NO_PASS || CSL_SERVER_IS_PASSWORD(self.m_info->MM))
    {
        if (pass==ASK_PASS || info->Password.IsEmpty())
        {
            bool admin=CSL_CAP_CONNECT_ADMIN_PASS(info->GetGame().GetCapabilities());

            CslConnectPassInfo pi(info->Password,info->PasswordAdmin,admin);

            if (CslDlgConnectPass(wxTheApp->GetTopWindow(),&pi).ShowModal()!=wxID_OK)
            {
                Reset();
                return false;
            }

            info->Password=pi.Password;
            info->PasswordAdmin=pi.AdminPassword;
            mode=pi.Admin ? CslServerInfo::CSL_CONNECT_ADMIN_PASS:
                 CslServerInfo::CSL_CONNECT_PASS;
        }
        else
            mode=CslServerInfo::CSL_CONNECT_PASS;
    }
    else
        mode=CslServerInfo::CSL_CONNECT_DEFAULT;

    self.m_cmd=info->GetGame().GameStart(info,mode,error);

    if (self.m_cmd.IsEmpty())
    {
        if (!error.IsEmpty())
            wxMessageBox(error,_("Error"),wxICON_ERROR,wxTheApp->GetTopWindow());
        Reset();
        return false;
    }

    if (!::wxSetWorkingDirectory(info->GetGame().GetClientSettings().GamePath))
    {
        Reset();
        return false;
    }

    return true;
}

bool CslGameConnection::Connect()
{
    CslGameConnection& self=GetInstance();

    if (self.m_info->Players>0 && self.m_info->PlayersMax>0 &&
        self.m_info->Players>=self.m_info->PlayersMax)
    {
        if (IsWaiting())
            return true;

        wxInt32 time=g_cslSettings->waitServerFull;

        CslDlgConnectWait *dlg=new CslDlgConnectWait(wxTheApp->GetTopWindow(),&time);
        wxInt32 ret=dlg->ShowModal();

        if (ret!=wxID_CANCEL)
        {
            self.m_locked=true;
            self.m_info->Lock();

            if (ret==wxID_OK)
            {
                self.m_info->ConnectWait=time;
                return true;
            }
        }
        else
        {
            Reset();
            return false;
        }
    }
    else if (!self.m_locked)
    {
        self.m_locked=true;
        self.m_info->Lock();
    }

    CslGameProcess *process=new CslGameProcess(self.m_info,self.m_cmd);
    if (!::wxExecute(self.m_cmd,wxEXEC_ASYNC,process))
    {
        wxMessageBox(_("Failed to start: ")+self.m_cmd,_("Error"),
                     wxICON_ERROR,wxTheApp->GetTopWindow());
        Reset();
        return false;
    }

    self.m_playing=true;

    self.m_info->ConnectWait=0;
    self.m_info->ConnectedTimes++;
    self.m_info->PlayedLast=wxDateTime::Now().GetTicks();

    CslStatusBar::SetText(1,wxString::Format(_("Connected to: '%s (%s)'"),
                          self.m_info->GetBestDescription().c_str(),
                          self.m_info->GetGame().GetName().c_str()));

    return true;
}
