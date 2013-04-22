
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

#ifndef CSLMENU_H
#define CSLMENU_H

#include <CslArt.h>

#define MENU_SERVER_EXT_FULL_STR          _("Full ...")
#define MENU_SERVER_EXT_MICRO_STR         _("Micro")
#define MENU_SERVER_EXT_MINI_STR          _("Mini")
#define MENU_SERVER_EXT_DEFAULT_STR       _("Default")
#define MENU_SERVER_EXT_STR               _("Extended information")

#define MENU_SERVER_CONN_STR              _("&Connect")
#define MENU_SERVER_CONN_PW_STR           _("Connect (&Password)")
#define MENU_SERVER_MESSAGE_STR           _("Server &message ...")
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
#define MENU_SERVER_LOCATION_STR          _("Location")
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
    MENU_CUSTOM_START = wxID_HIGHEST+1,

    MENU_GAME_SERVER_COUNTRY,
    MENU_GAME_PLAYER_COUNTRY,
    MENU_GAME_SEARCH_LAN,

    MENU_SERVER_CONNECT,
    MENU_SERVER_CONNECT_PW,

    MENU_SERVER_EXT_MICRO,
    MENU_SERVER_EXT_MINI,
    MENU_SERVER_EXT_DEFAULT,
    MENU_SERVER_EXT_FULL,

    MENU_SERVER_MESSAGE,

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
    MENU_VIEW_SEARCH,
    MENU_VIEW_OUTPUT,
    MENU_VIEW_TRAFFIC,
    MENU_VIEW_AUTO_SORT,
    MENU_VIEW_RELAYOUT,

    SEARCH_TEXT,
    SEARCH_RADIO_SERVER,
    SEARCH_RADIO_PLAYER,
    SEARCH_BUTTON_SEARCH,

    // no new id's after this line
    MENU_LISTCTRL_COLUMN_ADJUST,
    MENU_LISTCTRL_COLUMN = wxID_HIGHEST+1000,
    MENU_SERVER_LOCATION = wxID_HIGHEST+1100,

    MENU_CUSTOM_END = wxID_HIGHEST+2000
};


#define CSL_MENU_CREATE_CONNECT(menu, info) \
    { \
        wxUint32 _caps=info->GetGame().GetCapabilities(); \
        \
        CslMenu::AddItem(menu, MENU_SERVER_CONNECT, MENU_SERVER_CONN_STR, wxART_CONNECT); \
        \
        if (CSL_CAP_CONNECT_PASS(_caps)) \
            CslMenu::AddItem(menu, MENU_SERVER_CONNECT_PW, MENU_SERVER_CONN_PW_STR, wxART_CONNECT_PW); \
        if (CslGameConnection::IsPlaying()) \
        { \
            menu.Enable(MENU_SERVER_CONNECT, false); \
            if (CSL_CAP_CONNECT_PASS(_caps)) \
                menu.Enable(MENU_SERVER_CONNECT_PW, false); \
        } \
    }

#define CSL_MENU_CREATE_EXTINFO(menu, info, _view) \
    { \
        wxMenuItem *item; \
        wxMenu *sub=new wxMenu; \
        \
        item=menu.AppendSubMenu(sub, MENU_SERVER_EXT_STR); \
        item->SetBitmap(GET_ART_MENU(wxART_ABOUT)); \
        \
        if (info->ExtInfoStatus!=CSL_EXT_STATUS_OK || !CslEngine::PingOk(*info, CslGetSettings().UpdateInterval)) \
            item->Enable(false); \
        else \
        { \
            if (_view!=CslListCtrlPlayerView::SIZE_FULL) \
            { \
                CslMenu::AddItem(*sub, MENU_SERVER_EXT_FULL, MENU_SERVER_EXT_FULL_STR, wxART_ABOUT); \
                sub->AppendSeparator(); \
            } \
            if (_view!=CslListCtrlPlayerView::SIZE_MICRO) \
                CslMenu::AddItem(*sub, MENU_SERVER_EXT_MICRO, MENU_SERVER_EXT_MICRO_STR, wxART_EXTINFO_MICRO); \
            if (_view!=CslListCtrlPlayerView::SIZE_MINI) \
                CslMenu::AddItem(*sub, MENU_SERVER_EXT_MINI, MENU_SERVER_EXT_MINI_STR, wxART_EXTINFO_MINI); \
            if (_view!=CslListCtrlPlayerView::SIZE_DEFAULT) \
                CslMenu::AddItem(*sub, MENU_SERVER_EXT_DEFAULT, MENU_SERVER_EXT_DEFAULT_STR, wxART_EXTINFO_DEFAULT); \
        } \
    }

