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

#ifndef CSLIPC_H
#define CSLIPC_H

/**
    @author Glen Masgai <mimosius@gmx.de>
*/

#include "wx/wxprec.h"
#ifdef __BORLANDC__
#pragma hdrstop
#endif
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif
#include <wx/ipc.h>
#include "engine/CslGame.h"


#define CSL_IPC_HOST wxT("localhost")
#define CSL_IPC_SERV wxT("49152")
#define CSL_IPC_TOPIC wxT("CSL_IPC_CONTROL")

#define CSL_URI_SCHEME_STR          wxT("csl://")
#define CSL_URI_INFOPORT_STR        wxT("infoport")
#define CSL_URI_GAME_STR            wxT("game")
#define CSL_URI_ACTION_STR          wxT("action")
#define CSL_URI_ACTION_CONNECT_STR  wxT("connect")
#define CSL_URI_ACTION_ADDFAV_STR   wxT("addtofavourites")


class CslIpcEvent;

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE(wxCSL_EVT_IPC,wxID_ANY)
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

        CslIpcEvent(wxInt32 type=-1,const wxString& request=wxEmptyString) :
                wxEvent(wxID_ANY,wxCSL_EVT_IPC),
                Type(type),Request(request) {}

        virtual wxEvent* Clone() const
        {
            return new CslIpcEvent(*this);
        }

        wxInt32 Type;
        wxString Request;
};

class CslIpcConnection : public wxConnection
{
    public:
        CslIpcConnection(wxEvtHandler *evtHandler=NULL);

    protected:
        wxEvtHandler *m_evtHandler;

        virtual bool OnPoke(const wxString& topic,const wxString& item,
                            wxChar *data,int size,wxIPCFormat format);
        virtual bool OnDisconnect();
};


class CslIpcBase
{
    public:
        CslIpcBase() : m_connection(NULL) {}
        ~CslIpcBase() {}

        static wxString CreateURI(const CslServerInfo& info,bool pass,bool connect,bool addfav);

    protected:
        CslIpcConnection *m_connection;
};


class CslIpcServer : public CslIpcBase, public wxServer
{
    public:
        CslIpcServer(wxEvtHandler *evtHandler);
        ~CslIpcServer();

        void Disconnect();

    protected:
        wxEvtHandler *m_evtHandler;

        wxConnectionBase* OnAcceptConnection(const wxString& topic);
};


class CslIpcClient: public CslIpcBase, public wxClient
{
    public:
        CslIpcClient();
        ~CslIpcClient();

        bool Connect(const wxString& host,const wxString& service,const wxString& topic);
        void Disconnect();
        CslIpcConnection* GetConnection() { return m_connection; };

    protected:
        wxConnectionBase* OnMakeConnection();
};

#endif // CSLIPC_H
