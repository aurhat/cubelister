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

#include "Csl.h"
#include "CslEngine.h"
#include "CslApp.h"
#include "CslArt.h"
#include "CslGeoIP.h"
#include "CslDlgAddServer.h"
#include "CslDlgOutput.h"
#include "CslFrame.h"
#include "CslGameConnection.h"
#include "CslMenu.h"
#include "CslPanelMap.h"
#include "CslPanelServerView.h"
#include "CslSettings.h"
#include "CslStatusBar.h"
#include "CslServerBrowser.h"

enum
{
    COLUMN_HOST,
    COLUMN_NAME,
    COLUMN_VERSION,
    COLUMN_PING,
    COLUMN_MODE,
    COLUMN_MAP,
    COLUMN_TIME,
    COLUMN_PLAYER,
    COLUMN_MM,
    COLUMN_MAX
};

static const wxChar *ColumnNames[] =
{
    _("Host"),
    _("Name"),
    _("Version"),
    _("Ping"),
    _("Mode"),
    _("Map"),
    _("Time"),
    _("Player"),
    _("Mastermode")
};

BEGIN_EVENT_TABLE(CslListCtrlServer, CslListCtrl)
    EVT_KEY_DOWN(CslListCtrlServer::OnKeyDown)
    EVT_MENU(wxID_ANY,CslListCtrlServer::OnMenu)
    EVT_LIST_ITEM_ACTIVATED(wxID_ANY,CslListCtrlServer::OnItemActivated)
    CSL_EVT_LIST_COLUMN_TOGGLED(wxID_ANY,CslListCtrlServer::OnToggleColumn)
    CSL_EVT_LIST_ALL_ITEMS_SELECTED(wxID_ANY, CslListCtrlServer::OnItemSelected)
    EVT_LIST_ITEM_SELECTED(wxID_ANY,CslListCtrlServer::OnItemSelected)
    EVT_LIST_ITEM_DESELECTED(wxID_ANY,CslListCtrlServer::OnItemDeselected)
    EVT_CONTEXT_MENU(CslListCtrlServer::OnContextMenu)
END_EVENT_TABLE()


IMPLEMENT_DYNAMIC_CLASS(CslListCtrlServer, CslListCtrl)

CslListCtrlServer::CslListCtrlServer(wxWindow* parent,wxWindowID id,const wxPoint& pos,
                                     const wxSize& size,long style,
                                     const wxValidator& validator, const wxString& name) :
        CslListCtrl(parent,id,pos,size,style,validator,name),
        m_id(id),m_masterSelected(false),m_sibling(NULL),
        m_filterFlags(NULL)
{
}

CslListCtrlServer::~CslListCtrlServer()
{
    WX_CLEAR_ARRAY(m_servers);
}

void CslListCtrlServer::Init(CslListCtrlServer *sibling, wxInt32 *filter)
{
    m_sibling=sibling;
    m_filterVersion=-1;
    m_filterFlags=filter;

    wxUint32 columns=CslSettings::GetListSettings(GetName()).ColumnMask;

    #define COLENABLED(id) (columns>0 ? CSL_FLAG_CHECK(columns, 1<<(id)) : true)
    ListAddColumn(ColumnNames[COLUMN_HOST],    wxLIST_FORMAT_LEFT,  2.5f, COLENABLED(COLUMN_HOST),    !COLENABLED(COLUMN_NAME));
    ListAddColumn(ColumnNames[COLUMN_NAME],    wxLIST_FORMAT_LEFT,  3.0f, COLENABLED(COLUMN_NAME),    !COLENABLED(COLUMN_HOST));
    ListAddColumn(ColumnNames[COLUMN_VERSION], wxLIST_FORMAT_LEFT,  1.2f, COLENABLED(COLUMN_VERSION), false                   );
    ListAddColumn(ColumnNames[COLUMN_PING],    wxLIST_FORMAT_RIGHT, 1.0f, COLENABLED(COLUMN_PING),    false                   );
    ListAddColumn(ColumnNames[COLUMN_MODE],    wxLIST_FORMAT_LEFT,  2.0f, COLENABLED(COLUMN_MODE),    false                   );
    ListAddColumn(ColumnNames[COLUMN_MAP],     wxLIST_FORMAT_LEFT,  1.5f, COLENABLED(COLUMN_MAP),     false                   );
    ListAddColumn(ColumnNames[COLUMN_TIME],    wxLIST_FORMAT_RIGHT, 1.0f, COLENABLED(COLUMN_TIME),    false                   );
    ListAddColumn(ColumnNames[COLUMN_PLAYER],  wxLIST_FORMAT_LEFT,  1.0f, COLENABLED(COLUMN_PLAYER),  false                   );
    ListAddColumn(ColumnNames[COLUMN_MM],      wxLIST_FORMAT_LEFT,  1.4f, COLENABLED(COLUMN_MM),      false                   );
    #undef COLENABLED

    SetSortCallback(ListSortCompareCallBack, CslListSort::SORT_DSC, COLUMN_PLAYER);
}

