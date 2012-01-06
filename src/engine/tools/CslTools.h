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

#ifndef CSLTOOLS_H
#define CSLTOOLS_H

/**
    @author Glen Masgai <mimosius@users.sourceforge.net>
*/

#ifdef _MSC_VER
#ifdef UNICODE
#define _L_ L
#else
#define _L_
#endif //__UNICODE__
#define strcasecmp _stricmp
#else
#define _L_
#endif //_MSC_VER

#define COLOUR2INT(col) ((col.Red()<<16)|(col.Green()<<8)|col.Blue())
#define INT2COLOUR(int) wxColour((int>>16)&0xFF, (int>>8)&0xFF, int&0xFF)

#define SYSCOLOUR(x) wxSystemSettings::GetColour(x)
#define SYSMETRIC(x, w) wxSystemSettings::GetMetric(x, w)

#ifdef __WXMSW__
#define CSL_NEWLINE wxT("\r\n")
#define PATHDIV wxT("\\")
#else
#define CSL_NEWLINE wxT("\n")
#define PATHDIV wxT("/")
#endif

#define CSL_USER_DIR wxString(wxStandardPaths().GetUserDataDir()+PATHDIV)

#ifdef __WXMSW__
#define CSL_EXE_EXTENSIONS wxString(_("Executables (*.exe; *.bat; *.cmd)|*.exe;*.bat;*.cmd"))
#else
#define CSL_EXE_EXTENSIONS wxString(wxT("*"))
#endif

#ifdef __DEBUG__
void Debug_Printf(const char *file, int line, const char *func, const char *fmt, ...);
#define LOG_DEBUG(...) Debug_Printf(__FILE__, __LINE__, __FUNCTION__, ## __VA_ARGS__);
#else
#define LOG_DEBUG(...)
#endif

#define CSL_FLAG_SET(var, val)    var|=(val)
#define CSL_FLAG_UNSET(var, val)  var&=~(val)
#define CSL_FLAG_CHECK(var, val)  ((var&(val))!=0)

wxInt32 BitCount32(wxUint32 value);

#include <CslCharEncoding.h>
#include <CslIPV4Addr.h>
#include <CslCubeEngineTools.h>

typedef const char FourCCTag[4];
static inline wxUint32 CSL_BUILD_FOURCC(FourCCTag tag)
{
    return tag[0]|(tag[1]<<8)|(tag[2]<<16)|(tag[3]<<24);
};

static inline wxUint32 CSL_BUILD_VERSION(wxUint8 major, wxUint8 minor,
                                         wxUint8 release, wxUint8 build=0)
{
    return build|(release<<8)|(minor<<16)|(major<<24);
};
static inline wxString CSL_GET_VERSION(wxUint32 version)
{
    wxString v;
    v.Printf(wxT("%u.%u.%u"), version>>24&0xff, version>>16&0xff, version>>8&0xff);
    if (version&0xff) v<<wxT(".")<<(version&0xff);
    return v;
};

WX_DEFINE_ARRAY_PTR(void*, VoidPointerArray);

class StopWatch : public wxStopWatch
{
    public:
        StopWatch(bool dtor=true) : wxStopWatch(), m_onDtor(dtor) { }
        ~StopWatch() { if (m_onDtor) Dump(); }

        wxUint32 Elapsed() { return Time(); }
        void Dump() { LOG_DEBUG("%lu ms\n", Time()); }
    private:
        bool m_onDtor;
};

enum
{
    CSL_ERROR_NONE            =  0,
    CSL_ERROR_GAME_UNKNOWN    = -1,
    CSL_ERROR_FILE_OPERATION  = -20,
    CSL_ERROR_FILE_DONT_EXIST = -21,
};

wxString& CmdlineEscapeQuotes(wxString& str);
wxString& CmdlineEscapeSpaces(wxString& str);
char* FixString(char *src, wxInt32 coloursize, bool space=true, bool newline=false, bool tab=false);
wxString& FixFilename(wxString& name);
bool IsIP(const wxString& s);
bool IsLocalIP(const CslIPV4Addr& addr, vector<CslIPV4Addr*> *ifaces=NULL);
template<class T> bool IsLocalIP(const T& t, vector<CslIPV4Addr*> *ifaces=NULL) { return IsLocalIP(CslIPV4Addr(t), ifaces); }
void GetSystemIPAddresses(vector<CslIPV4Addr*>& ifaces);
wxUint32 AtoN(const wxString& s);
wxString NtoA(wxUint32 ip);
wxString FormatBytes(wxUint64 size);
wxString FormatSeconds(wxUint32 time, bool space=false, bool full=false);
wxUint32 GetTicks();
const wxString& GetHttpAgent();
wxString FileName(const wxString& path, bool native=false);
wxString DirName(const wxString& path, bool native=false);
void AddPluginDir(const wxString& path, bool native=false);
void RemovePluginDir(const wxString& path);
wxArrayString GetPluginDirs();
void AddDataDir(const wxString& path, bool native=false);
void RemoveDataDir(const wxString& path);
wxArrayString GetDataDirs();
wxString FindPackageFile(const wxString& filename);
wxInt32 FindFiles(const wxString& path, const wxString& filespec, wxArrayString& results);
bool HasDir(wxDir& dir, const wxString& path);
wxInt32 WriteTextFile(const wxString& filename, const wxString& data, const wxFile::OpenMode mode);

#endif //CSLTOOLS_H
