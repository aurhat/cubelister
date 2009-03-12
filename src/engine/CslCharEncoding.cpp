/***************************************************************************
 *   Copyright (C) 2007-2009 by Glen Masgai                                *
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

#include "CslTools.h"

#define CSL_CHAR_ENCODING_DEBUG  (__WXDEBUG__ && 1)

CslCharEncoding CslDefaultCharEncoding;


CslCharEncoding::CslCharEncoding(bool utf8,const wxString& name) :
        m_utf8(utf8),m_conv(NULL)
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
    if (m_conv)
    {
        m_name.Empty();

        delete m_conv;
        m_conv=NULL;
    }

    if (!name.IsEmpty() && !(m_conv=new wxCSConv(name))->IsOk())
    {
        delete m_conv;
        m_conv=NULL;
    }

    m_name=name;

    return m_conv!=NULL;
}

wxUint32 CslCharEncoding::GetEncodingId() const
{
    wxUint32 i;

    for (i=0;i<CSL_NUM_CHAR_ENCODINGS;i++)
    {
        if (CslCharEncodings[i].Encoding==m_name)
            return i;
    }

    return 0;
}

wxString CslCharEncoding::ToLocal(const char *data)
{
    if (!data || !*data)
        return wxEmptyString;

    wxString s;
    wxChar *buffer;

    if (m_utf8)
    {
        if ((buffer=ConvToLocalBuffer(data,wxConvUTF8)))
        {
            s=buffer;
            delete[] buffer;
            return s;
        }
#if CSL_CHAR_ENCODING_DEBUG
        fprintf(stderr,"Invalid UTF-8 sequence: %s\n",data);
#endif
    }

    if (m_conv)
    {
        LOG_DEBUG("trying '%s'\n",U2A(m_name));
        if ((buffer=ConvToLocalBuffer(data,*m_conv)))
        {
            s=buffer;
            delete[] buffer;
            return s;
        }
#if CSL_CHAR_ENCODING_DEBUG
        fprintf(stderr,"Invalid %s sequence: %s\n",U2A(m_name),data);
#endif
    }

    wxCSConv conv(wxT("ISO-8859-15"));

    if ((buffer=ConvToLocalBuffer(data,conv)))
    {
        s=buffer;
        delete[] buffer;
        return s;
    }
#if CSL_CHAR_ENCODING_DEBUG
    fprintf(stderr,"Invalid ISO-8859-15 sequence: %s\n",data);
#endif

    return wxConvCurrent->cMB2WX(data);
}

wxCharBuffer CslCharEncoding::ToServer(const wxString& str)
{
    wxCharBuffer buffer;

    if (m_conv)
    {
#if wxUSE_UNICODE
        buffer=m_conv->cWX2MB(str);
#else
        wxWCharBuffer ubuffer;

        if ((ubuffer=wxConvCurrent->cMB2WC(str)))
            buffer=m_conv->cWC2MB(ubuffer);
#endif
        if (buffer)
            return buffer;
#if CSL_CHAR_ENCODING_DEBUG
        fprintf(stderr,"Conversion failed.\n");
#endif
    }

    if (m_utf8)
    {
#if wxUSE_UNICODE
        buffer=wxConvUTF8.cWX2MB(str);
#else
        wxWCharBuffer ubuffer;

        if ((ubuffer=wxConvCurrent->cMB2WC(str)))
            buffer=wxConvUTF8.cWC2MB(ubuffer);
#endif
        if (buffer)
            return buffer;
#if CSL_CHAR_ENCODING_DEBUG
        fprintf(stderr,"Conversion to UTF-8 failed.\n");
#endif
    }

    if (!(buffer=wxConvCurrent->cWX2MB(str)))
        buffer=wxCSConv(_T("ISO8859-15")).cWX2MB(str);

    return buffer;
}

wxChar* CslCharEncoding::ConvToLocalBuffer(const char *data,wxMBConv& conv)
{
    size_t len;

    if (!(len=conv.MB2WC(NULL,data,0)) || len==wxCONV_FAILED)
        return NULL;

    wchar_t *ubuffer=new wchar_t[len+1];
    conv.MB2WC(ubuffer,data,len+1);
#if wxUSE_UNICODE
    return ubuffer;
#else
    if (!(len=wxConvCurrent->WC2MB(NULL,ubuffer,0)) || len==wxCONV_FAILED)
    {
        delete[] ubuffer;
        return NULL;
    }

    wxChar *buffer=new wxChar[len+1];
    wxConvCurrent->WC2MB(buffer,ubuffer,len+1);
    delete[] ubuffer;

    return buffer;
#endif
}
