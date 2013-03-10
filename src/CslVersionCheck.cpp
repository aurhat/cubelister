/***************************************************************************
 *   Copyright (C) 2007-2013 by Glen Masgai                                *
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

#include "Csl.h"
#include "CslVersionCheck.h"

DEFINE_EVENT_TYPE(wxCSL_EVT_VERSIONCHECK)

CslVersionCheckThread::CslVersionCheckThread(wxEvtHandler *evtHandler) :
        wxThread(wxTHREAD_JOINABLE),
        m_ok(false), m_terminate(false), m_evtHandler(evtHandler)
{
    m_ok = Create()==wxTHREAD_NO_ERROR;
}

CslVersionCheckThread::~CslVersionCheckThread()
{
    m_mutex.Lock();
    m_terminate = true;
    m_mutex.Unlock();

    if (IsRunning())
        Wait();
}

wxThread::ExitCode CslVersionCheckThread::Entry()
{
    wxHTTP http;
    http.SetTimeout(10);

    wxString version;
    wxString host = wxString(CSL_WEBADDR_STR);

    CSL_LOG_DEBUG("version check: checking %s\n", U2C(host));

    host.Replace(wxT("http://"), wxT(""), false);

    if (http.Connect(host, 80))
    {
        http.SetHeader(wxT("User-Agent"), GetHttpAgent());
        wxInputStream *data = http.GetInputStream(wxT("/latest.txt"));

        if (data)
        {
            if (http.GetResponse()==200)
            {
                char buf[64] = { 0 };
                size_t size = min(data->GetSize(), sizeof(buf)-1);

                if (size)
                {
                    data->Read((void*)buf, size);

                    if (!(strstr(buf, "<html>") || strstr(buf, "<HTML>")))
                    {
                        char *pend = strpbrk(buf, " \t\r\n");

                        if (pend)
                            *pend = '\0';

                        version = C2U(buf);

                        CSL_LOG_DEBUG("version check: %s\n", buf);
                    }
                }
            }

            delete data;
        }
    }
    else
        CSL_LOG_DEBUG("version check: couldn't connect to %s\n", U2C(host));

    m_mutex.Lock();

    if (!m_terminate)
    {
        wxCommandEvent evt(wxCSL_EVT_VERSIONCHECK);
        evt.SetString(version);
        wxPostEvent(m_evtHandler, evt);
    }

    m_mutex.Unlock();

    CSL_LOG_DEBUG("version check: thread exit\n");

    return 0;
}
