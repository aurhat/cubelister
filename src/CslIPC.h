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

#ifndef CSLIPC_H
#define CSLIPC_H

#include <wx/ipc.h>

#define CSL_IPC_HOST                wxT("localhost")
#ifdef __WXMSW__
#define CSL_IPC_SERV                wxT("CSL_IPC-") + GetHomeDir(wxPATH_UNIX);
#else
#define CSL_IPC_SERV                wxString(GetHomeDir() + wxT("ipc_sock"))
#endif
#define CSL_IPC_TOPIC               wxT("CSL_IPC_CONTROL")

#define CSL_URI_SCHEME_STR          wxT("csl://")
#define CSL_URI_INFOPORT_STR        wxT("infoport")
#define CSL_URI_GAME_STR            wxT("game")
#define CSL_URI_ACTION_STR          wxT("action")
#define CSL_URI_ACTION_CONNECT_STR  wxT("connect")
#define CSL_URI_ACTION_ADDFAV_STR   wxT("addtofavourites")


class CslIpcEvent;

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_LOCAL_EVENT_TYPE(wxCSL_EVT_IPC, wxID_ANY)
END_DECLARE_EVENT_TYPES()

typedef void (wxEvtHandler::*CslIpcEventEventFunction)(CslIpcEvent&);

#define CSL_EVT_IPC(id,fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
                               wxCSL_EVT_IPC,id,wxID_ANY, \
                               (wxObjectEventFunction)(wxEventFunction) \
                               wxStaticCastEvent(CslIpcEventEventFunction,&fn), \
                               (wxObject*)NULL \
                             ),

class CslIpcEvent : public wxEvent
{
    public:
        enum { IPC_CONNECT, IPC_DISCONNECT, IPC_COMMAND };

        CslIpcEvent(wxInt32 type = -1, const wxString& request = wxT("")) :
                wxEvent(wxID_ANY, wxCSL_EVT_IPC),
                m_type(type), m_request(request)
            { }
        virtual ~CslIpcEvent()
            { }

        virtual wxEvent* Clone() const
            { return new CslIpcEvent(*this); }

        wxInt32 GetType() const { return m_type; }
        const wxString& GetRequest() const { return m_request; }

    protected:
        wxInt32 m_type;
        wxString m_request;

    private:
        DECLARE_DYNAMIC_CLASS_NO_ASSIGN(CslIpcEvent)
};


class CslIpcConnection : public wxConnection
{
    public:
        CslIpcConnection(wxEvtHandler *evtHandler = NULL) :
                m_evtHandler(evtHandler)
        { }
        virtual ~CslIpcConnection()
        { }

    protected:
        virtual bool OnPoke(const wxString& topic,
                            const wxString& item,
#if wxCHECK_VERSION(2, 9, 0)
                            const void *data,
                            size_t size,
#else
                            wxChar *data,
                            int size,
#endif
                            wxIPCFormat format);

        virtual bool OnDisconnect();

        wxEvtHandler *m_evtHandler;
};


class CslIpcBase
{
    public:
        CslIpcBase() : m_connection(NULL) { }

        void Disconnect();

        static wxString CreateURI(const CslServerInfo& info, bool pass, bool connect, bool addfav);

    protected:
        CslIpcConnection *m_connection;
};


class CslIpcServer : public CslIpcBase, public wxServer
{
    public:
        CslIpcServer(wxEvtHandler *evtHandler) :
                m_evtHandler(evtHandler)
            { }
        ~CslIpcServer()
            { Disconnect(); }

    protected:
        wxConnectionBase* OnAcceptConnection(const wxString& topic);

        wxEvtHandler *m_evtHandler;
};


class CslIpcClient: public CslIpcBase, public wxClient
{
    public:
        CslIpcClient() { };
        ~CslIpcClient() { Disconnect(); }

        bool Connect(const wxString& host, const wxString& service, const wxString& topic);
        CslIpcConnection* GetConnection() { return m_connection; };

    protected:
        wxConnectionBase* OnMakeConnection()
            { return new CslIpcConnection; }
};

#endif // CSLIPC_H
