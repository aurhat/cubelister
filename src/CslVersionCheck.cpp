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

#include <wx/protocol/http.h>
#include "CslVersionCheck.h"
#include "engine/CslTools.h"
#include "engine/cube_tools.h"


DEFINE_EVENT_TYPE(wxCSL_EVT_VERSIONCHECK)


CslVersionCheckThread::CslVersionCheckThread(wxEvtHandler *evtHandler) :
        wxThread(wxTHREAD_JOINABLE),
        m_ok(false),m_evtHandler(evtHandler)
{
    m_ok=Create()==wxTHREAD_NO_ERROR;
}

wxThread::ExitCode CslVersionCheckThread::Entry()
{
    wxString *version=NULL;

    wxHTTP http;
    http.SetTimeout(10);

    if (http.Connect(CSL_WEBADDR_STR,80))
    {
        http.SetHeader(wxT("User-Agent"),GetHttpAgent());
        wxInputStream *data=http.GetInputStream(wxT("/latest.txt"));
        wxUint32 code=http.GetResponse();

        if (data)
        {
            if (code==200)
            {
                size_t size=min(data->GetSize(),15);
                if (size)
                {
                    char buf[16]={0};
                    data->Read((void*)buf,size);

                    if (!(strstr(buf,"<html>") ||
                          strstr(buf,"<HTML>")))
                    {
                        char *pend=strpbrk(buf," \t\r\n");
                        if (pend)
                            *pend='\0';
                        version=new wxString(A2U(buf));
                        LOG_DEBUG("version check: %s\n",buf);
                    }
                }
            }
            delete data;
        }
    }

    wxCommandEvent evt(wxCSL_EVT_VERSIONCHECK);
    evt.SetClientData(version);
    wxPostEvent(m_evtHandler,evt);

    LOG_DEBUG("version check: thread exit\n");

    return 0;
}
