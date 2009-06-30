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
#define SYSMETRIC(x,w) wxSystemSettings::GetMetric(x,w)

#ifdef __WXMSW__
#define NEWLINE wxT("\r\n")
#define PATHDIV wxT("\\")
#define PATHDIVA wxT('\\')
#else
#define NEWLINE wxT("\n")
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

wxString& CmdlineEscapeQuotes(wxString& str);
wxString& CmdlineEscapeSpaces(wxString& str);
void FixString(char *src,wxUint32 *len,wxUint32 count,bool keepnl=false,bool keeptab=false);
void FixFilename(wxString& name);
bool IsIP(const wxString& s);
bool IsLocalIP(const wxString& s);
wxUint32 IP2Int(const wxString& s);
wxString Int2IP(wxUint32 ip);
wxString FormatBytes(wxUint64 size);
wxString FormatSeconds(wxUint32 time,bool space=false,bool full=false);
wxUint32 GetTicks();
wxString GetHttpAgent();
wxInt32 WriteTextFile(const wxString& filename,const wxString& data,const wxFile::OpenMode mode);

#if wxUSE_GUI
wxBitmap AdjustBitmapSize(const char **data,const wxSize& size,const wxPoint& origin);
wxBitmap AdjustBitmapSize(const wxBitmap& bitmap,const wxSize& size,const wxPoint& origin);
wxBitmap BitmapFromData(wxInt32 type,const unsigned char *data,wxInt32 size);
bool BitmapFromWindow(wxWindow *window,wxBitmap& bitmap);
wxImage& OverlayImage(wxImage& dst,const wxImage& src,wxInt32 offx,wxInt32 offy);

wxWindow* GetParentWindow(wxWindow *window,wxInt32 depth);
void RegisterEventsRecursively(wxInt32 id,wxWindow *parent,wxEvtHandler *handler,
                               wxEventType type,wxObjectEventFunction function);

wxSize GetBestWindowSizeForText(wxWindow *window,const wxString& text,
                                wxInt32 minWidth,wxInt32 maxWidth,
                                wxInt32 minHeight,wxInt32 maxHeight,
                                wxInt32 scrollbar=0);
#endif //wxUSE_GUI

#endif //CSLTOOLS_H
