/***************************************************************************
 *   Copyright (C) 2007 -2009 by Glen Masgai                               *
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

#include "CslGameProcess.h"
#include "CslDlgOutput.h"
#include "CslSettings.h"

DEFINE_EVENT_TYPE(wxCSL_EVT_PROCESS)

CslGameProcess* CslGameProcess::m_self=NULL;

CslGameProcess::CslGameProcess(wxWindow *parent,CslServerInfo *info,const wxString& cmd) :
        wxProcess(parent),
        m_parent(parent),m_info(info),m_cmd(cmd)
{
    m_self=this;
    m_watch.Start(0);
    Redirect();
}

void CslGameProcess::OnTerminate(int pid,int code)
{
    m_watch.Pause();

    wxUint32 time=m_watch.Time()/1000;
    if (time>g_cslSettings->minPlaytime)
        m_info->SetLastPlayTime(time);

    // Cube returns with 1 - weird
    if (!m_info->GetGame().ReturnOk(code))
        wxMessageBox(wxString::Format(_("%s returned with code: %d"),
                                      m_cmd.c_str(),code),_("Error"),wxICON_ERROR,m_parent);

    ProcessInputStream();
    ProcessErrorStream();

    if (g_cslSettings->autoSaveOutput && !g_cslSettings->gameOutputPath.IsEmpty())
        CslDlgOutput::SaveFile(g_cslSettings->gameOutputPath);

    wxCommandEvent evt(wxCSL_EVT_PROCESS);
    evt.SetClientData(m_info);
    wxPostEvent(m_parent,evt);

    m_self=NULL;

    delete this;
}

void CslGameProcess::ProcessInputStream()
{
    if (!m_self)
        return;

    wxInputStream *stream=m_self->GetInputStream();
    if (!stream)
        return;

    while (stream->CanRead())
    {
        char buf[1025];
        stream->Read((void*)buf,1024);
        wxInt32 last=(wxInt32)stream->LastRead();
        if (last<=0)
            break;
        buf[last]=0;
        //Cube has color codes in it's output
        m_self->m_info->GetGame().ProcessOutput(buf,&last);
        CslDlgOutput::AddOutput(buf,last);
        //LOG_DEBUG("%s",buf);
    }
}

void CslGameProcess::ProcessErrorStream()
{
    if (!m_self)
        return;

    wxInputStream *stream=m_self->GetErrorStream();
    if (!stream)
        return;

    while (stream->CanRead())
    {
        char buf[1025];
        stream->Read((void*)buf,1024);

        wxInt32 last=(wxInt32)stream->LastRead();
        if (last<=0)
            break;
        buf[last]=0;
        /*
        //Cube has color codes in it's output
        m_self->m_info->GetGame().ProcessOutput(buf,&last);
        CslDlgOutput::AddOutput(buf,last);
        */
    }
}