#define CSL_MENU_CREATE_SRVMSG(menu, info) \
    wxMenuItem *item; \
    \
    item=&CslMenu::AddItem(menu, MENU_SERVER_MESSAGE, MENU_SERVER_MESSAGE_STR, wxART_ABOUT); \
    \
    if (!info || info->InfoText.IsEmpty()) \
        item->Enable(false);

#define CSL_MENU_CREATE_URICOPY(menu) \
    { \
        wxMenuItem *item; \
        wxMenu *sub=new wxMenu; \
        \
        item=menu.AppendSubMenu(sub, MENU_SERVER_COPY_STR); \
        item->SetBitmap(GET_ART_MENU(wxART_CSL)); \
        CslMenu::AddItem(*sub, MENU_SERVER_COPY_CON, MENU_SERVER_COPY_CON_STR, wxART_CSL); \
        CslMenu::AddItem(*sub, MENU_SERVER_COPY_FAV, MENU_SERVER_COPY_FAV_STR, wxART_CSL); \
        CslMenu::AddItem(*sub, MENU_SERVER_COPY_CONFAV, MENU_SERVER_COPY_CONFAV_STR, wxART_CSL); \
    }

#define CSL_MENU_CREATE_NOTIFY(menu, info) \
    { \
        wxMenuItem *item; \
        wxMenu *sub=new wxMenu; \
        \
        item=menu.AppendSubMenu(sub, MENU_SERVER_NOTIFY_STR); \
        item->SetBitmap(GET_ART_MENU(wxART_NOTIFICATION)); \
        /* \
        item=&CslMenu::AddItem(*sub, MENU_SERVER_NOTIFY_ONLINE, \
        MENU_SERVER_NOTIFY_ONLINE_STR, wxART_NONE, wxITEM_CHECK); \
        item->Check(info->HasRegisteredEvent(CslServerEvents::EVENT_ONLINE)); \
        item=&CslMenu::AddItem(*sub, MENU_SERVER_NOTIFY_OFFLINE, \
        MENU_SERVER_NOTIFY_OFFLINE_STR, wxART_NONE, wxITEM_CHECK); \
        item->Check(info->HasRegisteredEvent(CslServerEvents::EVENT_OFFLINE)); \
        */ \
        item=&CslMenu::AddItem(*sub, MENU_SERVER_NOTIFY_FULL, \
                                MENU_SERVER_NOTIFY_FULL_STR, wxART_NONE, wxITEM_CHECK); \
        item->Check(info->HasRegisteredEvent(CslServerEvents::EVENT_FULL)); \
        item=&CslMenu::AddItem(*sub, MENU_SERVER_NOTIFY_NOT_FULL, \
                                MENU_SERVER_NOTIFY_NOT_FULL_STR, wxART_NONE, wxITEM_CHECK); \
        item->Check(info->HasRegisteredEvent(CslServerEvents::EVENT_NOT_FULL)); \
        item=&CslMenu::AddItem(*sub, MENU_SERVER_NOTIFY_EMPTY, \
                                MENU_SERVER_NOTIFY_EMPTY_STR, wxART_NONE, wxITEM_CHECK); \
        item->Check(info->HasRegisteredEvent(CslServerEvents::EVENT_EMPTY)); \
        item=&CslMenu::AddItem(*sub, MENU_SERVER_NOTIFY_NOT_EMPTY, \
                                MENU_SERVER_NOTIFY_NOT_EMPTY_STR, wxART_NONE, wxITEM_CHECK); \
        item->Check(info->HasRegisteredEvent(CslServerEvents::EVENT_NOT_EMPTY)); \
        item=&CslMenu::AddItem(*sub, MENU_SERVER_NOTIFY_LOCKED, \
                                MENU_SERVER_NOTIFY_LOCKED_STR, wxART_NONE, wxITEM_CHECK); \
        item->Check(info->HasRegisteredEvent(CslServerEvents::EVENT_LOCKED)); \
        item=&CslMenu::AddItem(*sub, MENU_SERVER_NOTIFY_PRIVATE, \
                                MENU_SERVER_NOTIFY_PRIVATE_STR, wxART_NONE, wxITEM_CHECK); \
        item->Check(info->HasRegisteredEvent(CslServerEvents::EVENT_PRIVATE)); \
        sub->AppendSeparator(); \
        item=&CslMenu::AddItem(*sub, MENU_SERVER_NOTIFY_RESET, MENU_SERVER_NOTIFY_RESET_STR); \
    }

