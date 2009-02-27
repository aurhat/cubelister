
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

#ifndef CSLMENU_H
#define CSLMENU_H

/**
 @author Glen Masgai <mimosius@users.sourceforge.net>
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


#define MENU_SERVER_EXT_FULL_STR         _("Full ...")
#define MENU_SERVER_EXT_MICRO_STR        _("Micro")
#define MENU_SERVER_EXT_MINI_STR         _("Mini")
#define MENU_SERVER_EXT_DEFAULT_STR      _("Default")
#define MENU_SERVER_EXT_STR              _("Extended information")

#define MENU_SERVER_CONN_STR              _("&Connect")
#define MENU_SERVER_CONN_PW_STR           _("Connect (&Password)")
#define MENU_SERVER_COPY_STR              _("Create CS&L links")
#define MENU_SERVER_COPY_CON_STR          _("Connec&t")
#define MENU_SERVER_COPY_CONFAV_STR       _("Connect && &add to favourites")
#define MENU_SERVER_COPY_FAV_STR          _("Add to &favourites")
#define MENU_SERVER_COPY_SERVER_STR       _("C&opy to clipboard")
#define MENU_SERVER_ADD_STR               _("&Add server ...")
#define MENU_SERVER_FAV_ADD_STR           _("&Add to favourites")
#define MENU_SERVER_FAV_REM_STR           _("&Remove from favourites")
#define MENU_SERVER_DEL_STR               _("&Delete server")
#define MENU_SERVER_DELM_STR              _("&Delete servers")
#define MENU_SERVER_SAVEIMG_STR           _("&Save as image ...")

#define MENU_SERVER_NOTIFY_STR            _("Notifications")
#define MENU_SERVER_NOTIFY_ONLINE_STR     _("Online")
#define MENU_SERVER_NOTIFY_OFFLINE_STR    _("Offline")
#define MENU_SERVER_NOTIFY_FULL_STR       _("Full")
#define MENU_SERVER_NOTIFY_NOT_FULL_STR   _("Not full")
#define MENU_SERVER_NOTIFY_EMPTY_STR      _("Empty")
#define MENU_SERVER_NOTIFY_NOT_EMPTY_STR  _("Not empty")
#define MENU_SERVER_NOTIFY_LOCKED_STR     _("Locked")
#define MENU_SERVER_NOTIFY_PRIVATE_STR    _("Private")
#define MENU_SERVER_NOTIFY_RESET_STR      _("Reset")

#define MENU_SERVER_FILTER_STR            _("Filters")
#define MENU_SERVER_FILTER_OFF_STR        _("Offline")
#define MENU_SERVER_FILTER_FULL_STR       _("Full")
#define MENU_SERVER_FILTER_EMPTY_STR      _("Empty")
#define MENU_SERVER_FILTER_NONEMPTY_STR   _("Not empty")
#define MENU_SERVER_FILTER_MM2_STR        _("Locked")
#define MENU_SERVER_FILTER_MM3_STR        _("Restricted")
#define MENU_SERVER_FILTER_VER_STR        _("Version")
#define MENU_SERVER_FILTER_RESET_STR      _("Reset")

enum
{
    MENU_ADD = wxID_ADD,
    MENU_REM = wxID_REMOVE,
    MENU_DEL = wxID_DELETE,
    MENU_COPY = wxID_COPY,
    MENU_UPDATE = wxID_REFRESH,
    MENU_SAVEIMAGE = wxID_SAVEAS,
    //custom events/art
    MENU_CUSTOM = wxID_HIGHEST+1,

    MENU_GAME_SERVER_COUNTRY,
    MENU_GAME_PLAYER_COUNTRY,

    MENU_SERVER_CONNECT,
    MENU_SERVER_CONNECT_PW,

    MENU_SERVER_EXT_MICRO,
    MENU_SERVER_EXT_MINI,
    MENU_SERVER_EXT_DEFAULT,
    MENU_SERVER_EXT_FULL,

    MENU_SERVER_NOTIFY_RESET,
    MENU_SERVER_NOTIFY_ONLINE,
    MENU_SERVER_NOTIFY_OFFLINE,
    MENU_SERVER_NOTIFY_FULL,
    MENU_SERVER_NOTIFY_NOT_FULL,
    MENU_SERVER_NOTIFY_EMPTY,
    MENU_SERVER_NOTIFY_NOT_EMPTY,
    MENU_SERVER_NOTIFY_LOCKED,
    MENU_SERVER_NOTIFY_PRIVATE,

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


#define CSL_MENU_CREATE_CONNECT(_menu,_info) \
    { \
        wxUint32 _caps=_info->GetGame().GetCapabilities(); \
        \
        CslMenu::AddItem(&_menu,MENU_SERVER_CONNECT,MENU_SERVER_CONN_STR,wxART_CONNECT); \
        \
        if (CSL_CAP_CONNECT_PASS(_caps)) \
            CslMenu::AddItem(&_menu,MENU_SERVER_CONNECT_PW,MENU_SERVER_CONN_PW_STR,wxART_CONNECT_PW); \
        if (CslGameConnection::IsPlaying()) \
        { \
            _menu.Enable(MENU_SERVER_CONNECT,false); \
            if (CSL_CAP_CONNECT_PASS(_caps)) \
                _menu.Enable(MENU_SERVER_CONNECT_PW,false); \
        } \
    }

#define CSL_MENU_CREATE_EXTINFO(_menu,_info,_view) \
    { \
        wxMenuItem *_item; \
        wxMenu *_sub=new wxMenu; \
        \
        _item=_menu.AppendSubMenu(_sub,MENU_SERVER_EXT_STR); \
        _item->SetBitmap(GET_ART_MENU(wxART_ABOUT)); \
        \
        if (_info->ExtInfoStatus!=CSL_EXT_STATUS_OK || !CslEngine::PingOk(*_info,g_cslSettings->updateInterval)) \
            _item->Enable(false); \
        else \
        { \
            if (_view!=CslListCtrlPlayer::SIZE_FULL) \
            { \
                CslMenu::AddItem(_sub,MENU_SERVER_EXT_FULL,MENU_SERVER_EXT_FULL_STR,wxART_ABOUT); \
                _sub->AppendSeparator(); \
            } \
            if (_view!=CslListCtrlPlayer::SIZE_MICRO) \
                CslMenu::AddItem(_sub,MENU_SERVER_EXT_MICRO,MENU_SERVER_EXT_MICRO_STR,wxART_EXTINFO_MICRO); \
            if (_view!=CslListCtrlPlayer::SIZE_MINI) \
                CslMenu::AddItem(_sub,MENU_SERVER_EXT_MINI,MENU_SERVER_EXT_MINI_STR,wxART_EXTINFO_MINI); \
            if (_view!=CslListCtrlPlayer::SIZE_DEFAULT) \
                CslMenu::AddItem(_sub,MENU_SERVER_EXT_DEFAULT,MENU_SERVER_EXT_DEFAULT_STR,wxART_EXTINFO_DEFAULT); \
        } \
    }

#define CSL_MENU_CREATE_URICOPY(_menu) \
    { \
        wxMenuItem *_item; \
        wxMenu *_sub=new wxMenu; \
        \
        _item=_menu.AppendSubMenu(_sub,MENU_SERVER_COPY_STR); \
        _item->SetBitmap(GET_ART_MENU(wxART_CSL)); \
        CslMenu::AddItem(_sub,MENU_SERVER_COPY_CON,MENU_SERVER_COPY_CON_STR,wxART_CSL); \
        CslMenu::AddItem(_sub,MENU_SERVER_COPY_FAV,MENU_SERVER_COPY_FAV_STR,wxART_CSL); \
        CslMenu::AddItem(_sub,MENU_SERVER_COPY_CONFAV,MENU_SERVER_COPY_CONFAV_STR,wxART_CSL); \
    }

#define CSL_MENU_CREATE_NOTIFY(_menu,_info) \
    { \
        wxMenuItem *_item; \
        wxMenu *_sub=new wxMenu; \
        \
        _item=_menu.AppendSubMenu(_sub,MENU_SERVER_NOTIFY_STR); \
        /*FIXME _item->SetBitmap(GET_ART_MENU(wxART_CSL));*/ \
        /* \
        _item=&CslMenu::AddItem(_sub,MENU_SERVER_NOTIFY_ONLINE, \
        MENU_SERVER_NOTIFY_ONLINE_STR,wxART_NONE,wxITEM_CHECK); \
        _item->Check(_info->HasRegisteredEvent(CslServerEvents::EVENT_ONLINE)); \
        _item=&CslMenu::AddItem(_sub,MENU_SERVER_NOTIFY_OFFLINE, \
        MENU_SERVER_NOTIFY_OFFLINE_STR,wxART_NONE,wxITEM_CHECK); \
        _item->Check(_info->HasRegisteredEvent(CslServerEvents::EVENT_OFFLINE)); \
        */ \
        _item=&CslMenu::AddItem(_sub,MENU_SERVER_NOTIFY_FULL, \
                                MENU_SERVER_NOTIFY_FULL_STR,wxART_NONE,wxITEM_CHECK); \
        _item->Check(_info->HasRegisteredEvent(CslServerEvents::EVENT_FULL)); \
        _item=&CslMenu::AddItem(_sub,MENU_SERVER_NOTIFY_NOT_FULL, \
                                MENU_SERVER_NOTIFY_NOT_FULL_STR,wxART_NONE,wxITEM_CHECK); \
        _item->Check(_info->HasRegisteredEvent(CslServerEvents::EVENT_NOT_FULL)); \
        _item=&CslMenu::AddItem(_sub,MENU_SERVER_NOTIFY_EMPTY, \
                                MENU_SERVER_NOTIFY_EMPTY_STR,wxART_NONE,wxITEM_CHECK); \
        _item->Check(_info->HasRegisteredEvent(CslServerEvents::EVENT_EMPTY)); \
        _item=&CslMenu::AddItem(_sub,MENU_SERVER_NOTIFY_NOT_EMPTY, \
                                MENU_SERVER_NOTIFY_NOT_EMPTY_STR,wxART_NONE,wxITEM_CHECK); \
        _item->Check(_info->HasRegisteredEvent(CslServerEvents::EVENT_NOT_EMPTY)); \
        _item=&CslMenu::AddItem(_sub,MENU_SERVER_NOTIFY_LOCKED, \
                                MENU_SERVER_NOTIFY_LOCKED_STR,wxART_NONE,wxITEM_CHECK); \
        _item->Check(_info->HasRegisteredEvent(CslServerEvents::EVENT_LOCKED)); \
        _item=&CslMenu::AddItem(_sub,MENU_SERVER_NOTIFY_PRIVATE, \
                                MENU_SERVER_NOTIFY_PRIVATE_STR,wxART_NONE,wxITEM_CHECK); \
        _item->Check(_info->HasRegisteredEvent(CslServerEvents::EVENT_PRIVATE)); \
        _sub->AppendSeparator(); \
        _item=&CslMenu::AddItem(_sub,MENU_SERVER_NOTIFY_RESET,MENU_SERVER_NOTIFY_RESET_STR); \
    }