void CslListCtrlServer::Clear()
{
    DeleteAllItems();

    m_selected.Clear();
    WX_CLEAR_ARRAY(m_servers);

    m_filterVersion = -1;
}

wxUint32 CslListCtrlServer::Rebuild(bool force)
{
    wxUint32 count = 0;

    wxWindowUpdateLocker locker(this);

    if (force)
        DeleteAllItems();

    loopv(m_servers)
    {
        CslListServerData *data = m_servers[i];

        if (force)
            data->Reset();

        if (UpdateServer(data->Info))
            count++;
    }

    ListSort();

    return count;
}

void CslListCtrlServer::DeleteServers()
{
    wxInt32 i;
    wxString msg;
    CslServerInfo *info;
    CslListServerData *data;
    wxInt32 skipFav=wxCANCEL,skipStats=wxCANCEL;

    for (i=m_selected.GetCount()-1;i>=0;i--)
    {
        msg.Empty();
        data=(CslListServerData*)m_selected.Item(i);
        info=data->Info;

        if (skipFav==wxCANCEL && info->IsFavourite())
        {
            msg=_("You are about to delete servers which are favourites.");
            msg<<CSL_NEWLINE_WX<<_("Really delete these servers?");
            skipFav=wxMessageBox(msg,_("Warning"),wxYES_NO|wxCANCEL|wxICON_WARNING,this);

            if (skipFav==wxCANCEL)
                return;
            if (skipFav==wxNO)
                continue;
        }
        if (skipStats==wxCANCEL && info->HasStats())
        {
            msg=_("You are about to delete servers which have statistics.");
            msg<<CSL_NEWLINE_WX<<_("Really delete these servers?");
            skipStats=wxMessageBox(msg,_("Warning"),wxYES_NO|wxCANCEL|wxICON_WARNING,this);

            if (skipStats==wxCANCEL)
                return;
            if (skipStats==wxNO)
                continue;
        }
    }

    for (i=m_selected.GetCount()-1;i>=0;i--)
    {
        data=(CslListServerData*)m_selected.Item(i);
        info=data->Info;

        if (data->Info->IsLocked())
        {
            msg=wxString::Format(_("Server \"%s\" is currently locked, so deletion is not possible!"),
                                 info->GetBestDescription().c_str());
            wxMessageBox(msg,_("Error"),wxICON_ERROR,this);
            continue;
        }

        if ((info->IsFavourite() && skipFav==wxNO) ||
                (info->HasStats() && skipStats==wxNO))
            continue;

        RemoveServer(data, info, i);

        if (m_id==CSL_LIST_MASTER && info->IsFavourite())
            m_sibling->RemoveServer(NULL,info,-1);
        else if (m_id==CSL_LIST_FAVOURITE)
            m_sibling->RemoveServer(NULL,info,-1);

        wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED,MENU_DEL);
        event.SetClientData((void*)info);
        wxPostEvent(m_parent,event);
    }
}

void CslListCtrlServer::RemoveServers()
{
    wxInt32 i, pos;
    wxListItem item;
    CslListServerData *data;

    for (i=m_selected.GetCount()-1;i>=0;i--)
    {
        data=(CslListServerData*)m_selected.Item(i);

        if ((pos=ListFindItem(data))!=wxNOT_FOUND)
        {
            // remove selection before removing item
            // since removing the item calls OnItemDeselected()
            // at least on Windows
            m_selected.RemoveAt(i);

            item.SetId(pos);
            DeleteItem(item);

            m_servers.Remove(data);

            if (m_id==CSL_LIST_FAVOURITE)
                data->Info->RemoveFavourite();
        }
    }
}

void CslListCtrlServer::RemoveServer(CslListServerData *server,CslServerInfo *info,wxInt32 selId)
{
    wxInt32 i;
    wxListItem item;

    if (!server)
    {
        for (i=m_servers.GetCount()-1; i>=0; i--)
        {
            if (m_servers.Item(i)->Info==info)
            {
                server=m_servers.Item(i);
                break;
            }
        }
    }

    if (!server)
        return;

    if (selId<0)
    {
        if ((i=m_selected.Index(server))!=wxNOT_FOUND)
            m_selected.RemoveAt(i);
    }
    else
        m_selected.RemoveAt(selId);

    if ((i=ListFindItem(server))!=wxNOT_FOUND)
    {
        item.SetId(i);
        DeleteItem(item);
    }

    m_servers.Remove(server);
    delete server;
}

wxUint32 CslListCtrlServer::SearchServers(const wxString& search)
{
    m_searchString = search.Lower();

    if (m_searchString.IsEmpty())
    {
        Highlight(CSL_HIGHLIGHT_SEARCH, false, true);
        return 0;
    }

    wxUint32 matches = 0;
    wxWindowUpdateLocker locker(this);

    loopi(m_servers.GetCount())
    {
        CslServerInfo *info = m_servers.Item(i)->Info;

        if (UpdateServer(info))
            matches++;
    }

    if (matches)
        ListSort();

    return matches;
}

