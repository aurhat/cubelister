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

#include "Csl.h"

bool CslIPV4Addr::Create(const char *addr, wxUint16 port, wxUint32 netmask)
{
    wxInt32 n;
    char *end = (char*)"0";

    union
    {
        wxUint32 i;
        wxUint8 b[sizeof(wxUint32)];
    } ip, mask;

    ip.i = mask.i = 0;

    Reset();

    m_port = port;

    loopi(5)
    {
        n=strtol(addr, &end, 10);

        if (end>addr)
        {
            if (i<4)
            {
                if (n<0 || n>255)
                    return false;

                ip.b[i] = n;
                mask.b[i] = 0xff;
            }
            else
            {
                if (n<0 || n>32)
                    return false;

                if (!netmask)
                    SetMaskBits(n);
                break;
            }

            if (!end[0])
                break;

            if (end[0]!=(i<3 ? '.' : '/'))
                return false;

            addr=end+1;
        }
        else
            return false;
    }

    if (end[0])
        return false;

    if (!m_mask)
        m_mask = netmask ? netmask : mask.i;

    m_ip = ip.i;

    return (m_ip&m_mask)!=0;
}

wxString CslIPV4Addr::Format(const wxString& fmt) const
{
    wxChar c;
    wxString buf;

    for (wxUint32 i=0; i<fmt.Length(); i++)
    {
        c=fmt.GetChar(i);

        if (c=='%') switch ((c=fmt.GetChar(++i)))
        {
            case 'f': // start address (from)
                buf<<NtoA(m_ip&m_mask);
                break;

            case 'h': // possible hosts
                buf<<wxString::Format(wxT("%d"), GetHostcount());
                break;

            case 'i': // IP
                buf<<NtoA(m_ip);
                break;

            case 'm': // netmask as ip
                buf<<NtoA(m_mask);
                break;

            case 'p': // port
                buf<<wxString::Format(wxT("%d"), m_port);
                break;

            case 's': // netmask bitcount (/x)
                buf<<wxString::Format(wxT("%d"), GetMaskBits());
                break;

            case 't': // to address (end)
                buf<<NtoA(m_ip|(m_mask^-1));
                break;

            default:
                break;
        }
        else
            buf<<c;
    }

    return buf;
}

wxInt32 CslIPV4Addr::GetMaskFromRange(const CslIPV4Addr& addr1, const CslIPV4Addr& addr2)
{
    CslIPV4Addr a1=addr1;
    CslIPV4Addr a2=addr2;

    if (a1.m_ip==a2.m_ip)
        return 32;

    for (wxInt32 i=1; i<32; i++)
    {
        a1.SetMaskBits(i);
        a2.SetMaskBits(i);

        if (!a1.IsInRange(a2))
            return i-1;
    }

    return 0;
}
