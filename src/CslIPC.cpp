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

#include "CslIPC.h"

DEFINE_EVENT_TYPE(wxCSL_EVT_IPC)

CslIpcConnection::CslIpcConnection(wxEvtHandler *evtHandler) :
        wxConnection(),
        m_evtHandler(evtHandler)
{
}

bool CslIpcConnection::OnPoke(const wxString& topic,const wxString& item,
                              wxChar *data,int size,wxIPCFormat format)
{
    if (m_evtHandler && format==wxIPC_TEXT && topic==CSL_IPC_TOPIC)
    {
        CslIpcEvent evt(CslIpcEvent::IPC_COMMAND,data);
        wxPostEvent(m_evtHandler,evt);
    }

    return wxConnection::OnPoke(topic,item,data,size,format);
}

bool CslIpcConnection::OnDisconnect()
{
    CslIpcEvent evt(CslIpcEvent::IPC_DISCONNECT);
    wxPostEvent(m_evtHandler,evt);

    return true;
}


CslIpcServer::CslIpcServer(wxEvtHandler *evtHandler) :
        wxServer(),
        m_evtHandler(evtHandler),m_connection(NULL)
{
}

CslIpcServer::~CslIpcServer()
{
    Disconnect();
}

wxConnectionBase *CslIpcServer::OnAcceptConnection(const wxString& topic)
{
    if (!m_connection && topic==CSL_IPC_TOPIC)
    {
        m_connection=new CslIpcConnection(m_evtHandler);
        return m_connection;
    }

    return NULL;
}

void CslIpcServer::Disconnect()
{
    if (m_connection)
    {
        m_connection->Disconnect();
        delete m_connection;
        m_connection=NULL;
    }
}


CslIpcClient::CslIpcClient() : wxClient()
{
    m_connection=NULL;
}

CslIpcClient::~CslIpcClient()
{
    Disconnect();
}

bool CslIpcClient::Connect(const wxString& host,const wxString& service,const wxString& topic)
{
    m_connection=(CslIpcConnection*)MakeConnection(host,service,topic);

    return m_connection!=NULL;
}

wxConnectionBase *CslIpcClient::OnMakeConnection()
{
    return new CslIpcConnection;
}

void CslIpcClient::Disconnect()
{
    if (m_connection)
    {
        m_connection->Disconnect();
        delete m_connection;
        m_connection=NULL;
    }
}