bool CslListCtrlServer::UpdateServer(CslServerInfo *info)
{
    if (!info)
        return false;

    wxString s;
    wxInt32 i, j;
    wxListItem item;
    CslListServerData *infoCmp = NULL;

    for (i = m_servers.GetCount()-1; i>=0; i--)
    {
        if (m_servers[i]->Info==info)
        {
            infoCmp = m_servers[i];
            break;
        }
    }

    if ((j = ListFindItemByServerInfo(info))==wxNOT_FOUND)
        i = GetItemCount();
    else
        i = j;

    item.SetId(i);
    item.SetMask(wxLIST_MASK_TEXT|wxLIST_MASK_DATA|wxLIST_MASK_IMAGE);

    // filter
    bool filtered = m_searchString.IsEmpty() && (*m_filterFlags || m_filterVersion>-1) ?
                        ListCheckFilter(info) : false;

    if (filtered && j!=wxNOT_FOUND)
    {
        DeleteItem(item);

        wxInt32 pos = m_selected.Index(infoCmp);

        if (pos!=wxNOT_FOUND)
            m_selected.RemoveAt(pos);
    }

    if (j==wxNOT_FOUND)
    {
        if (!infoCmp)
        {
            infoCmp = new CslListServerData(info);
            m_servers.Add(infoCmp);
        }
        else
            infoCmp->Reset();

        if (filtered)
        {
            if (!m_searchString.IsEmpty())
                return ListCheckSearch(info);

            return true;
        }

        InsertItem(item);
        SetItemPtrData(i, (wxUIntPtr)infoCmp);
    }
    else
    {
        if (filtered)
        {
            if (!m_searchString.IsEmpty())
                return ListCheckSearch(info);

            return true;
        }

        infoCmp = (CslListServerData*)GetItemData(item);
    }

    if (infoCmp->Host!=info->Host)
        ListSetItem(i, COLUMN_HOST, FormatServerEntity(s, COLUMN_HOST, info));

    if (CslEngine::PingOk(*info, CslGetSettings().UpdateInterval) || info->PingResp)
    {
        if (infoCmp->Description!=info->Description)
            ListSetItem(i, COLUMN_NAME, FormatServerEntity(s, COLUMN_NAME, info));

        if (infoCmp->Protocol!=info->Protocol)
            ListSetItem(i, COLUMN_VERSION, FormatServerEntity(s, COLUMN_VERSION, info));

        if (infoCmp->Ping!=info->Ping)
            ListSetItem(i, COLUMN_PING, FormatServerEntity(s, COLUMN_PING, info));

        if (infoCmp->GameMode!=info->GameMode)
            ListSetItem(i, COLUMN_MODE, FormatServerEntity(s, COLUMN_MODE, info));

        if (infoCmp->Map!=info->Map)
            ListSetItem(i, COLUMN_MAP, FormatServerEntity(s, COLUMN_MAP, info));

        if (infoCmp->TimeRemain!=info->TimeRemain)
            ListSetItem(i, COLUMN_TIME, FormatServerEntity(s, COLUMN_TIME, info));

        if (infoCmp->Players!=info->Players || infoCmp->PlayersMax!=info->PlayersMax)
            ListSetItem(i, COLUMN_PLAYER, FormatServerEntity(s, COLUMN_PLAYER, info));

        if (infoCmp->MM!=info->MM)
            ListSetItem(i, COLUMN_MM, FormatServerEntity(s, COLUMN_MM, info));
    }
    else
    {
        if (info->Description.IsEmpty() && !info->DescriptionOld.IsEmpty())
            ListSetItem(i, COLUMN_NAME, FormatServerEntity(s, COLUMN_NAME, info, false));
    }

    wxColour colour(CSL_SYSCOLOUR(wxSYS_COLOUR_WINDOWTEXT));

    if (!CslEngine::PingOk(*info, CslGetSettings().UpdateInterval))
        colour=CslGetSettings().ColServerOff;
    else if (CSL_SERVER_IS_BAN(info->MM) ||
             CSL_SERVER_IS_PASSWORD(info->MM) ||
             CSL_SERVER_IS_BLACKLIST(info->MM))
        colour=CslGetSettings().ColServerMM3;
    else
    {
        if (info->IsFull())
            colour=CslGetSettings().ColServerFull;
        else if (CSL_SERVER_IS_PRIVATE(info->MM))
            colour=CslGetSettings().ColServerMM3;
        else if (CSL_SERVER_IS_LOCKED(info->MM))
            colour=CslGetSettings().ColServerMM2;
        else if (CSL_SERVER_IS_VETO(info->MM))
            colour=CslGetSettings().ColServerMM1;
        else if (info->Players==0)
            colour=CslGetSettings().ColServerEmpty;
    }

    SetItemTextColour(i,colour);

    // search
    bool found = m_searchString.IsEmpty() ? false : ListCheckSearch(info);
    Highlight(CSL_HIGHLIGHT_FOUND_SERVER, found, false, info, &item);

    if (m_id==CSL_LIST_MASTER)
    {
        if (!CslEngine::PingOk(*info, CslGetSettings().UpdateInterval))
            i = CSL_LIST_IMG_GREY;
        else if (info->Ping>(wxInt32)CslGetSettings().PingBad)
            i = info->ExtInfoStatus!=CSL_EXT_STATUS_FALSE ? CSL_LIST_IMG_RED_EXT : CSL_LIST_IMG_RED;
        else if (info->Ping>(wxInt32)CslGetSettings().PingGood)
            i = info->ExtInfoStatus!=CSL_EXT_STATUS_FALSE ? CSL_LIST_IMG_YELLOW_EXT : CSL_LIST_IMG_YELLOW;
        else
            i = info->ExtInfoStatus!=CSL_EXT_STATUS_FALSE ? CSL_LIST_IMG_GREEN_EXT : CSL_LIST_IMG_GREEN;

        if (infoCmp->ImgId!=i)
        {
            SetItemImage(item,i);
            infoCmp->ImgId = i;
        }
    }
    else
    {
        i = GetGameImage(::wxGetApp().GetCslEngine(), info->GetGame().GetFourCC(), info->ExtInfoStatus==CSL_EXT_STATUS_OK ? 1 : 0);

        if (infoCmp->ImgId!=i)
        {
            SetItemImage(item,i);
            infoCmp->ImgId=i;
        }
    }

    *infoCmp = *info;

    return found;
}

