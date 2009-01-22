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

#include "CslListCtrlPlayerSearch.h"
#include "engine/CslTools.h"


BEGIN_EVENT_TABLE(CslListCtrlPlayerSearch,CslListCtrl)
    EVT_SIZE(CslListCtrlPlayerSearch::OnSize)
    EVT_LIST_ITEM_ACTIVATED(wxID_ANY,CslListCtrlPlayerSearch::OnItemActivated)
    EVT_CONTEXT_MENU(CslListCtrlPlayerSearch::OnContextMenu)
    EVT_MENU(wxID_ANY,CslListCtrlPlayerSearch::OnMenu)
END_EVENT_TABLE()


CslListCtrlPlayerSearch::CslListCtrlPlayerSearch(wxWindow* parent,wxWindowID id,const wxPoint& pos,
        const wxSize& size,long style,
        const wxValidator& validator,const wxString& name) :
        CslListCtrl(parent,id,pos,size,style,validator,name)
{
    FlickerFree(false);
    ListInit();
}


CslListCtrlPlayerSearch::~CslListCtrlPlayerSearch()
{
}

void CslListCtrlPlayerSearch::ListInit()
{
    wxListItem item;

    SetImageList(&ListImageList,wxIMAGE_LIST_SMALL);

    item.SetMask(wxLIST_MASK_TEXT|wxLIST_MASK_FORMAT);
    item.SetImage(-1);

    item.SetAlign(wxLIST_FORMAT_LEFT);

    item.SetText(_("Player"));
    InsertColumn(0,item);
    SetColumn(0,item);

    item.SetText(_("Server"));
    InsertColumn(1,item);
    SetColumn(1,item);
}

void CslListCtrlPlayerSearch::ListAdjustSize(const wxSize& size)
{
    wxInt32 w;

    if ((w=size==wxDefaultSize ? GetClientSize().x-8 : size.x-8)<0)
        return;

    SetColumnWidth(0,(wxInt32)(w*0.44f));
    SetColumnWidth(1,(wxInt32)(w*0.55f));
}

void CslListCtrlPlayerSearch::OnSize(wxSizeEvent& event)
{
    event.Skip();

    Freeze();
    ListAdjustSize(event.GetSize());
    Thaw();
}

void CslListCtrlPlayerSearch::OnItemActivated(wxListEvent& event)
{
    /*if (!m_info)
        return;

    CslConnectionState::CreateConnectState(m_info);*/

    event.Skip();
}

void CslListCtrlPlayerSearch::OnContextMenu(wxContextMenuEvent& event)
{
    /*if (!m_info)
        return;

    wxMenu menu;
    wxMenuItem *item;
    wxPoint point=event.GetPosition();

    //from keyboard
    if (point.x==-1 && point.y==-1)
        point=wxGetMousePosition();

    CslMenu::AddItem(&menu,MENU_SERVER_CONNECT,MENU_SERVER_CONN_STR,wxART_CONNECT);
    if (CSL_CAP_CONNECT_PASS(m_info->GetGame().GetCapabilities()))
        CslMenu::AddItem(&menu,MENU_SERVER_CONNECT_PW,MENU_SERVER_CONN_PW_STR,wxART_CONNECT_PW);

    if (m_info)
    {
        menu.AppendSeparator();

        wxMenu *ext=new wxMenu();
        item=menu.AppendSubMenu(ext,_("Extended information"));
        item->SetBitmap(GET_ART_MENU(wxART_ABOUT));
        if (m_info->ExtInfoStatus!=CSL_EXT_STATUS_OK || !CslEngine::PingOk(*m_info,g_cslSettings->updateInterval))
            item->Enable(false);
        else
        {
            if (m_view!=CSL_LIST_PLAYER_DEFAULT_SIZE_DLG)
            {
                CslMenu::AddItem(ext,MENU_SERVER_EXTENDED_FULL,_("Full"),wxART_ABOUT);
                ext->AppendSeparator();
            }
            if (m_view!=CSL_LISTPLAYER_MICRO_SIZE)
                CslMenu::AddItem(ext,MENU_SERVER_EXTENDED_MICRO,_("Micro"),wxART_EXTINFO_MICRO);
            if (m_view!=CSL_LISTPLAYER_MINI_SIZE)
                CslMenu::AddItem(ext,MENU_SERVER_EXTENDED_MINI,_("Mini"),wxART_EXTINFO_MINI);
            if (m_view!=CSL_LIST_PLAYER_DEFAULT_SIZE)
                CslMenu::AddItem(ext,MENU_SERVER_EXTENDED_DEFAULT,_("Default"),wxART_EXTINFO_DEFAULT);
        }
    }

    point=ScreenToClient(point);
    PopupMenu(&menu,point);*/
}

