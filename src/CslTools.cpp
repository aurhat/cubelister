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

#include <wx/wxprec.h>
#ifdef __BORLANDC__
#pragma hdrstop
#endif
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/regex.h>

char* StripColours(const char *s)
{
    int i=0,j=0;
    int l=strlen(s);

    if (!l)
        return NULL;

    char *buf=strdup(s);

    for (;i<l;i++)
    {
        char c=s[i];
        if (c!=0xc)
        {
            buf[j]=c;
            j++;
        }
        else
            i++;
    }

    buf[j]=0;
    return buf;
}

bool IsIP(const wxString& s)
{
    const wxChar* dot=wxT(".");
    wxString digit=wxT("0*[0-9]{1,3}");
    wxString exp=wxT("^")+digit+dot+digit+dot+digit+dot+digit+wxT("$");
    wxRegEx regex;

    regex.Compile(exp);
    return regex.Matches(s);
}

bool IP2Int(const wxString& s,wxUint32 *ip)
{
    long unsigned int l;
    wxUint32 mult=0x1000000;
    wxUint32 len=s.Len();
    wxUint32 i=0,v=0;
    wxString m;

    for (;i<=len;i++)
    {
        if (i<len && s.Mid(i,1).IsNumber())
            m+=s.Mid(i,1);
        else
        {
            m.ToULong(&l,10);
            if (l>255)
                return false;
            v+=l*mult;
            mult>>=8;
            m.Empty();
        }
    }

    *ip=v;
    return true;
}
