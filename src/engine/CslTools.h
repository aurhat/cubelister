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

#ifndef CSLTOOLS_H
#define CSLTOOLS_H

/**
    @author Glen Masgai <mimosius@gmx.de>
*/

#include <wx/file.h>
#include <wx/stdpaths.h>
#include <wx/stopwatch.h>
#include <wx/bitmap.h>

// since WX > 2.8.4 the listctrl items
// get deselected when sorting (only wxGTK ?)
#if defined(__WXGTK__) || defined(__WXMAC__)
#ifndef CSL_USE_WX_LIST_DESELECT_WORKAROUND
#if wxVERSION_NUMBER > 2804 || defined(__WXMAC__)
#define CSL_USE_WX_LIST_DESELECT_WORKAROUND
#endif
#endif
#endif

#ifdef _MSC_VER
#define strcasecmp _stricmp
#else
#endif

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

#define A2U(PSZA_CHART) wxString(wxConvertMB2WX(PSZA_CHART))
#define U2A(PSZT_CHART) (char*)(const char*)wxConvertWX2MB(PSZT_CHART)

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

#ifdef __WXDEBUG__
void Debug_Printf(const char *DbgFunc, const char *FmtStr,...);
#define LOG_DEBUG(...) Debug_Printf(__FUNCTION__,## __VA_ARGS__);
#else
#define LOG_DEBUG(...)
#endif

class stopwatch : wxStopWatch
{
    public:
        stopwatch() : wxStopWatch() {}
        ~stopwatch()
        {
            printf("%li ms\n",Time());
        }
};

enum
{
    CSL_ERROR_NONE            =  0,
    CSL_ERROR_GAME_UNKNOWN    = -1,
    CSL_ERROR_FILE_OPERATION  = -20,
    CSL_ERROR_FILE_DONT_EXIST = -21,
};

extern wxString g_basePath;

extern void StripColours(char *src,wxInt32 *len,wxInt32 count);
extern bool IsIP(const wxString& s);
extern bool IP2Int(const wxString& s,wxUint32 *ip);
extern wxString FormatBytes(wxUint64 size);
extern wxString FormatSeconds(wxUint32 time,bool space=false,bool full=false);
extern wxUint32 GetTicks();
extern wxString GetHttpAgent();
extern wxInt32 WriteTextFile(const wxString& filename,const wxString& data,const wxFile::OpenMode mode);

#ifdef __WXMSW__
wxBitmap AdjustIconSize(const char **data,const wxIcon& icon,
                        const wxSize& size,const wxPoint& origin);
#endif

enum { CSL_SORT_ASC = 0, CSL_SORT_DSC };
enum { CSL_LIST_SORT_INT = 0, CSL_LIST_SORT_UINT, CSL_LIST_SORT_STRING };

class CslListSortHelper
{
    public:
        void Init(wxUint32 mode,wxUint32 type)
        {
            m_sortMode=mode;
            m_sortType=type;
        }

        wxInt32 m_sortMode;
        wxInt32 m_sortType;
};

#endif // CSLTOOLS_H
