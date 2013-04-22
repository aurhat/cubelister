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

#ifndef CSLUDP_H
#define CSLUDP_H

#define CSL_MAX_PACKET_SIZE 5000
#define CSL_UDP_OVERHEAD    42

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EXPORTED_EVENT_TYPE(CSL_DLL_ENGINE, wxCSL_EVT_PING,wxID_ANY)
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
    friend class CslUDP;
    friend class CslEngine;

    public:
        CslNetPacket(wxUint32 size = 0, void *data = NULL,
                     const CslIPV4Addr *addr = NULL) :
            m_data(data), m_size(size)
        {
            if (addr)
                m_addr = *addr;
        }
        ~CslNetPacket() { }

        static CslNetPacket* Create(wxUint32 size = 0, void *data = NULL,
                                    const CslIPV4Addr *addr = NULL)
        {
            return new CslNetPacket(size, data, addr);
        }

        static void Destroy(CslNetPacket *packet, bool free = false)
        {
            packet->Destroy(free);
        }

        static void* Alloc(CslNetPacket *packet, wxUint32 size)
        {
            return packet->Alloc(size);
        }

        static void Free(CslNetPacket *packet)
        {
            packet->Free();
        }

        void Destroy(bool free = false)
        {
            if (free)
                Free();
            delete this;
        }

        void* Alloc(wxUint32 size)
        {
            if (size)
            {
                if ((m_data = calloc(1, size)))
                    m_size = size;
            }
            return m_data;
        }

        void Free()
        {
            if (m_data)
            {
                free(m_data);
                m_size = 0;
                m_data = NULL;
            }
        }

        void SetSize(wxUint32 size) { m_size = size; }
        void SetAddr(const CslIPV4Addr& addr) { m_addr = addr; }

        void* Data() const { return m_data; }
        wxUint32 Size() const { return m_size; }
        const CslIPV4Addr& Address() const { return m_addr; }

    private:
        void *m_data;
        wxUint32 m_size;
        CslIPV4Addr m_addr;
};


enum { CSL_UDP_TRAFFIC_IN, CSL_UDP_TRAFFIC_OUT };

class CSL_DLL_ENGINE CslUDP : public wxEvtHandler
{
    public:
        CslUDP(wxEvtHandler *evtHandler);
        ~CslUDP();

        bool IsOk() const
        {
            return m_socket ? m_socket->IsOk() : false;
        }

        bool Send(const CslNetPacket& packet);

        static wxUint32 GetTraffic(wxUint32 type, bool overhead = false);
        static wxUint32 GetPacketCount(wxUint32 type);

        const wxChar* GetSocketError(wxInt32 code) const;

    private:
        wxEvtHandler *m_evtHandler;
        wxDatagramSocket *m_socket;

        static wxUint32 m_bytesIn;
        static wxUint32 m_bytesOut;
        static wxUint32 m_packetsIn;
        static wxUint32 m_packetsOut;

        void OnSocketEvent(wxSocketEvent& event);

        DECLARE_EVENT_TABLE()
};

#endif // CSLUDP_H
