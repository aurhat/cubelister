
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
 @author Glen Masgai <mimosius@gmx.de>
*/

#include "wx/wxprec.h"
#ifdef __BORLANDC__
#pragma hdrstop
#endif
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif
#include <wx/platinfo.h>
#include "CslArt.h"
#include "CslSettings.h"


#define MENU_SERVER_EXT_FULL_STR         _("Full")
#define MENU_SERVER_EXT_MICRO_STR        _("Micro")
#define MENU_SERVER_EXT_MINI_STR         _("Mini")
#define MENU_SERVER_EXT_DEFAULT_STR      _("Default")
#define MENU_SERVER_EXT_STR              _("Extended information")

#define MENU_SERVER_CONN_STR             _("&Connect")
#define MENU_SERVER_CONN_PW_STR          _("Connect (&Password)")
#define MENU_SERVER_COPY_STR             _("Create CSL links")
#define MENU_SERVER_COPY_CON_STR         _("Connect")
#define MENU_SERVER_COPY_CONFAV_STR      _("Connect && add to favourites")
#define MENU_SERVER_COPY_FAV_STR         _("Add to favourites")
#define MENU_SERVER_COPY_SERVER_STR      _("C&opy to clipboard")
#define MENU_SERVER_MAS_ADD_STR          _("&Add to favourites")
#define MENU_SERVER_FAV_ADD_STR          _("&Add server ...")
#define MENU_SERVER_FAV_REM_STR          _("&Remove from favourites")
#define MENU_SERVER_DEL_STR              _("&Delete server")
#define MENU_SERVER_DELM_STR             _("&Delete servers")

#define MENU_SERVER_FILTER_STR           _("Filter")
#define MENU_SERVER_FILTER_OFF_STR       _("Offline")
#define MENU_SERVER_FILTER_FULL_STR      _("Full")
#define MENU_SERVER_FILTER_EMPTY_STR     _("Empty")
#define MENU_SERVER_FILTER_NONEMPTY_STR  _("Not empty")
#define MENU_SERVER_FILTER_MM2_STR       _("Mastermode 2")
#define MENU_SERVER_FILTER_MM3_STR       _("Mastermode 3")
#define MENU_SERVER_FILTER_VER_STR       _("Version")

enum
{
    MENU_MASTER_ADD = wxID_ADD,
    MENU_MASTER_DEL = wxID_REMOVE,
    MENU_MASTER_UPDATE = wxID_REFRESH,

    MENU_SERVER_ADD = wxID_ADD,
    MENU_SERVER_REM = wxID_REMOVE,
    MENU_SERVER_DEL = wxID_DELETE,
    MENU_SERVER_COPY_SERVER = wxID_COPY,
    //custom events/art
    MENU_SERVER_CUSTOM = wxID_HIGHEST+1,

    MENU_GAME_SERVER_COUNTRY,
    MENU_GAME_PLAYER_COUNTRY,

    MENU_SERVER_CONNECT,
    MENU_SERVER_CONNECT_PW,
    MENU_SERVER_EXT_MICRO,
    MENU_SERVER_EXT_MINI,
    MENU_SERVER_EXT_DEFAULT,
    MENU_SERVER_EXT_FULL,

    MENU_SERVER_COPY_CON,
    MENU_SERVER_COPY_CONFAV,
    MENU_SERVER_COPY_FAV,

    MENU_SERVER_FILTER_OFF,
    MENU_SERVER_FILTER_FULL,
    MENU_SERVER_FILTER_EMPTY,
    MENU_SERVER_FILTER_NONEMPTY,
    MENU_SERVER_FILTER_MM2,
    MENU_SERVER_FILTER_MM3,
    MENU_SERVER_FILTER_VER,

    MENU_VIEW_GAMES,
    MENU_VIEW_SERVER_INFO,
    MENU_VIEW_PLAYER_LIST,
    MENU_VIEW_PLAYER_SEARCH,
    MENU_VIEW_COUNTRY,
    MENU_VIEW_FAVOURITES,
    MENU_VIEW_USER_CHAT,
    MENU_VIEW_SEARCH,
    MENU_VIEW_OUTPUT,
    MENU_VIEW_TRAFFIC,
    MENU_VIEW_AUTO_SORT,
    MENU_VIEW_RELAYOUT,

    MENU_END
};

class CslMenu
{
    public:
        CslMenu(wxMenuBar *menuBar);

        static void EnableMenuItem(const wxInt32 id,const bool enable=true);
        static void CheckMenuItem(const wxInt32 id,const bool check=true);
        static wxMenuItem& AddItem(wxMenu *menu,const wxInt32 id,
                                   const wxString& text,
                                   const wxArtID& art=wxART_NONE,
                                   const wxItemKind kind=wxITEM_NORMAL,
                                   const wxString& help=wxEmptyString);
    protected:
        static wxMenuBar *m_menuBar;
};


#endif // CSLMENU_H
