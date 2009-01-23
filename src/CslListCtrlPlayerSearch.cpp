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
    event.Skip();
}

void CslListCtrlPlayerSearch::OnContextMenu(wxContextMenuEvent& event)
{
}

void CslListCtrlPlayerSearch::OnMenu(wxCommandEvent& event)
{
}

void CslListCtrlPlayerSearch::GetToolTipText(wxInt32 row,wxString& title,wxArrayString& text)
{
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
