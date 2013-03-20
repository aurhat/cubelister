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

#ifndef CSLNETADDR_H
#define CSLNETADDR_H

#include <CslTools.h>

class CSL_DLL_TOOLS CslIPV4Addr
{
    public:
        CslIPV4Addr()
            { Reset(); }
        CslIPV4Addr(wxUint32 ip, wxUint16 port = 0, wxUint32 netmask = (wxUint32)-1)
            { Create(ip, port, netmask); }
        CslIPV4Addr(const CslIPV4Addr& addr)
            { *this = addr; }
        CslIPV4Addr(const wxIPV4address& addr)
            { Create(addr); }
        CslIPV4Addr(const char *addr, wxUint16 port = 0, wxUint32 netmask = 0)
            { Create(addr, port, netmask); }
        CslIPV4Addr(const wxString& addr, wxUint16 port = 0, wxUint32 netmask = 0)
            { Create(U2C(addr), port, netmask); }

        bool operator==(const CslIPV4Addr& addr) const
            { return m_ip == addr.m_ip && m_port == addr.m_port; }
        bool operator<(const CslIPV4Addr& addr) const
            { return wxUINT32_SWAP_ON_LE(m_ip)<wxUINT32_SWAP_ON_LE(addr.m_ip); }
        bool operator>(const CslIPV4Addr& addr) const
            { return !(*this<addr); }

        bool Create(const char *addr, wxUint16 port = 0, wxUint32 netmask = 0);
        bool Create(const wxString& addr, wxUint16 port = 0, wxUint32 netmask = 0)
            { return Create(U2C(addr), port, netmask); }
        bool Create(const wxIPV4address& addr)
            { return Create(addr.IPAddress(), addr.Service()); }
        bool Create(wxUint32 ip, wxUint16 port = 0, wxUint32 netmask = (wxUint32)-1)
        {
            m_ip = ip;
            m_port = port;
            m_mask = netmask;
            return (m_ip&m_mask)!=0;
        }

        void Reset() { m_ip = m_mask = 0; }

        bool IsOk() const {    return m_ip!=0 && m_mask!=0; }

        wxUint32 SetIP(wxUint32 ip)    { return (m_ip = ip); }
        wxUint16 SetPort(wxUint16 port)    { return (m_port = port); }
        wxUint32 SetMask(wxUint32 mask)    { return (m_mask = mask); }
        wxUint32 SetMaskBits(wxInt32 bits)
        {
            if (bits==32)
                m_mask = (wxUint32)-1;
            else
                m_mask = wxUINT32_SWAP_ON_LE(((1<<bits)-1)<<(32-bits));
            return m_mask;
        }

        wxUint32 GetIP() const { return m_ip; }
        wxString GetIPString() const { return Format(wxT("%i")); }
        wxUint16 GetPort() const { return m_port; }
        wxUint32 GetMask() const { return m_mask; }
        wxUint32 GetMaskBits() const { return BitCount32(m_mask); }
        wxUint32 GetHostcount() const
        {
            wxInt32 i = GetMaskBits();
            return i==32 ? 1 : (1<<(32-i))-2;
        }

        bool IsInRange(const CslIPV4Addr& addr) const
            { return (m_ip&m_mask)==(addr.m_ip&m_mask); }

        const wxIPV4address TowxIPV4Address() const
        {
            wxIPV4address addr;
            addr.Hostname(GetIPString());
            addr.Service(m_port);
            return addr;
        }

        wxString Format(const wxString& fmt) const;

        static wxInt32 GetMaskFromRange(const CslIPV4Addr& addr1, const CslIPV4Addr& addr2);

    protected:
        wxUint32 m_ip, m_mask;
        wxUint16 m_port;
        wxIPV4address *m_wxIPV4;
};

WX_DEFINE_USER_EXPORTED_ARRAY(CslIPV4Addr*, CslArrayCslIPV4Addr, class CSL_DLL_TOOLS);

#endif // CSLNETADDR_H
