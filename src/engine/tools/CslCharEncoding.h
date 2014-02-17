/***************************************************************************
 *   Copyright (C) 2007-2014 by Glen Masgai                                *
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

#if wxCHECK_VERSION(2, 9, 0)
#define T2C(x) x.c_str()
#else
#define T2C(x) x
#endif

// this is the wx(W)Charbuffer class (slightly modified) from
// wxWidgets 2.8 (buffer.h). the new template based ones within
// wxWidgets 2.9+ ares using reference counters, which causes
// trouble with static CRT builds, so don't use any wx-function
// which returns wx(W)Charbuffer !

#define CSL_DEFINE_STRING_BUFFER(classname, chartype, strdupfunc)           \
class CSL_DLL_TOOLS classname                                               \
{                                                                           \
public:                                                                     \
    classname(const chartype *str = NULL)                                   \
        : m_str(str ? strdupfunc(str) : NULL)                               \
    {                                                                       \
    }                                                                       \
                                                                            \
    classname(size_t len, const chartype *str = NULL)                       \
        : m_str((chartype *)malloc((len + 1)*sizeof(chartype)))             \
    {                                                                       \
        m_str[len] = (chartype)0;                                           \
                                                                            \
        if (str)                                                            \
            memcpy(m_str, str, len);                                        \
    }                                                                       \
                                                                            \
    ~classname() { free(m_str); }                                           \
                                                                            \
    chartype *release() const                                               \
    {                                                                       \
        chartype *p = m_str;                                                \
        ((classname *)this)->m_str = NULL;                                  \
        return p;                                                           \
    }                                                                       \
                                                                            \
    void reset()                                                            \
    {                                                                       \
        free(m_str);                                                        \
        m_str = NULL;                                                       \
    }                                                                       \
                                                                            \
    classname(const classname& src)                                         \
        : m_str(src.release())                                              \
    {                                                                       \
    }                                                                       \
                                                                            \
    classname& operator=(const chartype *str)                               \
    {                                                                       \
        free(m_str);                                                        \
        m_str = str ? strdupfunc(str) : NULL;                               \
        return *this;                                                       \
    }                                                                       \
                                                                            \
    classname& operator=(const classname& src)                              \
    {                                                                       \
        free(m_str);                                                        \
        m_str = src.release();                                              \
                                                                            \
        return *this;                                                       \
    }                                                                       \
                                                                            \
    bool extend(size_t len)                                                 \
    {                                                                       \
        chartype *                                                          \
            str = (chartype *)realloc(m_str, (len + 1)*sizeof(chartype));   \
        if ( !str )                                                         \
            return false;                                                   \
                                                                            \
        m_str = str;                                                        \
                                                                            \
        return true;                                                        \
    }                                                                       \
                                                                            \
    chartype *data() { return m_str; }                                      \
    const chartype *data() const { return m_str; }                          \
    operator const chartype *() const { return m_str; }                     \
    chartype operator[](size_t n) const { return m_str[n]; }                \
                                                                            \
private:                                                                    \
    chartype *m_str;                                                        \
}

CSL_DEFINE_STRING_BUFFER(CslCharBuffer, char, wxStrdupA);
CSL_DEFINE_STRING_BUFFER(CslWCharBuffer, wchar_t, wxStrdupW);

struct CslCharEncodingTableEntry
{
    const wxChar *Encoding;
    const wxString Name;
};

#define CSL_NUM_CHAR_ENCODINGS  13
CSL_DLL_TOOLS extern const CslCharEncodingTableEntry CslCharEncodingTable[CSL_NUM_CHAR_ENCODINGS];

class CSL_DLL_TOOLS CslCharEncoding
{
    public:
        CslCharEncoding(const wxString& name = wxEmptyString);
        ~CslCharEncoding();

        bool SetEncoding(const wxString& name);
        const wxString& GetEncoding() const { return m_name; }
        wxInt32 GetEncodingId() const;

        wxString MB2WX(const char *data) const;
        const CslCharBuffer WX2MB(const wxString& str) const;

        static char* FromWChar(const wxMBConv *conv, const wchar_t *pwz);
        static wchar_t* ToWChar(const wxMBConv *conv, const char *psz);

        static wxString CubeMB2WX(const char *data);

    private:
        wxString m_name;
        wxCSConv *m_conv;

    protected:
};

CSL_DLL_TOOLS extern const CslCharEncoding CslCurrentEncoding;
CSL_DLL_TOOLS extern const CslCharEncoding CslUTF8Encoding;
CSL_DLL_TOOLS extern const CslCharEncoding CslISO_8859_15Encoding;

static inline wxString CslMB2WX(const char *s, const CslCharEncoding *conv = NULL)
{
    const CslCharEncoding *c = conv ? conv : &CslCurrentEncoding;
    return c->MB2WX(s);
}

template<class T>
inline CslCharBuffer CslWX2MB(const T& s, const CslCharEncoding *conv = NULL)
{
    const CslCharEncoding *c = conv ? conv : &CslUTF8Encoding;
    return c->WX2MB(s);
}

#define C2U(PSZA_CHART)              CslMB2WX(PSZA_CHART)
#define U2C(PSZW_CHART) (const char*)CslWX2MB(PSZW_CHART)

#endif //CSLCHARENCODING_H
