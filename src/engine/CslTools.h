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

#ifndef CSLTOOLS_H
#define CSLTOOLS_H

/**
    @author Glen Masgai <mimosius@users.sourceforge.net>
*/

#include <wx/file.h>
#include <wx/stdpaths.h>
#include <wx/stopwatch.h>
#include <wx/bitmap.h>
#include <wx/window.h>
#include "CslCharEncoding.h"
#include "cube_tools.h"

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

#ifndef DATADIR
#undef LOCALEDIR
#ifdef __WXMAC__
#define DATAPATH g_basePath+wxString(wxT("/../Resources"))
#else
#define DATAPATH g_basePath+wxString(PATHDIV)+wxString(wxT("data"))
#endif //__WXMAC__
#ifdef __WXMSW__
#define LOCALEPATH g_basePath+wxString(PATHDIV)+wxString(wxT("lang"))
#else
#define LOCALEPATH DATAPATH+wxString(PATHDIV)+wxString(wxT("lang"))
#endif //__WXMSW__
#else
#define DATAPATH wxString(wxT(DATADIR))+wxString(PATHDIV)
#define LOCALEPATH wxString(wxT(LOCALEDIR))+wxString(PATHDIV)
#endif //DATADIR

#define COLOUR2INT(col) ((col.Red()<<16)|(col.Green()<<8)|col.Blue())
#define INT2COLOUR(int) wxColour((int>>16)&0xFF,(int>>8)&0xFF,int&0xFF)

#define SYSCOLOUR(x) wxSystemSettings::GetColour(x)

#ifdef __WXMSW__
#define PATHDIV wxT("\\")
#define PATHDIVA wxT('\\')
#else
#define PATHDIV wxT("/")
#define PATHDIVA wxT('/')
#endif

#define CSL_USER_DATADIR wxString(wxStandardPaths().GetUserDataDir()+PATHDIV)

#ifdef __WXDEBUG__
void Debug_Printf(const char *file,int line,const char *func,const char *fmt,...);
#define LOG_DEBUG(...) Debug_Printf(__FILE__,__LINE__,__FUNCTION__,## __VA_ARGS__);
#else
#define LOG_DEBUG(...)
#endif

WX_DEFINE_ARRAY_PTR(void*,VoidPointerArray);

class stopwatch : wxStopWatch
{
    public:
        stopwatch() : wxStopWatch() {}
        ~stopwatch() { dump(); }
        void dump() { fprintf(stderr,"stopwatch: %li ms\n",Time()); }
};

enum
{
    CSL_ERROR_NONE            =  0,
    CSL_ERROR_GAME_UNKNOWN    = -1,
    CSL_ERROR_FILE_OPERATION  = -20,
    CSL_ERROR_FILE_DONT_EXIST = -21,
};

extern wxString g_basePath;

extern void FixString(char *src,wxUint32 *len,wxUint32 count,bool keepnl=false);
extern void FixFilename(wxString& name);
extern bool IsIP(const wxString& s);
extern bool IsLocalIP(const wxString& s);
extern wxUint32 IP2Int(const wxString& s);
inline wxString Int2IP(wxUint32 ip)
{
    return wxString::Format(wxT("%d.%d.%d.%d"),ip>>24,ip>>16&0xff,ip>>8&0xff,ip&0xff);
}
extern wxString FormatBytes(wxUint64 size);
extern wxString FormatSeconds(wxUint32 time,bool space=false,bool full=false);
extern wxUint32 GetTicks();
extern bool BitmapFromWindow(wxWindow *window,wxBitmap& bitmap);
extern void RegisterEventsRecursively(wxInt32 id,wxWindow *parent,wxEvtHandler *handler,
                                          wxEventType type,wxObjectEventFunction function);
extern wxString GetHttpAgent();
extern wxInt32 WriteTextFile(const wxString& filename,const wxString& data,const wxFile::OpenMode mode);


wxBitmap AdjustIconSize(const char **data,const wxIcon& icon,
                        const wxSize& size,const wxPoint& origin);

#endif // CSLTOOLS_H
