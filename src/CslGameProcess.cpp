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

#include "CslGameProcess.h"
#include "CslDlgOutput.h"
#include "CslToolTip.h"
#include "CslSettings.h"

DEFINE_EVENT_TYPE(wxCSL_EVT_PROCESS)

CslGameProcess* CslGameProcess::m_self=NULL;

CslGameProcess::CslGameProcess(CslServerInfo *info,const wxString& cmd) :
        wxProcess(wxPROCESS_REDIRECT),
        m_info(info),m_cmd(cmd)
{
    m_self=this;

    CslToolTip::ResetTip();
    CslDlgOutput::Reset(m_info->GetBestDescription());

    m_watch.Start(0);
}

void CslGameProcess::OnTerminate(int pid,int code)
{
    m_watch.Pause();

    wxUint32 time=m_watch.Time()/1000;
    if (time>g_cslSettings->minPlaytime)
        m_info->SetLastPlayTime(time);

    // Cube returns with 1 - weird
    if (!m_info->GetGame().ReturnOk(code))
        wxMessageBox(wxString::Format(_("%s returned with code: %d"),m_cmd.c_str(),code),
                     _("Error"),wxICON_ERROR,wxTheApp->GetTopWindow());

    ProcessOutput(INPUT_STREAM);
    ProcessOutput(ERROR_STREAM);

    if (g_cslSettings->autoSaveOutput && !g_cslSettings->gameOutputPath.IsEmpty())
        CslDlgOutput::SaveFile(g_cslSettings->gameOutputPath);

    wxCommandEvent evt(wxCSL_EVT_PROCESS);
    evt.SetClientData(m_info);
    wxPostEvent(wxTheApp->GetTopWindow(),evt);

    m_self=NULL;

    delete this;
}

void CslGameProcess::ProcessOutput(wxInt32 type)
{
    if (!m_self)
        return;

    wxInputStream *stream;

    if (type==INPUT_STREAM)
        stream=m_self->GetInputStream();
    else if (type==ERROR_STREAM)
        stream=m_self->GetErrorStream();
    else
        return;

    if (!stream)
        return;

    while (stream->CanRead())
    {
        stream->Read((void*)m_self->m_buffer,CSL_PROCESS_BUFFER_SIZE-1);

        wxUint32 last=stream->LastRead();
        if (!last)
            break;

        m_self->m_buffer[last]=0;

        //Cube has color codes in it's output
        m_self->m_info->GetGame().ProcessOutput(m_self->m_buffer,&last);
        CslDlgOutput::AddOutput(m_self->m_buffer,last);
        //LOG_DEBUG("%s",buf);
    }
}
