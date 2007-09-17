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

#ifdef __WXDEBUG__
#define CSL_EXT_SERVER_INFO
#endif

/**
    @author Glen Masgai <mimosius@gmx.de>
*/

#define A2U(PSZA_CHART) wxString(wxConvertMB2WX(PSZA_CHART))
#define U2A(PSZT_CHART) (char*)(const char*)wxConvertWX2MB(PSZT_CHART)

#define COLOUR2INT(col) ((col.Red()<<16)|(col.Green()<<8)|col.Blue())
#define INT2COLOUR(int) wxColour((int>>16)&0xFF,(int>>8)&0xFF,int&0xFF)

#define SYSCOLOUR(x) wxSystemSettings::GetColour(x)


#ifdef __WXDEBUG__
void Debug_Printf(const char *DbgFunc, const char *FmtStr,...);
#define LOG_DEBUG(...) Debug_Printf(__FUNCTION__,## __VA_ARGS__);
#else
#define LOG_DEBUG(...)
#endif


extern char* StripColours(const char *s);
extern bool IsIP(const wxString& s);
extern bool IP2Int(const wxString& s,wxUint32 *ip);
extern wxString FormatSeconds(wxUint32 time);


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

extern wxUint32 GetTicks();


#endif // CSLTOOLS_H
