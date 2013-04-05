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

#include "Csl.h"

const CslCharEncodingTableEntry CslCharEncodingTable[CSL_NUM_CHAR_ENCODINGS] =
{
    { wxT("UTF-8"),       _("UTF-8")            },
    { wxT("ISO-8859-2"),  _("Central European") },
    { wxT("ISO-8859-3"),  _("Central European") },
    { wxT("cp 1250"),     _("Central European") },
    { wxT("cp 1251"),     _("Cyrillic")         },
    { wxT("ISO-8859-5"),  _("Cyrillic")         },
    { wxT("koi8-r"),      _("Cyrillic")         },
    { wxT("koi8-u"),      _("Cyrillic")         },
    { wxT("cp 1254"),     _("Turkish")          },
    { wxT("ISO-8859-9"),  _("Turkish")          },
    { wxT("ISO-8859-1"),  _("Western European") },
    { wxT("ISO-8859-15"), _("Western European") },
    { wxT("cp 1252"),     _("Western European") }
};

const CslCharEncoding CslCurrentEncoding(wxT(""));
const CslCharEncoding CslUTF8Encoding(wxT("UTF-8"));
const CslCharEncoding CslISO_8859_15Encoding(wxT("ISO-8859-15"));


CslCharEncoding::CslCharEncoding(const wxString& name) :
        m_conv(NULL)
{
    SetEncoding(name);
}

CslCharEncoding::~CslCharEncoding()
{
    if (m_conv)
        delete m_conv;
}

bool CslCharEncoding::SetEncoding(const wxString& name)
{
    m_name.Empty();

    if (m_conv)
    {
        delete m_conv;
        m_conv = NULL;
    }

    if (!name.IsEmpty() && !(m_conv = new wxCSConv(name))->IsOk())
    {
        delete m_conv;
        m_conv = NULL;
    }

    m_name = name;

    return m_conv!=NULL;
}

wxInt32 CslCharEncoding::GetEncodingId() const
{
    loopi(CSL_NUM_CHAR_ENCODINGS)
    {
        if (CslCharEncodingTable[i].Encoding==m_name)
            return i;
    }

    return 0;
}

wxString CslCharEncoding::MB2WX(const char *psz) const
{
    if (!psz || !psz[0])
        return wxEmptyString;

    wxString s;

#if wxUSE_UNICODE_UTF8
    if (!m_conv || this==&CslCurrentEncoding || this==&CslUTF8Encoding)
        s = psz;
    else
        s = wxString(psz, *m_conv);
#else
    wchar_t *wbuf = ToWChar(m_conv ? m_conv : wxConvCurrent, psz);

    if (wbuf)
    {
#if wxUSE_UNICODE
        s = wbuf;
#else
        char *buf = FromWChar(wxConvCurrent, wbuf);

        if (buf)
            s = buf;
#endif // wxUSE_UNICODE
        delete[] wbuf;
    }

    if (s.empty())
    {
        //CSL_LOG_DEBUG("Invalid %s sequence: %s\n",
        //              (const char*)m_name.mb_str(wxConvLocal), psz);
        if (m_conv)
            return CslCurrentEncoding.MB2WX(psz);
    }
#endif // wxUSE_UNICODE_UTF8

    return s;
}

const CslCharBuffer CslCharEncoding::WX2MB(const wxString& str) const
{
#if wxUSE_UNICODE_UTF8
    if (!m_conv || this==&CslCurrentEncoding || this==&CslUTF8Encoding)
        return CslCharBuffer(str.wx_str());
    else
        return CslCharBuffer(str.mb_str(*m_conv));
#else
    const char *buf = NULL;

#if wxUSE_UNICODE
    buf = FromWChar(m_conv ? m_conv : wxConvCurrent, str);
#else
    if (!m_conv)
        return CslCharBuffer(str.c_str());

    wchar_t *wbuf = ToWChar(wxConvCurrent, str);

    if (wbuf)
        buf = FromWChar(m_conv, wbuf);
#endif // wxUSE_UNICODE
    if (buf)
    {
        CslCharBuffer cbuf((const char*)buf);
        delete[] buf;
        return cbuf;
    }
    else
    {
        //CSL_LOG_DEBUG("conversion to %s failed.\n",
        //              (const char*)m_name.mb_str(wxConvLocal));
        if (m_conv)
            return CslCurrentEncoding.WX2MB(str);
    }
#endif // wxUSE_UNICODE_UTF8

    return CslCharBuffer("");
}

char* CslCharEncoding::FromWChar(const wxMBConv *conv, const wchar_t *pwz)
{
    size_t len;
    char *buf = NULL;

    if ((len = conv->FromWChar(NULL, 0, pwz))!=wxCONV_FAILED)
    {
        buf = new char[len];

        if (buf && conv->FromWChar(buf, len, pwz)!=wxCONV_FAILED)
            return buf;
    }

    if (buf)
        delete[] buf;

    return NULL;
}

wchar_t* CslCharEncoding::ToWChar(const wxMBConv *conv, const char *psz)
{
    size_t len;
    wchar_t *wbuf = NULL;

    if ((len = conv->ToWChar(NULL, 0, psz))!=wxCONV_FAILED)
    {
        wchar_t *wbuf = new wchar_t[len];

        if (wbuf && conv->ToWChar(wbuf, len+1, psz)!=wxCONV_FAILED)
            return wbuf;
    }

    if (wbuf)
        delete[] wbuf;

    return NULL;
}

wxString CslCharEncoding::CubeMB2WX(const char *pnz)
{
    wxString ret;
    wxInt32 numu = 0, carry = 0;
    wxInt32 len = strlen(pnz);
    uchar ubuf[4*MAXSTRLEN];

    while (carry<len)
    {
        numu = encodeutf8(ubuf, sizeof(ubuf)-1, &((uchar*)pnz)[carry], len-carry, &carry);
        ubuf[numu] = '\0';
        ret << CslUTF8Encoding.MB2WX((const char*)ubuf);
    }

    return ret;
}