wxUint32 CslListCtrlServer::UpdateServers(CslArrayCslServerInfo& servers)
{
    wxUint32 c=0;

    wxWindowUpdateLocker locker(this);

    loopv(servers)
    {
        CslServerInfo *info=servers[i];

        switch (m_id)
        {
            case CSL_LIST_MASTER:
                if (m_masterSelected && !info->IsDefault() && !info->IsUnused())
                    continue;
                break;
            case CSL_LIST_FAVOURITE:
                if (!info->IsFavourite())
                    break;
        }

        if (UpdateServer(info))
            c++;
    }

    if (!servers.size() || !CslGetSettings().AutoSortColumns)
        return c;

    ListSort();

    return c;
}

void CslListCtrlServer::Highlight(wxInt32 type,bool high,bool sort,CslServerInfo *info,wxListItem *item)
{
    wxInt32 pos;
    wxUint32 i,t;
    wxListItem it;

    wxColour colour;

    if (info)
    {
        if (!item)
        {
            if ((pos=ListFindItemByServerInfo(info))==wxNOT_FOUND)
                return;

            item=&it;
            it.SetId(pos);
        }

        CslListServerData *server=(CslListServerData*)GetItemData(*item);
        t=server->SetHighlight(type,high);

        if (CSL_FLAG_CHECK(t, CSL_HIGHLIGHT_LOCKED))
            colour=CslGetSettings().ColServerPlay;
        else if (CSL_FLAG_CHECK(t, CSL_HIGHLIGHT_FOUND_SERVER) ||
                 CSL_FLAG_CHECK(t, CSL_HIGHLIGHT_FOUND_PLAYER))
            colour=CslGetSettings().ColServerHigh;
        else if (CSL_FLAG_CHECK(t, CSL_HIGHLIGHT_SEARCH_PLAYER))
            colour=wxColour(CslGetSettings().ColServerOff);
        else
            colour=GetBackgroundColour();

        SetItemBackgroundColour(*item,colour);
    }
    else
    {
        for (i=0;i<m_servers.GetCount();i++)
        {
            info=m_servers.Item(i)->Info;

            if ((pos=ListFindItemByServerInfo(info))==wxNOT_FOUND)
                continue;

            it.SetId(pos);

            t=m_servers.Item(i)->SetHighlight(type,high);

            if (CSL_FLAG_CHECK(t, CSL_HIGHLIGHT_FOUND_SERVER) ||
                CSL_FLAG_CHECK(t, CSL_HIGHLIGHT_FOUND_PLAYER))
                colour=CslGetSettings().ColServerHigh;
            else if (CSL_FLAG_CHECK(t, CSL_HIGHLIGHT_LOCKED))
                colour=CslGetSettings().ColServerPlay;
            else
                colour=GetBackgroundColour();

            SetItemBackgroundColour(it,colour);
        }
    }

    if (sort)
        ListSort();
}

wxUint32 CslListCtrlServer::GetPlayerCount()
{
    wxUint32 count = 0;

    loopv(m_servers)
    {
        CslListServerData *data = m_servers[i];

        if (data->Players>0 &&
            CslEngine::PingOk(*data->Info, CslGetSettings().UpdateInterval))
            count += data->Players;
    }

    return count;
}

wxInt32 CslListCtrlServer::ListFindItemByServerInfo(CslServerInfo *info)
{
    wxListItem item;

    for (wxInt32 i=GetItemCount()-1; i>=0; i--)
    {
        item.SetId(i);

        if (((CslListServerData*)GetItemData(item))->Info==info)
            return i;
    }

    return wxNOT_FOUND;
}

bool CslListCtrlServer::ListCheckSearch(CslServerInfo *info)
{
    if (info->Host.Lower().Find(m_searchString)!=wxNOT_FOUND)
        return true;
    if (info->Description.Lower().Find(m_searchString)!=wxNOT_FOUND)
        return true;
    if (info->GameMode.Lower().Find(m_searchString)!=wxNOT_FOUND)
        return true;
    if (info->Map.Lower().Find(m_searchString)!=wxNOT_FOUND)
        return true;

    return false;
}

