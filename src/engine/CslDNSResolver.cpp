/***************************************************************************
 *   Copyright (C) 2007-2014 by Glen Masgai                                *
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
#include "CslEngine.h"

DEFINE_EVENT_TYPE(wxCSL_EVT_DNS_RESOLVE)


IMPLEMENT_DYNAMIC_CLASS(CslDNSResolveEvent, wxEvent)
IMPLEMENT_DYNAMIC_CLASS(CslDNSResolver, wxEvtHandler)


class CSL_DLL_ENGINE CslDNSResolverQuery
{
    friend class CslDNSResolver;

    public:
        CslDNSResolverQuery(const wxString& host,
                            const void *clientData = NULL,
                            wxEvtHandler *handler = NULL) :
                m_host(host), m_clientData(clientData), m_evtHandler(handler)
       { }

    private:
        wxString m_host;
        const void *m_clientData;
        wxEvtHandler *m_evtHandler;
};

bool CslDNSResolver::Init(wxEvtHandler *evtHandler)
{
    wxASSERT(m_condition==NULL);

    m_condition = new wxCondition(m_mutex);

    if ((m_ok = Create()==wxTHREAD_NO_ERROR) && evtHandler)
        SetNextHandler(evtHandler);

    return m_ok;
}

wxThread::ExitCode CslDNSResolver::Entry()
{
    m_mutex.Lock();

    while (!m_terminate)
    {
        if (!m_queries.size())
        {
            CSL_LOG_DEBUG("resolver waiting\n");
            m_condition->Wait();
            CSL_LOG_DEBUG("resolver signaled\n");
        }

        if (m_terminate || !m_queries.size())
            continue;

        m_querySection.Enter();
        CslDNSResolverQuery *query = (CslDNSResolverQuery*)m_queries[0];
        m_queries.erase(m_queries.begin());
        m_querySection.Leave();

        m_addr.Clear();
        m_addr.Hostname(query->m_host);

        wxEvtHandler *handler = query->m_evtHandler;
        CslDNSResolveEvent evt(m_addr.IPAddress(), m_addr.Hostname(), query->m_clientData, query->m_evtHandler);

        delete query;

        if (m_terminate)
            break;

        ::wxPostEvent(handler ? handler : this, evt);
    }

    loopv(m_queries) delete (CslDNSResolverQuery*)m_queries[i];
    m_queries.Empty();

    m_mutex.Unlock();

    CSL_LOG_DEBUG("DNS resolver exit\n");

    return 0;
}

void CslDNSResolver::Resolve(const wxString& host, const void* clientData, wxEvtHandler *evtHandler)
{
    CslDNSResolverQuery *packet = new CslDNSResolverQuery(host, clientData, evtHandler);

    m_querySection.Enter();

    if (IsIPV4(host))
        m_queries.push_back(packet);
    else
        m_queries.insert(m_queries.begin(), packet);

    m_querySection.Leave();

    if (m_mutex.TryLock()==wxMUTEX_NO_ERROR)
    {
        m_condition->Signal();
        m_mutex.Unlock();
    }
}

void CslDNSResolver::Terminate()
{
    m_terminate = true;
    wxMutexLocker lock(m_mutex);
    m_condition->Signal();
}
