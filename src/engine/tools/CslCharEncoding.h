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

#ifndef CSLCHARENCODING_H
#define CSLCHARENCODING_H

/**
 @author Glen Masgai <mimosius@users.sourceforge.net>
*/

class CslCharEncoding;

extern const CslCharEncoding CslDefaultCharEncoding;
//#define A2U(PSZA_CHART) wxString(wxConvertMB2WX(PSZA_CHART))
//#define U2A(PSZT_CHART) (char*)(const char*)wxConvertWX2MB(PSZT_CHART)
#define A2U(PSZA_CHART) CslDefaultCharEncoding.ToLocal(PSZA_CHART)
#define U2A(PSZT_CHART) (const char*)CslDefaultCharEncoding.ToServer(PSZT_CHART)

#if wxCHECK_VERSION(2, 9, 0)
#define T2C(x) x.c_str()
#else
#define T2C(x) x
#endif

static const struct { const wxChar *Encoding, *Name; } CslCharEncodingTable[] =
{
    { wxT("UTF-8"),       _("Default")          },
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
    { wxT("cp 1252"),     _("Western European") },
};
static const wxInt32 CSL_NUM_CHAR_ENCODINGS=sizeof(CslCharEncodingTable)/sizeof(CslCharEncodingTable[0]);

class CslCharEncoding
{
    public:
        CslCharEncoding(bool utf8=true, const wxString& name=wxEmptyString);
        ~CslCharEncoding();

        bool SetEncoding(const wxString& name);
        const wxString& GetEncoding() const { return m_name; }
        wxInt32 GetEncodingId() const;

        wxString ToLocal(const char *data) const;
        wxCharBuffer ToServer(const wxString& str) const;

    private:
        bool m_utf8;
        wxString m_name;
        wxCSConv *m_conv;

    protected:
        wxChar* ConvToLocalBuffer(const char *data, const wxMBConv& conv) const;
};

wxString CslCubeEncodingToLocal(const char *data);

#endif //CSLCHARENCODING_H