bool CslListCtrlServer::ListCheckFilter(CslServerInfo *info)
{
    if (m_filterVersion>-1 && info->Protocol!=m_filterVersion)
        return true;
    if (CSL_FLAG_CHECK(*m_filterFlags, CSL_FILTER_OFFLINE) &&
        !CslEngine::PingOk(*info, CslGetSettings().UpdateInterval))
        return true;
    if (CSL_FLAG_CHECK(*m_filterFlags, CSL_FILTER_FULL) && info->PlayersMax>0 &&
        info->Players>=info->PlayersMax)
        return true;
    if (CSL_FLAG_CHECK(*m_filterFlags, CSL_FILTER_EMPTY) && info->Players==0)
        return true;
    if (CSL_FLAG_CHECK(*m_filterFlags, CSL_FILTER_NONEMPTY) && info->Players>0)
        return true;
    if (CSL_FLAG_CHECK(*m_filterFlags, CSL_FILTER_MM2) && info->MM==2)
        return true;
    if (CSL_FLAG_CHECK(*m_filterFlags, CSL_FILTER_MM3) && info->MM==3)
        return true;

    return false;
}

void CslListCtrlServer::GetToolTipText(wxInt32 row,CslToolTipEvent& event)
{
    CslServerInfo *info;
    CslListServerData *data;

    if (row>=GetItemCount() || !(data=(CslListServerData*)GetItemData(row)))
        return;

    info = data->Info;

    event.Title=_("Server information");

    wxString s;
    bool pingok=CslEngine::PingOk(*data->Info, CslGetSettings().UpdateInterval);

    for (wxInt32 i = 0; i<COLUMN_MAX; i++)
    {
        event.Text.Add(ColumnNames[i]);

        if (FormatServerEntity(s, i, info, pingok).IsEmpty())
            s = _("no data");

        event.Text.Add(s);
    }

    if (pingok && !info->Map.IsEmpty())
    {
        CslMapInfo mapinfo;
        wxArrayString paths;
        wxBitmap& bmp = mapinfo.m_bitmap;

        // reverse order as within the "full" extinfo dialog
        // check for previews here, provided by the game or
        // map authors first since base-infos aren't important

        // check game paths
        if (info->GetGame().GetMapImagePaths(paths))
        {
            loopv(paths)
            {
                if (mapinfo.LoadMapImage(info->Map, paths.Item(i)))
                    break;
            }
        }

        // not found, check the out map-data folder
        if (!bmp.IsOk() && mapinfo.LoadMapData(info->Map, info->GetGame().GetName(), info->Protocol))
            bmp = mapinfo.m_bitmap;

        // some image was found
        if (bmp.IsOk())
        {
            // TODO: should min size base on screen resolution ?
            if (bmp.GetWidth()>150)
                bmp = RescaleBitmap(bmp, wxSize(150, 150), true);

              CslPanelMap *pane = new CslPanelMap(event.Parent, wxID_ANY);
            pane->SetMap(bmp);
            event.UserWindow = pane;
         }
    }
}

inline wxString& CslListCtrlServer::FormatServerEntity(wxString& in, int type, CslServerInfo *info, bool pingok)
{
    in.Empty();

    switch (type)
    {
        case COLUMN_HOST:
            in<<info->Host;
            break;
        case COLUMN_NAME:
            if (!pingok && info->Description.IsEmpty() && !info->DescriptionOld.IsEmpty())
                in<<info->DescriptionOld;
            else
                in<<info->Description;
            break;
        case COLUMN_VERSION:
            in<<info->Version;
            break;
        case COLUMN_PING:
            in<<info->Ping;
            break;
        case COLUMN_MODE:
            in<<info->GameMode;
            break;
        case COLUMN_MAP:
            in<<info->Map;
            break;
        case COLUMN_TIME:
            if (info->TimeRemain<0)
                in<<_("no limit");
            else
                in<<(info->TimeRemain+60-1)/60;
            break;
        case COLUMN_PLAYER:
            if (info->PlayersMax<=0)
                in<<info->Players;
            else
                in<<in.Format(wxT("%d/%d"), info->Players, info->PlayersMax);
            break;
        case COLUMN_MM:
            in<<info->MMDescription;
            break;
        default: break;
    }

    return in;
}

void CslListCtrlServer::OnKeyDown(wxKeyEvent &event)
{
    switch (event.GetKeyCode())
    {
        case WXK_DELETE:
        case WXK_NUMPAD_DELETE:
            //TODO multiple events?
            if (event.ShiftDown())
                DeleteServers();
            else if (m_id!=CSL_LIST_MASTER)
                RemoveServers();
            break;
    }

    event.Skip();
}

void CslListCtrlServer::OnToggleColumn(wxListEvent& event)
{
    wxInt32 lock = event.GetColumn()==COLUMN_HOST ? COLUMN_NAME : COLUMN_HOST;

    ListLockColumn(lock, !ListColumnIsEnabled(event.GetColumn()));

    Rebuild(true);
}

