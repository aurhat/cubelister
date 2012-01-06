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

#ifndef CSLNETADDR_H
#define CSLNETADDR_H

#include <CslTools.h>

class CslIPV4Addr
{
    public:
        CslIPV4Addr()
        {
            Reset();
        }

        CslIPV4Addr(wxUint32 ip, wxInt32 mask=32)
        {
            Create(ip, mask);
        }

        CslIPV4Addr(const CslIPV4Addr& addr)
        {
            *this=addr;
        }

        CslIPV4Addr(const char *addr)
        {
            Create(addr);
        }

        CslIPV4Addr(const wxString& addr)
        {
            Create(U2A(addr));
        }

        CslIPV4Addr& operator=(const CslIPV4Addr& addr)
        {
            m_ip=addr.m_ip;
            m_mask=addr.m_mask;
            return *this;
        }

        bool operator<(const CslIPV4Addr& addr) const
        {
            return wxUINT32_SWAP_ON_LE(m_ip)<wxUINT32_SWAP_ON_LE(addr.m_ip);
        }

        bool operator>(const CslIPV4Addr& addr) const
        {
            return !(*this<addr);
        }

        void Reset() { m_ip=m_mask=0; }
        bool IsOk() const { return m_ip!=0 && m_mask!=0; }

        wxUint32 SetNetmask(wxInt32 bits)
        {
            if (bits==32)
                m_mask=-1;
            else
                m_mask=wxUINT32_SWAP_ON_LE(((1<<bits)-1)<<(32-bits));
            return m_mask;
        }

        wxInt32 GetNetmask() const { return BitCount32(m_mask); }

        wxUint32 GetHostcount() const
        {
            wxInt32 i=GetNetmask();
            return i==32 ? 1 : (1<<(32-i))-2;
        }

        wxUint32 GetIP() const { return m_ip; }

        bool IsInRange(const CslIPV4Addr& addr) const
        {
            return (m_ip&m_mask)==(addr.m_ip&m_mask);
        }

        wxInt32 Create(wxUint32 ip, wxInt32 mask=32)
        {
            m_ip=ip;
            SetNetmask(mask);
            return mask;
        }

        wxInt32 Create(const char *addr);

        wxInt32 Create(const wxString& addr)
        {
            return Create(U2A(addr));
        }

        wxString Format(const wxString& fmt) const;

        static wxInt32 GetMaskFromRange(const CslIPV4Addr& addr1, const CslIPV4Addr& addr2);

    protected:
        wxUint32 m_ip, m_mask;
};

#endif // CSLNETADDR_H
