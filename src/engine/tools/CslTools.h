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

#ifndef CSLTOOLS_H
#define CSLTOOLS_H

#ifdef _MSC_VER
#ifdef _DEBUG
#define CSL_DEBUG
#endif
#define strcasecmp _stricmp
#endif //_MSC_VER

#include <wx/uri.h>
#include <CslDynlib.h>

#ifdef __WXMSW__
#define CSL_PATHDIV_MB  "\\"
#define CSL_NEWLINE_MB  "\r\n"
#else
#define CSL_PATHDIV_MB  "/"
#define CSL_NEWLINE_MB  "\n"
#endif
#define CSL_PATHDIV_WX wxT(CSL_PATHDIV_MB)
#define CSL_NEWLINE_WX wxT(CSL_NEWLINE_MB)

#ifdef __WXMSW__
#define chmod _chmod
#define CSL_EXE_EXTENSIONS wxString(_("Executables (*.exe; *.bat; *.cmd)|*.exe;*.bat;*.cmd"))
#else
#define CSL_EXE_EXTENSIONS wxString(wxT("*"))
#endif

#ifdef __GNUC__
#define CSL_PRETTY_FN __PRETTY_FUNCTION__
#define CSL_PRINTF_CHECK(fmt, args) __attribute__((format(printf, fmt, args)))
#else
#define CSL_PRETTY_FN __FUNCTION__
#define CSL_PRINTF_CHECK(fmt, args)
#endif

CSL_DLL_TOOLS void Debug_Printf(const char *file, int line, const char *func,
                                const char *fmt, ...) CSL_PRINTF_CHECK(4, 5);

#ifdef CSL_DEBUG
#define CSL_DEF_DEBUG(...)  __VA_ARGS__
#define CSL_DO_DEBUG(body)  do { body; } while (0)
#define CSL_LOG_DEBUG(...)  Debug_Printf(__FILE__, __LINE__, CSL_PRETTY_FN, ## __VA_ARGS__)
#else
#define CSL_DEF_DEBUG(...)
#define CSL_DO_DEBUG(body)  do { } while (0)
#define CSL_LOG_DEBUG(...)  do { } while (0)
#endif

#define CSL_FLAG_SET(var, val)    var|=(val)
#define CSL_FLAG_UNSET(var, val)  var&=~(val)
#define CSL_FLAG_CHECK(var, val)  ((var&(val))!=0)

#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif
template<class T> inline T max(T a, T b) { return a > b ? a : b; }
template<class T> inline T min(T a, T b) { return a < b ? a : b; }
#define clamp(a, b, c) (max(b, min(a, c)))

#define loop(i, m)      for (wxInt32 i = 0; wxInt32(i)<wxInt32(m); i++)
#define loopi(m)        loop(i, m)
#define loopj(m)        loop(j, m)
#define loopk(m)        loop(k, m)
#define loopl(m)        loop(l, m)

#define _loopv(i, v)    for (int i=0; i<(int)(v).GetCount(); ++i)
#define _loopvrev(i, v) for (int i=(v).GetCount()-1; i>=0; --i)
#define loopv(v)        _loopv(i, v)
#define loopvj(v)       _loopv(j, v)
#define loopvk(v)       _loopv(k, v)
#define loopvrev(v)     _loopvrev(i, v)

#ifndef wxT_2 // older wx versions
#define wxT_2(x) wxT(x)
#endif

#include <CslCharEncoding.h>
#include <CslCubeEngineTools.h>

typedef const char FourCCTag[4];
static inline wxUint32 CSL_BUILD_FOURCC(FourCCTag tag)
{
    return tag[0]|(tag[1]<<8)|(tag[2]<<16)|(tag[3]<<24);
};

// same as wxBITMAP_TYPE_xxx but available if wxUSE_GUI=0
// wrapper Csl2wxBitmapType(type) is in CslGuiTools.h
enum
{
    CSL_BITMAP_TYPE_BMP,
    CSL_BITMAP_TYPE_GIF,
    CSL_BITMAP_TYPE_JPEG,
    CSL_BITMAP_TYPE_PNG,
    CSL_BITMAP_TYPE_PCX,
    CSL_BITMAP_TYPE_PNM,
    CSL_BITMAP_TYPE_TIF,
    CSL_BITMAP_TYPE_XPM,
    CSL_BITMAP_TYPE_ICO,
    CSL_BITMAP_TYPE_CUR,
    CSL_BITMAP_TYPE_ANI,
    CSL_BITMAP_TYPE_ANY
};

