
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

#ifndef CSLMENU_H
#define CSLMENU_H

/**
 @author Glen Masgai <mimosius@flexo>
*/

#include "wx/wxprec.h"
#ifdef __BORLANDC__
#pragma hdrstop
#endif
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "CslSettings.h"

#define MENU_SERVER_CONN_STR      _("&Connect to server")
#define MENU_SERVER_COPY_STR      _("C&opy to clipboard")
#define MENU_SERVER_MAS_ADD_STR   _("&Add to favourites")
#define MENU_SERVER_MAS_REM_STR   _("&Remove from master")
#define MENU_SERVER_FAV_ADD_STR   _("&Add server ...")
#define MENU_SERVER_FAV_REM_STR   _("&Remove from favourites")
#define MENU_SERVER_DEL_STR       _("&Delete server")
#define MENU_SERVER_DELM_STR      _("&Delete servers")

enum
{
    MENU_MASTER_ADD            =  wxID_HIGHEST + 1,
    MENU_MASTER_DEL,
    MENU_MASTER_UPDATE,

    MENU_SERVER_ADD,
    MENU_SERVER_CONNECT,
    MENU_SERVER_REM,
    MENU_SERVER_COPYTEXT,
    MENU_SERVER_DEL,

    MENU_VIEW_SEARCH,
    MENU_VIEW_FILTER,
    MENU_VIEW_AUTO_SORT,
    MENU_VIEW_AUTO_FIT,
    MENU_VIEW_SPLITTER_LIVE,
    MENU_VIEW_TOOLBAR,
    MENU_VIEW_TOOLBAR_TOP,
    MENU_VIEW_TOOLBAR_BOTTOM,
    MENU_VIEW_TOOLBAR_LEFT,
    MENU_VIEW_TOOLBAR_RIGHT,
    MENU_VIEW_TOOLBAR_TEXT,

    MENU_INFO_ABOUT,

    MENU_END
};


class CslMenu
{
    public:
        CslMenu(wxMenuBar *menuBar) : m_menuBar(menuBar)
        {
            EnableMenuItem(MENU_MASTER_ADD,false);
            EnableMenuItem(MENU_MASTER_DEL,false);
            EnableMenuItem(MENU_MASTER_UPDATE,false);

//             EnableMenuItem(MENU_SERVER_CONNECT,false);
//             EnableMenuItem(MENU_SERVER_ADD,false);
//             EnableMenuItem(MENU_SERVER_DEL,false);
//             EnableMenuItem(MENU_SERVER_COPYTEXT,false);

            CheckMenuItem(MENU_VIEW_FILTER,g_cslSettings->m_showFilter);
            CheckMenuItem(MENU_VIEW_SEARCH,g_cslSettings->m_showSearch);
            CheckMenuItem(MENU_VIEW_AUTO_FIT,g_cslSettings->m_autoFitColumns);
            CheckMenuItem(MENU_VIEW_AUTO_SORT,g_cslSettings->m_autoSortColumns);
            CheckMenuItem(MENU_VIEW_SPLITTER_LIVE,g_cslSettings->m_splitterLive);
        }

        void EnableMenuItem(wxInt32 id,bool enable=true)
        {
            //m_toolBar->EnableTool(id,enable);
            m_menuBar->Enable(id,enable);
        }

        void CheckMenuItem(wxInt32 id,bool check=true)
        {
            //m_toolBar->EnableTool(id,enable);
            m_menuBar->Check(id,check);
        }

    protected:
        wxMenuBar *m_menuBar;
};

extern CslMenu *g_cslMenu;

#endif // CSLMENU_H
