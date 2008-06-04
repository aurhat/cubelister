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

#ifndef CSLUDP_H
#define CSLUDP_H

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
#include <wx/socket.h>
#include <wx/sckaddr.h>

#define CSL_MAX_PACKET_SIZE 5000
#define CSL_UDP_OVERHEAD 42

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE(wxCSL_EVT_PING,wxID_ANY)
END_DECLARE_EVENT_TYPES()

#define CSL_EVT_PING(id,fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
                               wxCSL_EVT_PING,id,wxID_ANY, \
                               (wxObjectEventFunction)(wxEventFunction) \
                               wxStaticCastEvent(wxCommandEventFunction,&fn), \
                               (wxObject*)NULL \
                             ),

class CslUDPPacket
{
    public:
        CslUDPPacket(const wxUint32 size=0) { Create(size); }
        CslUDPPacket(const wxUint32 size,void *data) :
                m_size(size),m_data(data) {};

        ~CslUDPPacket() { FreeData(); }

        void* Create(const wxUint32 size)
        {
            if (size>0)
            {
                m_size=size;
                m_data=calloc(1,size);
                return m_data;
            }
            m_size=0;
            m_data=NULL;
            return NULL;
        }

        void FreeData()
        {
            if (m_size>0)
            {
                free(m_data);
                m_size=0;
                m_data=NULL;
                return;
            }
        }

        void Set(const wxIPV4address& addr,void *data,const wxUint32 size)
        {
            m_addr=addr;
            m_data=data;
            m_size=size;
        }
        void SetSize(const wxUint32 size) { m_size=size; }
        void SetAddr(const wxIPV4address& addr) { m_addr=addr; }

        void* Data() { return m_data; }
        wxUint32 Size() { return m_size; }
        wxIPV4address& Address() { return m_addr; }

    private:
        wxUint32 m_size;
        void *m_data;
        wxIPV4address m_addr;
};


enum { CSL_UDP_TRAFFIC_IN, CSL_UDP_TRAFFIC_OUT };

class CslUDP : public wxEvtHandler
{
    public:
        CslUDP(wxEvtHandler *evtHandler);
        ~CslUDP();

        bool IsInit() const { return m_init; }
        bool SendPing(CslUDPPacket *packet);
        static wxUint32 GetTraffic(wxUint32 type,bool overhead=false);
        static wxUint32 GetPacketCount(wxUint32 type);

    private:
        bool m_init;
        wxEvtHandler *m_evtHandler;
        wxDatagramSocket *m_socket;
        static wxUint32 m_bytesIn,m_bytesOut,m_packetsIn,m_packetsOut;

        void OnSocketEvent(wxSocketEvent& event);

        DECLARE_EVENT_TABLE()
};


#endif // CSLUDP_H
