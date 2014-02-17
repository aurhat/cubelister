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

#ifndef CSLDNSRESOLVER_H
#define CSLDNSRESOLVER_H

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EXPORTED_EVENT_TYPE(CSL_DLL_ENGINE, wxCSL_EVT_DNS_RESOLVE, wxID_ANY)
END_DECLARE_EVENT_TYPES()

class CslDNSResolveEvent;

typedef void (wxEvtHandler::*CslDNSResolveEventFunction)(CslDNSResolveEvent&);

#define CslDNSResolveEventHandler(fn) \
    (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(CslDNSResolveEventFunction, &fn)

#define CSL_EVT_DNS_RESOLVE(fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
                               wxCSL_EVT_DNS_RESOLVE, wxID_ANY, wxID_ANY, \
                               CslDNSResolveEventHandler(fn), \
                               (wxObject*)NULL \
                             ),


class CslDNSResolveEvent : public wxEvent
{
    public:
        CslDNSResolveEvent(const wxString& ip,
                           const wxString& domain,
                           const void* clientData = NULL,
                           wxEvtHandler *evtHandler = NULL) :
                wxEvent(wxID_ANY, wxCSL_EVT_DNS_RESOLVE),
                m_ip(ip), m_domain(domain),
                m_clientData(clientData),
                m_evtHandler(evtHandler)
        { }

        virtual wxEvent* Clone() const
        {
            return new CslDNSResolveEvent(*this);
        }

        wxString GetIP() const { return m_ip; }
        wxString GetDomain() const { return m_domain; }
        wxEvtHandler* GetEvtHandler() const { return m_evtHandler; }
        const void* GetClientData() const { return m_clientData; }

    private:
        wxString m_ip;
        wxString m_domain;
        const void *m_clientData;
        wxEvtHandler *m_evtHandler;

    private:
        CslDNSResolveEvent() { }

        DECLARE_DYNAMIC_CLASS_NO_ASSIGN(CslDNSResolveEvent)
};


class CslDNSResolver : public wxEvtHandler, public wxThread
{
    public:
        CslDNSResolver(wxEvtHandler *evtHandler = NULL) :
                wxThread(wxTHREAD_JOINABLE),
                m_ok(false),
                m_terminate(false),
                m_condition(NULL)
            { Init(evtHandler); }

        ~CslDNSResolver()
            { if (m_condition) delete m_condition; }

        bool IsOk() { return m_ok; }
        bool Init(wxEvtHandler *evtHandler);
        void Terminate();

        void Resolve(const wxString& host, const void* clientData = NULL, wxEvtHandler *evtHandler = NULL);

    protected:
        wxThread::ExitCode Entry();

    private:
        bool m_ok;
        bool m_terminate;
        wxIPV4address m_addr;
        wxMutex m_mutex;
        wxCondition *m_condition;
        wxCriticalSection m_querySection;
        wxArrayPtrVoid m_queries;

        DECLARE_DYNAMIC_CLASS_NO_COPY(CslDNSResolver)
};

#endif // CSLDNSRESOLVER_H