#define CSL_MENU_CREATE_SAVEIMAGE(_menu) \
    CslMenu::AddItem(&_menu,MENU_SAVEIMAGE,MENU_SERVER_SAVEIMG_STR,wxART_FILE_SAVE_AS);


#define CSL_MENU_EVENT_IS_CONNECT(_id)   (_id>=MENU_SERVER_CONNECT && _id<=MENU_SERVER_CONNECT_PW)
#define CSL_MENU_EVENT_IS_EXTINFO(_id)   (_id>=MENU_SERVER_EXT_MICRO && _id<=MENU_SERVER_EXT_FULL)
#define CSL_MENU_EVENT_IS_URICOPY(_id)   (_id>=MENU_SERVER_COPY_CON && _id<=MENU_SERVER_COPY_FAV)
#define CSL_MENU_EVENT_IS_NOTIFY(_id)    (_id>=MENU_SERVER_NOTIFY_RESET && _id<=MENU_SERVER_NOTIFY_PRIVATE)
#define CSL_MENU_EVENT_IS_FILTER(_id)    (_id>=MENU_SERVER_FILTER_OFF && _id<=MENU_SERVER_FILTER_VER)
#define CSL_MENU_EVENT_IS_SAVEIMAGE(_id) (_id==MENU_SAVEIMAGE)


#define CSL_MENU_EVENT_SKIP_CONNECT(_id,_info) \
    if (CSL_MENU_EVENT_IS_CONNECT(_id)) \
    { \
        event.SetClientData(_info); \
        event.Skip(); \
        return; \
    }

#define CSL_MENU_EVENT_SKIP_EXTINFO(_id,_info) \
    if (CSL_MENU_EVENT_IS_EXTINFO(_id)) \
    { \
        event.SetClientData(_info); \
        event.Skip(); \
        return; \
    }

#define CSL_MENU_EVENT_SKIP_NOTIFY(_id,_info) \
    if (CSL_MENU_EVENT_IS_NOTIFY(_id)) \
    { \
        event.SetClientData(_info); \
        event.Skip(); \
        return; \
    }

#define CSL_MENU_EVENT_SKIP_SAVEIMAGE(_id) \
    if (CSL_MENU_EVENT_IS_SAVEIMAGE(_id)) \
    { \
        event.Skip(); \
        return; \
    }


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