void CslListCtrlServer::OnItemActivated(wxListEvent& event)
{
    CslListServerData *s=(CslListServerData*)GetItemData(event.GetIndex());
    event.SetClientData((void*)s->Info);
    event.Skip();
}

void CslListCtrlServer::OnItemSelected(wxListEvent& event)
{
    CslListServerData *s=(CslListServerData*)GetItemData(event.GetIndex());
    m_selected.Add(s);

    //dont't skip the event when selecting all items
    if (event.GetEventType()!=wxCSL_EVT_COMMAND_LIST_ALL_ITEMS_SELECTED)
    {
        event.SetClientData((void*)s->Info);
        event.Skip();
    }
}

void CslListCtrlServer::OnItemDeselected(wxListEvent& event)
{
    wxInt32 i;
    CslListServerData *s=(CslListServerData*)GetItemData(event.GetIndex());

    if ((i=m_selected.Index(s))!=wxNOT_FOUND)
        m_selected.RemoveAt(i);
}

void CslListCtrlServer::OnContextMenu(wxContextMenuEvent& event)
{
    wxInt32 selected;
    wxMenuItem *item;
    wxMenu menu,*filter=NULL;
    CslServerInfo *info=NULL;
    wxPoint point=event.GetPosition();

    //from keyboard
    if (point==wxDefaultPosition)
        point=wxGetMousePosition();

    if ((selected=m_selected.GetCount())>0)
        info=((CslListServerData*)m_selected.Item(0))->Info;

    if (selected==1)
    {
        CSL_MENU_CREATE_CONNECT(menu,info)
        CSL_MENU_CREATE_EXTINFO(menu,info,-1)
        menu.AppendSeparator();
        CSL_MENU_CREATE_SRVMSG(menu,info)
        menu.AppendSeparator();
    }

    if (m_id==CSL_LIST_MASTER && selected)
        CslMenu::AddItem(menu, MENU_ADD, MENU_SERVER_FAV_ADD_STR, wxART_ADD_BOOKMARK);

    else if (m_id==CSL_LIST_FAVOURITE)
    {
        CslMenu::AddItem(menu, MENU_ADD, MENU_SERVER_ADD_STR, wxART_ADD_BOOKMARK);
        if (selected)
            CslMenu::AddItem(menu, MENU_REM, MENU_SERVER_FAV_REM_STR, wxART_DEL_BOOKMARK);
    }

    if (selected>0)
    {
        CslMenu::AddItem(menu, MENU_DEL, selected>1 ?
                         MENU_SERVER_DELM_STR:MENU_SERVER_DEL_STR,wxART_DELETE);
        menu.AppendSeparator();
        CSL_MENU_CREATE_URICOPY(menu)
        CslMenu::AddItem(menu, MENU_COPY, MENU_SERVER_COPY_SERVER_STR, wxART_COPY);
        menu.AppendSeparator();
        filter=(wxMenu*)1;
    }
    else if (m_id==CSL_LIST_FAVOURITE)
    {
        menu.AppendSeparator();
        filter=(wxMenu*)1;
    }

    if (filter)
    {
        filter=new wxMenu();
        item=menu.AppendSubMenu(filter,MENU_SERVER_FILTER_STR);
        item->SetBitmap(GET_ART_MENU(wxART_FILTER));
    }
    else
        filter=&menu;

    item=&CslMenu::AddItem(*filter, MENU_SERVER_FILTER_OFF,
                           MENU_SERVER_FILTER_OFF_STR,wxART_NONE,wxITEM_CHECK);
    item->Check(CSL_FLAG_CHECK(*m_filterFlags, CSL_FILTER_OFFLINE));
    item=&CslMenu::AddItem(*filter, MENU_SERVER_FILTER_FULL,
                           MENU_SERVER_FILTER_FULL_STR,wxART_NONE,wxITEM_CHECK);
    item->Check(CSL_FLAG_CHECK(*m_filterFlags, CSL_FILTER_FULL));
    item=&CslMenu::AddItem(*filter, MENU_SERVER_FILTER_EMPTY,
                           MENU_SERVER_FILTER_EMPTY_STR,wxART_NONE,wxITEM_CHECK);
    item->Check(CSL_FLAG_CHECK(*m_filterFlags, CSL_FILTER_EMPTY));
    item->Enable(!(CSL_FLAG_CHECK(*m_filterFlags, CSL_FILTER_NONEMPTY)));
    item=&CslMenu::AddItem(*filter, MENU_SERVER_FILTER_NONEMPTY,
                           MENU_SERVER_FILTER_NONEMPTY_STR,wxART_NONE,wxITEM_CHECK);
    item->Check(CSL_FLAG_CHECK(*m_filterFlags, CSL_FILTER_NONEMPTY));
    item->Enable(!CSL_FLAG_CHECK(*m_filterFlags, CSL_FILTER_EMPTY));
    item=&CslMenu::AddItem(*filter, MENU_SERVER_FILTER_MM2,
                           MENU_SERVER_FILTER_MM2_STR,wxART_NONE,wxITEM_CHECK);
    item->Check(CSL_FLAG_CHECK(*m_filterFlags, CSL_FILTER_MM2));
    item=&CslMenu::AddItem(*filter, MENU_SERVER_FILTER_MM3,
                           MENU_SERVER_FILTER_MM3_STR,wxART_NONE,wxITEM_CHECK);
    item->Check(CSL_FLAG_CHECK(*m_filterFlags, CSL_FILTER_MM3));

    if (m_id==CSL_LIST_MASTER)
    {
        filter->AppendSeparator();
        item=&CslMenu::AddItem(*filter, MENU_SERVER_FILTER_VER, MENU_SERVER_FILTER_VER_STR, wxART_NONE, wxITEM_CHECK);
        item->Enable(selected>0 || m_filterVersion>-1);
        item->Check(m_filterVersion>-1);
    }

    if (selected==1)
    {
        menu.AppendSeparator();
        CSL_MENU_CREATE_NOTIFY(menu,info)
        menu.AppendSeparator();
        CSL_MENU_CREATE_LOCATION(menu)
    }

    point=ScreenToClient(point);
    PopupMenu(&menu,point);
}