void CslListCtrlPlayerSearch::OnMenu(wxCommandEvent& event)
{
    /*switch (event.GetId())
    {

        case MENU_SERVER_CONNECT:
            CslConnectionState::CreateConnectState(m_info);
            event.Skip();
            break;

        case MENU_SERVER_CONNECT_PW:
        {
            wxInt32 i=CSL_CAP_CONNECT_ADMIN_PASS(m_info->GetGame().GetCapabilities());

            CslConnectPassInfo pi(m_info->Password,m_info->PasswordAdmin,i!=0);

            if (CslDlgConnectPass(this,&pi).ShowModal()==wxID_OK)
            {
                m_info->Password=pi.Password;
                m_info->PasswordAdmin=pi.AdminPassword;
                i=pi.Admin ? CslServerInfo::CSL_CONNECT_ADMIN_PASS:CslServerInfo::CSL_CONNECT_PASS;
                CslConnectionState::CreateConnectState(m_info,i);
                event.Skip();
            }
            break;
        }
        case MENU_SERVER_EXTENDED_FULL:
        case MENU_SERVER_EXTENDED_MICRO:
        case MENU_SERVER_EXTENDED_MINI:
        case MENU_SERVER_EXTENDED_DEFAULT:
            event.SetClientData(m_info);
            event.Skip();
            break;

        default:
            break;
    }*/
}

void CslListCtrlPlayerSearch::GetToolTipText(wxInt32 row,wxString& title,wxArrayString& text)
{
    /*if (row<GetItemCount())
    {
        wxInt32 i;
        const char *c;
        wxListItem item,column;

        item.SetId(row);

        for (i=0;i<GetColumnCount();i++)
        {
            item.SetColumn(i);
            GetItem(item);
            GetColumn(i,column);

            const wxString& s=item.GetText();

            if (!s.IsEmpty())
            {
                text.Add(column.GetText());
                text.Add(s);
            }
        }

        CslPlayerStatsData *data=(CslPlayerStatsData*)GetItemData(item);

        c=CslGeoIP::GetCountryNameByIPnum(data->IP);
        text.Add(_("Country"));
        text.Add(c ? (A2U(c)).c_str() : CslGeoIP::IsOk() ?
                 _("Unknown") : _("GeoIP database not found"));

        text.Add(wxT("ID / IP"));
        text.Add(wxString::Format(wxT("%d / %d.%d.%d.x"),data->ID,
                                  data->IP>>24,data->IP>>16&0xff,data->IP>>8&0xff));

        title=_("Player information");
    }*/
}

void CslListCtrlPlayerSearch::ListClear()
{
    DeleteAllItems();
    m_servers.Empty();;
}

void CslListCtrlPlayerSearch::AddResult(CslServerInfo *info,CslPlayerStatsData *player)
{
    wxListItem item;
    wxInt32 i=GetItemCount();

    item.SetId(i);
    item.SetMask(wxLIST_MASK_TEXT|wxLIST_MASK_DATA);
    InsertItem(item);
    SetItem(i,0,player->Name,GetCountryFlag(player->IP));
    SetItem(i,1,info->GetBestDescription(),GetCountryFlag(IP2Int(info->Addr.IPAddress())));
    SetItemData(i,(long)info);

    for (i=0;i<(wxInt32)m_servers.GetCount();i++)
    {
        if (m_servers.Item(i)==info)
            return;
    }

    m_servers.Add(info);
}