enum
{
    CSL_ERROR_NONE            =  0,
    CSL_ERROR_GAME_UNKNOWN    = -1,
    CSL_ERROR_FILE_OPERATION  = -20,
    CSL_ERROR_FILE_DONT_EXIST = -21,
};

BEGIN_DECLARE_EVENT_TYPES()
        DECLARE_EXPORTED_EVENT_TYPE(CSL_DLL_TOOLS, wxCSL_EVT_THREAD_TERMINATE, wxID_ANY)
        END_DECLARE_EVENT_TYPES()

#define CSL_EVT_THREAD_TERMINATE(fn)                                             \
    DECLARE_EVENT_TABLE_ENTRY(                                                   \
                               wxCSL_EVT_THREAD_TERMINATE, wxID_ANY, wxID_ANY,   \
                               (wxObjectEventFunction)(wxEventFunction)          \
                               wxStaticCastEvent(wxCommandEventFunction, &fn),   \
                               (wxObject*)NULL                                   \
                             ),


class CslStopWatch : public wxStopWatch
{
    public:
        CslStopWatch(bool dtor = true) :
            m_onDtor(dtor)
            { }
        ~CslStopWatch()
            { if (m_onDtor) Dump(); }

        void Dump()
            { Debug_Printf(__FILE__, __LINE__, CSL_PRETTY_FN, "%ld ms\n", Time()); }

    private:
        bool m_onDtor;
};


template<class T>
class CslValueRestore
{
    public:
        CslValueRestore(T& type, const T& set) :
                m_type(&type), m_restore(type)
            { *m_type = set;  }
        ~CslValueRestore()
            { *m_type = m_restore; }

    private:
        CslValueRestore(const CslValueRestore&);
        CslValueRestore& operator=(const CslValueRestore&);

    protected:
        T* m_type;
        T m_restore;
};


class CSL_DLL_TOOLS CslFileTime : public wxObject
{
    public:
        CslFileTime(const wxDateTime& modify = wxDefaultDateTime,
                    const wxDateTime& access = wxDefaultDateTime,
                    const wxDateTime& create = wxDefaultDateTime) :
                Modify(modify),
                Access(access),
                Create(create)
            { }

        void Check()
        {
            if (!Modify.IsValid())
                Modify = wxDateTime::Now().MakeUTC();
            if (!Access.IsValid())
                Access = Modify;
            if (!Create.IsValid())
                Create = Modify;
        }

        bool Get(const wxFileName& fileName)
        {
            if (fileName.GetTimes(&Access, &Modify, &Create))
            {
                MakeUTC();
                return true;
            }

            return false;
        }

        bool Set(wxFileName& fileName)
        {
            Check();

            wxDateTime modify = Modify.FromTimezone(wxDateTime::UTC);
            wxDateTime access = Access.FromTimezone(wxDateTime::UTC);
            wxDateTime create = Create.FromTimezone(wxDateTime::UTC);

            if (!fileName.SetTimes(&access, &modify, &create))
                return false;

            return true;
        }

        void MakeUTC()
        {
            Check();

            Modify.MakeUTC();
            Access.MakeUTC();
            Create.MakeUTC();
        }

        wxDateTime Modify;
        wxDateTime Access;
        wxDateTime Create;

    private:
        DECLARE_DYNAMIC_CLASS(CslFileTime);
};


// same as wxTAR_xxx
enum
{
    CSL_FILE_REGTYPE,   // regular file
    CSL_FILE_LNKTYPE,   // hard link
    CSL_FILE_SYMTYPE,   // symbolic link
    CSL_FILE_CHRTYPE,   // character special
    CSL_FILE_BLKTYPE,   // block special
    CSL_FILE_DIRTYPE,   // directory
    CSL_FILE_FIFOTYPE,  // named pipe
    CSL_FILE_CONTTYPE   // contiguous file
};

class CSL_DLL_TOOLS CslFileProperties : public wxObject
{
    public:
        CslFileProperties(const wxString& name = wxEmptyString,
                          wxInt32 type = CSL_FILE_REGTYPE,
                          wxInt32 mode = 0,
                          size_t size = 0,
                          const wxDateTime& modify = wxDefaultDateTime,
                          const wxDateTime& access = wxDefaultDateTime,
                          const wxDateTime& create = wxDefaultDateTime) :
                Name(name),
                Type(type),
                Mode(mode ? mode : (type==CSL_FILE_DIRTYPE ? 0700 : 0600)),
                Size(size),
                Time(modify, access, create)
            { }