#define CSL_MENU_CREATE_LOCATION(menu) \
    { \
        wxMenuItem *item; \
        wxMenu *sub=new wxMenu; \
        \
        item=menu.AppendSubMenu(sub, MENU_SERVER_LOCATION_STR); \
        item->SetBitmap(GET_ART_MENU(wxART_GEO)); \
        \
        const CslGeoIPServices& _services=CslGeoIP::GetServices(); \
        for (wxUint32 _i=0; _i<_services.GetCount(); _i++) \
            CslMenu::AddItem(*sub, MENU_SERVER_LOCATION+_i, _services[_i]->Name, wxART_GEO); \
    }

#define CSL_MENU_CREATE_SAVEIMAGE(menu) \
    CslMenu::AddItem(menu, MENU_SAVEIMAGE, MENU_SERVER_SAVEIMG_STR, wxART_FILE_SAVE_AS);


#define CSL_MENU_EVENT_IS_CONNECT(id)   (id>=MENU_SERVER_CONNECT && id<=MENU_SERVER_CONNECT_PW)
#define CSL_MENU_EVENT_IS_EXTINFO(id)   (id>=MENU_SERVER_EXT_MICRO && id<=MENU_SERVER_EXT_FULL)
#define CSL_MENU_EVENT_IS_SRVMSG(id)    (id==MENU_SERVER_MESSAGE)
#define CSL_MENU_EVENT_IS_URICOPY(id)   (id>=MENU_SERVER_COPY_CON && id<=MENU_SERVER_COPY_FAV)
#define CSL_MENU_EVENT_IS_NOTIFY(id)    (id>=MENU_SERVER_NOTIFY_RESET && id<=MENU_SERVER_NOTIFY_PRIVATE)
#define CSL_MENU_EVENT_IS_FILTER(id)    (id>=MENU_SERVER_FILTER_OFF && id<=MENU_SERVER_FILTER_VER)
#define CSL_MENU_EVENT_IS_COLUMN(id)    (id>=MENU_LISTCTRL_COLUMN && id<MENU_SERVER_LOCATION)
#define CSL_MENU_EVENT_IS_LOCATION(id)  (id>=MENU_SERVER_LOCATION && \
                                         id<(wxInt32)(MENU_SERVER_LOCATION+CslGeoIP::GetServices().GetCount()))
#define CSL_MENU_EVENT_IS_SAVEIMAGE(id) (id==MENU_SAVEIMAGE)


#define CSL_MENU_EVENT_SKIP_CONNECT(id, info) \
    if (CSL_MENU_EVENT_IS_CONNECT(id)) \
    { \
        event.SetClientData(info); \
        event.Skip(); \
        return; \
    }

#define CSL_MENU_EVENT_SKIP_EXTINFO(id, info) \
    if (CSL_MENU_EVENT_IS_EXTINFO(id)) \
    { \
        event.SetClientData(info); \
        event.Skip(); \
        return; \
    }

#define CSL_MENU_EVENT_SKIP_SRVMSG(id, info) \
    if (CSL_MENU_EVENT_IS_SRVMSG(id)) \
    { \
        event.SetClientData(info); \
        event.Skip(); \
        return; \
    }

#define CSL_MENU_EVENT_SKIP_NOTIFY(id, info) \
    if (CSL_MENU_EVENT_IS_NOTIFY(id)) \
    { \
        event.SetClientData(info); \
        event.Skip(); \
        return; \
    }

#define CSL_MENU_EVENT_SKIP_LOCATION(id, _ip) \
    if (CSL_MENU_EVENT_IS_LOCATION(id)) \
    { \
        event.SetClientData((void*)_ip); \
        event.Skip(); \
        return; \
    }

#define CSL_MENU_EVENT_SKIP_SAVEIMAGE(id) \
    if (CSL_MENU_EVENT_IS_SAVEIMAGE(id)) \
    { \
        event.Skip(); \
        return; \
    }


class CSL_DLL_GUITOOLS CslMenu
{
    public:
        static void SetMainMenu(wxMenuBar *menu);

        static void EnableItem(wxInt32 id, bool enable=true);
        static void EnableItem(wxMenu& menu, wxInt32 id, bool enable=true);
        static void CheckItem(wxInt32 id, bool check=true);
        static void CheckItem(wxMenu& menu, wxInt32 id, bool check=true);

        static wxMenuItem& AddItem(wxMenu& menu, wxInt32 id,
                                   const wxString& text,
                                   const wxArtID& art=wxART_NONE,
                                   const wxItemKind kind=wxITEM_NORMAL,
                                   const wxString& help=wxEmptyString);

    private:
        CslMenu();

        static CslMenu& GetInstance();

        wxMenuBar *m_mainMenu;
};


#endif // CSLMENU_H
