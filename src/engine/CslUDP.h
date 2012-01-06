/***************************************************************************
 *   Copyright (C) 2007-2011 by Glen Masgai                                *
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

#ifndef CSLUDP_H
#define CSLUDP_H

/**
 @author Glen Masgai <mimosius@users.sourceforge.net>
*/

#define CSL_MAX_PACKET_SIZE 5000
#define CSL_UDP_OVERHEAD 42

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE(wxCSL_EVT_PING,wxID_ANY)
END_DECLARE_EVENT_TYPES()

class CslPingEvent;
class CslNetPacket;

typedef void (wxEvtHandler::*CslPingEventFunction)(CslPingEvent&);

#define CSL_EVT_PING(fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
                               wxCSL_EVT_PING, wxID_ANY, wxID_ANY, \
                               (wxObjectEventFunction)(wxEventFunction) \
                               wxStaticCastEvent(CslPingEventFunction, &fn), \
                               (wxObject*)NULL \
                             ),

class CslPingEvent : public wxEvent
{
    public:
        CslPingEvent(CslNetPacket *packet=NULL) :
                wxEvent(wxID_ANY, wxCSL_EVT_PING),
                m_packet(packet) { }

        virtual wxEvent* Clone() const
        {
            return new CslPingEvent(*this);
        }

        CslNetPacket* GetPacket() { return m_packet; }

    private:
        CslNetPacket *m_packet;
};

class CslNetPacket
{
    public:
        CslNetPacket(wxUint32 size=0) { Create(size); }
        CslNetPacket(wxUint32 size, void *data) :
                m_size(size),m_data(data) {};

        ~CslNetPacket() { FreeData(); }

        void* Create(wxUint32 size)
        {
            if (size>0)
            {
                m_size=size;
                return (m_data=calloc(1, size)) ? m_data : NULL;
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

        void Init(const wxIPV4address& addr, void *data, wxUint32 size)
        {
            m_addr=addr;
            m_data=data;
            m_size=size;
        }
        void SetSize(wxUint32 size) { m_size=size; }
        void SetAddr(const wxIPV4address& addr) { m_addr=addr; }

        void* Data() const { return m_data; }
        wxUint32 Size() const { return m_size; }
        const wxIPV4address& Address() const { return m_addr; }

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

        bool IsOk() const { return m_socket ? m_socket->IsOk():false; }
        bool Send(CslNetPacket *packet);
        static wxUint32 GetTraffic(wxUint32 type,bool overhead=false);
        static wxUint32 GetPacketCount(wxUint32 type);
        const wxString GetSocketError(wxInt32 code) const;

    private:
        wxEvtHandler *m_evtHandler;
        wxDatagramSocket *m_socket;
        static wxUint32 m_bytesIn,m_bytesOut,m_packetsIn,m_packetsOut;

        void OnSocketEvent(wxSocketEvent& event);

        DECLARE_EVENT_TABLE()
};


#endif // CSLUDP_H