void CslListCtrlServer::OnMenu(wxCommandEvent& event)
{
    wxUint32 i=0;
    wxInt32 id=event.GetId();
    CslServerInfo *info=NULL;

    if (!m_selected.IsEmpty())
    {
        info=((CslListServerData*)m_selected.Item(0))->Info;

        CSL_MENU_EVENT_SKIP_CONNECT(id,info)
        CSL_MENU_EVENT_SKIP_EXTINFO(id,info)
        CSL_MENU_EVENT_SKIP_SRVMSG(id,info)
        CSL_MENU_EVENT_SKIP_LOCATION(id,new wxString(info->Host))
        CSL_MENU_EVENT_SKIP_NOTIFY(id,info)

        if (CSL_MENU_EVENT_IS_URICOPY(id))
        {
            wxArrayPtrVoid *servers=new wxArrayPtrVoid;

            for (i=0;i<m_selected.GetCount();i++)
            {
                info=((CslListServerData*)m_selected.Item(i))->Info;
                servers->Add((void*)info);
            }

            event.SetClientData((void*)servers);
            event.Skip();
            return;
        }
    }

    switch (id)
    {
        case MENU_ADD:
        {
            if (m_id==CSL_LIST_MASTER)
            {
                bool sort=false;
                for (i=0;i<m_selected.GetCount();i++)
                {
                    info=((CslListServerData*)m_selected.Item(i))->Info;
                    info->SetFavourite();
                    sort=true;
                    m_sibling->UpdateServer(info);
                    /*if (!info->IsDefault())
                    {
                        item.SetId(c);
                        DeleteItem(item);
                    }*/
                }
                if (sort)
                    m_sibling->ListSort();
                break;
            }
            if (m_id==CSL_LIST_FAVOURITE)
            {
                CslFrame *frame = (CslFrame*)::wxGetApp().GetTopWindow();
                CslDlgAddServer dlg(frame);

                info = dlg.InitDlg(CslServerInfo::Create(frame->GetSelectedGame()));

                if (dlg.ShowModal()==wxID_OK)
                {
                    info->SetFavourite();
                    UpdateServer(info);
                    ListSort();
                }
                else
                    CslServerInfo::Destroy(info);
            }
            break;
        }

        case MENU_REM:
        {
            RemoveServers();
            break;
        }

        case MENU_DEL:
        {
            DeleteServers();
            break;
        }

        case MENU_COPY:
        {
            wxString s;
            wxInt32 pos;
            wxUint16 port;
            wxListItem item;

            item.SetMask(wxLIST_MASK_TEXT);

            for (i=0; i<m_selected.GetCount(); i++)
            {
                info=((CslListServerData*)m_selected.Item(i))->Info;

                if ((pos=ListFindItemByServerInfo(info))==wxNOT_FOUND)
                    continue;

                item.SetId(pos);

                if ((port=info->GetGame().GetDefaultGamePort())!=info->GamePort)
                    port=info->GamePort;
                else
                    port=0;

                for (wxInt32 j=0; j<GetColumnCount(); j++)
                {
                    item.SetColumn(j);
                    GetItem(item);
                    const wxString& is=item.GetText();

                    if (!is.IsEmpty())
                    {
                        s<<is;
                        if (port)
                        {
                            GetColumn(j, item);
                            if (item.GetText()==_("Host"))
                            {
                                s<<wxT(":")<<port;
                                port=0;
                            }
                        }
                        s<<wxT("  ");
                    }
                }
                if (!s.IsEmpty())
                    s<<CSL_NEWLINE_WX;
            }

            if (s.IsEmpty())
                break;

            if (wxTheClipboard->Open())
            {
                wxTheClipboard->SetData(new wxTextDataObject(s));
                wxTheClipboard->Close();
            }
            break;
        }

        case MENU_SERVER_FILTER_OFF:
            i=CSL_FILTER_OFFLINE;
        case MENU_SERVER_FILTER_FULL:
            if (i==0)
                i=CSL_FILTER_FULL;
        case MENU_SERVER_FILTER_EMPTY:
            if (i==0)
                i=CSL_FILTER_EMPTY;
        case MENU_SERVER_FILTER_NONEMPTY:
            if (i==0)
                i=CSL_FILTER_NONEMPTY;
        case MENU_SERVER_FILTER_MM2:
            if (i==0)
                i=CSL_FILTER_MM2;
        case MENU_SERVER_FILTER_MM3:
            if (i==0)
                i=CSL_FILTER_MM3;
        case MENU_SERVER_FILTER_VER:
        {
            if (event.IsChecked())
            {
                if (i==0 && !m_selected.IsEmpty())
                    m_filterVersion=((CslListServerData*)m_selected.Item(0))->Info->Protocol;
                CSL_FLAG_SET(*m_filterFlags, i);
            }
            else
            {
                if (i==0)
                    m_filterVersion=-1;
                CSL_FLAG_UNSET(*m_filterFlags, i);
            }

            Rebuild();

            event.SetClientData((void*)(wxUIntPtr)m_id);
            event.Skip();

            break;
        }

        default:
            break;
    }
}