        bool Get()
        {
            const wxString filename = Name.GetFullPath();

            if (Type==CSL_FILE_DIRTYPE && !wxFileName::DirExists(filename))
                return false;
            else if (!wxFileName::FileExists(filename))
                return false;

            if (!Time.Get(Name))
                return false;

            if ((Size = Name.GetSize().GetValue())==wxInvalidSize)
                return false;
        }

        bool Set()
        {
            const wxString filename = Name.GetFullPath();

            if (Type==CSL_FILE_DIRTYPE && !wxFileName::DirExists(filename))
                return false;
            else if (!wxFileName::FileExists(filename))
                return false;

            if (chmod(U2C(filename), Mode)<0)
                return false;

            if (!Time.Set(Name))
                return false;

            return true;
        }

        wxFileName Name;
        wxInt32 Type;
        wxInt32 Mode;
        size_t Size;
        CslFileTime Time;

    private:
        DECLARE_DYNAMIC_CLASS(CslFileProperties)
};


CSL_DLL_TOOLS wxUint32 BitCount32(wxUint32 value);

#include <CslIPV4Addr.h>

CSL_DLL_TOOLS wxString& CmdlineEscapeQuotes(wxString& str);
CSL_DLL_TOOLS wxString& CmdlineEscapeSpaces(wxString& str);
CSL_DLL_TOOLS char* FilterCubeString(char *src, wxInt32 coloursize, bool space = true, bool newline = false, bool tab = false);
CSL_DLL_TOOLS wxString& FixFilename(wxString& name);
inline bool IsIPV4(const CslIPV4Addr& addr, wxUint32 maskbits = 32) { return addr.GetMaskBits()==maskbits; }
template<class T>
bool IsIPV4(const T& t, wxUint32 maskbits = 32) { return IsIPV4(CslIPV4Addr(t), maskbits); }
CSL_DLL_TOOLS bool IsLocalIPV4(const CslIPV4Addr& addr, CslArrayCslIPV4Addr *addresses=NULL);
template<class T>
bool IsLocalIPV4(const T& t, CslArrayCslIPV4Addr *addresses=NULL) { return IsLocalIPV4(CslIPV4Addr(t), addresses); }
CSL_DLL_TOOLS void GetSystemIPV4Addresses(CslArrayCslIPV4Addr& addresses);
CSL_DLL_TOOLS void FreeSystemIPV4Addresses(CslArrayCslIPV4Addr& addresses);
CSL_DLL_TOOLS wxUint32 AtoN(const wxString& s);
CSL_DLL_TOOLS wxString NtoA(wxUint32 ip);
CSL_DLL_TOOLS wxString FormatBytes(wxUint64 size);
CSL_DLL_TOOLS wxString FormatSeconds(wxUint32 time, bool space=false, bool full=false);
CSL_DLL_TOOLS wxString ToRfc2822(const wxDateTime& date, bool isUTC = true);
CSL_DLL_TOOLS wxDateTime FromRfc2822(const wxString& date, bool toUTC = true);
CSL_DLL_TOOLS time_t GetTicks();
CSL_DLL_TOOLS const wxString& GetHttpAgent();
CSL_DLL_TOOLS wxString GetTempDir();
CSL_DLL_TOOLS wxString FileName(const wxString& path, const wxString& cwd = wxEmptyString);
CSL_DLL_TOOLS wxString DirName(const wxString& path, const wxString& cwd = wxEmptyString);
CSL_DLL_TOOLS wxString FindFile(const wxString& name, wxArrayString& paths, bool first = false);
CSL_DLL_TOOLS wxString AddPluginDir(const wxString& path, const wxString& cwd = wxEmptyString);
CSL_DLL_TOOLS wxString RemovePluginDir(const wxString& path);
CSL_DLL_TOOLS wxArrayString GetPluginDirs();
CSL_DLL_TOOLS wxString AddDataDir(const wxString& path, const wxString& cwd = wxEmptyString);
CSL_DLL_TOOLS wxString RemoveDataDir(const wxString& path);
CSL_DLL_TOOLS wxArrayString GetDataDirs();
CSL_DLL_TOOLS wxString FindPackageFile(const wxString& name, bool home = false);
CSL_DLL_TOOLS wxInt32 FindFiles(const wxString& path, const wxString& filespec, wxArrayString& results);
CSL_DLL_TOOLS bool HasDir(wxDir& dir, const wxString& path);
CSL_DLL_TOOLS wxInt32 WriteTextFile(const wxString& filename, const wxString& data, const wxFile::OpenMode mode);

#endif //CSLTOOLS_H
