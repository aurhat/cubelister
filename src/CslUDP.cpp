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

#include "CslUDP.h"
#include "CslTools.h"

DEFINE_EVENT_TYPE(wxCSL_EVT_PING)

BEGIN_EVENT_TABLE(CslUDP,wxEvtHandler)
    EVT_SOCKET(wxID_ANY,CslUDP::OnSocketEvent)
END_EVENT_TABLE()

#define CSL_USE_WIN32_TICK

#ifdef _MSC_VER
#ifndef CSL_USE_WIN32_TICK
#include <time.h>
#if !defined(_WINSOCK2API_) && !defined(_WINSOCKAPI_)
struct timeval
{
    long tv_sec;
    long tv_usec;
};
#endif
int gettimeofday(struct timeval* tv,void *dummy)
{
    union
    {
        long long ns100;
        FILETIME ft;
    } now;

    GetSystemTimeAsFileTime(&now.ft);
    tv->tv_usec = (long)((now.ns100 / 10LL) % 1000000LL);
    tv->tv_sec = (long)((now.ns100 - 116444736000000000LL) / 10000000LL);
    return 0;
}
#endif //USE_WIN32_TICK
#else
#include <sys/time.h>
#endif //_MSC_VER



CslUDP::CslUDP(wxEvtHandler *handler,const wxUint32 interval) :
        wxEvtHandler()
{
    m_init=false;
    m_evtHandler=handler;

#if defined(__WXMSW__) && defined(CSL_USE_WIN32_TICK)
    m_initTicks = GetTickCount();
#else
    struct timeval tv;
    gettimeofday(&tv,NULL);
    m_initTicks=tv.tv_sec*1000+tv.tv_usec/1000-interval;
#endif
    wxIPV4address address;
    address.AnyAddress();

    LOG_DEBUG("Listen port: %d\n",address.Service());
    m_socket=new wxDatagramSocket(address,wxSOCKET_NOWAIT);
    if (!m_socket->IsOk())
    {
        LOG_DEBUG("Couldn't create socket\n");
        delete m_socket;
        m_socket=NULL;
        return;
    }

    m_socket->SetEventHandler(*this);
    m_socket->SetNotify(wxSOCKET_INPUT_FLAG);
    m_socket->Notify(true);

    m_init=true;
}

CslUDP::~CslUDP()
{
    delete m_socket;
}

void CslUDP::OnSocketEvent(wxSocketEvent& event)
{
    if (event.GetSocketEvent()!=wxSOCKET_INPUT)
        return;

    wxIPV4address addr;
    CslUDPPacket *packet=new CslUDPPacket(CSL_MAX_PACKET_SIZE);

    m_socket->RecvFrom(addr,packet->Data(),CSL_MAX_PACKET_SIZE);

    if (m_socket->Error())
    {
        packet->FreeData();
        LOG_DEBUG("Error read\n");
    }

    packet->SetAddr(addr);
    packet->SetSize(m_socket->LastCount());

    wxCommandEvent evt(wxCSL_EVT_PING);
    evt.SetClientData(packet);
    wxPostEvent(m_evtHandler,evt);
}

bool CslUDP::SendPing(CslUDPPacket *packet)
{
    void *data=packet->Data();
    wxUint32 size=packet->Size();
    wxIPV4address addr=packet->Address();

    packet->SetSize(0);
    delete packet;

    m_socket->SendTo(addr,data,size);

    if (m_socket->Error())
    {
        LOG_DEBUG("Error send\n");
        return false;
    }

    return true;
}

wxUint32 CslUDP::GetTicks(const bool init)
{
    if (init)
        return m_initTicks;

#if defined(__WXMSW__) && defined(CSL_USE_WIN32_TICK)
    return GetTickCount()-m_initTicks;
#else
    struct timeval tv;
    gettimeofday(&tv,NULL);
    wxUint32 r=tv.tv_usec/1000;
    wxUint64 v=tv.tv_sec*1000;
    return v+r-m_initTicks;
#endif
}