int wxCALLBACK CslListCtrlServer::ListSortCompareCallBack(long item1, long item2, long data)
{
    CslListServerData *data1=(CslListServerData*)item1;
    CslListServerData *data2=(CslListServerData*)item2;
    CslListSort *helper=(CslListSort*)data;

    bool high1=CSL_FLAG_CHECK(data1->HighLight, CSL_HIGHLIGHT_FOUND_SERVER|CSL_HIGHLIGHT_FOUND_PLAYER);
    bool high2=CSL_FLAG_CHECK(data2->HighLight, CSL_HIGHLIGHT_FOUND_SERVER|CSL_HIGHLIGHT_FOUND_PLAYER);

    if ((high1 || high2) && !(high1 && high2))
        return high1 ? -1:1;

    bool ping1Ok=CslEngine::PingOk(*data1->Info, CslGetSettings().UpdateInterval);
    bool ping2Ok=CslEngine::PingOk(*data2->Info, CslGetSettings().UpdateInterval);

    if (helper->Column!=COLUMN_HOST)
    {
        if (!ping1Ok && !ping2Ok)
            return 0;
        if (!ping1Ok)
            return 1;
        if (!ping2Ok)
            return -1;
    }

    wxInt32 ret=0;

#define LIST_SORT_PLAYER(_data1,_data2,_mode) \
    if (!(ret=helper->Cmp(_data1->Players,_data2->Players,_mode)) && _data1->Players>0) \
    { \
        ret=helper->Cmp(_data1->PlayersMax-_data1->Players, \
                        _data2->PlayersMax-_data2->Players, \
                        CslListSort::SORT_DSC); \
    }

    switch (helper->Column)
    {
        case COLUMN_HOST:
        {
            bool isip1=IsIPV4(data1->Host);
            bool isip2=IsIPV4(data2->Host);
            if (isip1 && !isip2)
                ret=helper->Mode==CslListSort::SORT_ASC ? -1 : 1;
            else if (!isip1 && isip2)
                ret=helper->Mode==CslListSort::SORT_ASC ? 1 : -1;
            else if (isip1 && isip2)
                ret=helper->Cmp(wxUINT32_SWAP_ON_LE(AtoN(data1->Host)),
                                wxUINT32_SWAP_ON_LE(AtoN(data2->Host)));
            break;
        }

        case COLUMN_NAME:
            ret=helper->Cmp(data1->Description,data2->Description);
            break;

        case COLUMN_PING:
            ret=helper->Cmp(data1->Ping,data2->Ping);
            break;

        case COLUMN_VERSION:
            ret=helper->Cmp(data1->Protocol,data2->Protocol);
            break;

        case COLUMN_MODE:
            ret=helper->Cmp(data1->GameMode,data2->GameMode);
            break;

        case COLUMN_MAP:
            ret=helper->Cmp(data1->Map,data2->Map);
            break;

        case COLUMN_TIME:
            ret=helper->Cmp(data1->TimeRemain,data2->TimeRemain);
            break;

        case COLUMN_PLAYER:
            LIST_SORT_PLAYER(data1,data2,helper->Mode)
            break;

        case COLUMN_MM:
            ret=helper->Cmp(data1->MM,data2->MM);
            break;

        default:
            break;
    }

    if (!ret)
    {
        if (helper->Column!=COLUMN_PLAYER)
            LIST_SORT_PLAYER(data1, data2, CslListSort::SORT_DSC);

        if (!ret && !(ret=helper->Cmp(data1->Info->ConnectedTimes,
                                      data2->Info->ConnectedTimes,
                                      CslListSort::SORT_DSC)))
            ret=helper->Cmp(data1->Info->Uptime,data2->Info->Uptime,
                            CslListSort::SORT_DSC);
    }

#undef LIST_SORT_PLAYER

    return ret;
}
