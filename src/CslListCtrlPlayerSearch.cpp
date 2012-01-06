/***************************************************************************
 *   Copyright (C) 2007-2011 by Glen Masgai                                *
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
#include "CslGeoIP.h"
#include "CslMenu.h"
#include "CslSettings.h"
#include "CslGameConnection.h"
#include "CslListCtrlPlayer.h"
#include "CslListCtrlPlayerSearch.h"

enum
{
    COLUMN_PLAYER,
    COLUMN_SERVER
};

BEGIN_EVENT_TABLE(CslListCtrlPlayerSearch,CslListCtrl)
    EVT_SIZE(CslListCtrlPlayerSearch::OnSize)
    EVT_LIST_COL_CLICK(wxID_ANY, CslListCtrlPlayerSearch::OnColumnLeftClick)
    EVT_LIST_ITEM_ACTIVATED(wxID_ANY,CslListCtrlPlayerSearch::OnItem)
    EVT_LIST_ITEM_SELECTED(wxID_ANY,CslListCtrlPlayerSearch::OnItem)
    EVT_LIST_ITEM_DESELECTED(wxID_ANY,CslListCtrlPlayerSearch::OnItemDeselected)
    EVT_CONTEXT_MENU(CslListCtrlPlayerSearch::OnContextMenu)
    EVT_MENU(wxID_ANY,CslListCtrlPlayerSearch::OnMenu)
END_EVENT_TABLE()


CslListCtrlPlayerSearch::CslListCtrlPlayerSearch(wxWindow* parent,wxWindowID id,const wxPoint& pos,
        const wxSize& size,long style,
        const wxValidator& validator,const wxString& name) :
        CslListCtrl(parent,id,pos,size,style|wxLC_SINGLE_SEL,validator,name),
        m_selected(NULL)
{
    ListAddColumn(_("Player"), wxLIST_FORMAT_LEFT, 1.0f, true, true);
    ListAddColumn(_("Server"), wxLIST_FORMAT_LEFT, 1.3f, true, true);
}


CslListCtrlPlayerSearch::~CslListCtrlPlayerSearch()
{
    WX_CLEAR_ARRAY(m_entries);
}

void CslListCtrlPlayerSearch::OnSize(wxSizeEvent& event)
{
    //fixes flickering if scrollbar is shown
    wxWindowUpdateLocker lock(this);

    ListAdjustSize(event.GetSize());

    event.Skip();
}

void CslListCtrlPlayerSearch::OnColumnLeftClick(wxListEvent& WXUNUSED(event))
{
    //catch the event to prevent sorting
}

void CslListCtrlPlayerSearch::OnItem(wxListEvent& event)
{
    m_selected=(CslPlayerSearchEntry*)m_entries.Item(event.GetIndex());

    if (!m_selected->Info)
        return;

    event.SetClientData((void*)m_selected->Info);
    event.Skip();
}

void CslListCtrlPlayerSearch::OnItemDeselected(wxListEvent& event)
{
    m_selected=NULL;
    event.Skip();
}

void CslListCtrlPlayerSearch::OnContextMenu(wxContextMenuEvent& event)
{
    if (!GetItemCount())
        return;

    wxMenu menu;
    wxPoint point=event.GetPosition();

    if (m_selected && m_selected->Info)
    {
        CslServerInfo *info=m_selected->Info;

        CSL_MENU_CREATE_CONNECT(menu,info)
        CSL_MENU_CREATE_EXTINFO(menu,info,-1)
        menu.AppendSeparator();
        CSL_MENU_CREATE_SRVMSG(menu,info)
        menu.AppendSeparator();
        if (!info->IsFavourite())
        {
            CslMenu::AddItem(menu, MENU_ADD, MENU_SERVER_FAV_ADD_STR, wxART_ADD_BOOKMARK);
            menu.AppendSeparator();
        }
        CSL_MENU_CREATE_URICOPY(menu)
        menu.AppendSeparator();
        CSL_MENU_CREATE_NOTIFY(menu,info)
        menu.AppendSeparator();
        CSL_MENU_CREATE_LOCATION(menu)
        menu.AppendSeparator();
    }

    CSL_MENU_CREATE_SAVEIMAGE(menu)

    //from keyboard
    if (point==wxDefaultPosition)
        point=wxGetMousePosition();
    point=ScreenToClient(point);
    PopupMenu(&menu,point);
}

void CslListCtrlPlayerSearch::OnMenu(wxCommandEvent& event)
{
    CslServerInfo *info;

    if (!(info=m_selected->Info))
        return;

    wxInt32 id=event.GetId();

    event.SetEventObject(this);

    CSL_MENU_EVENT_SKIP_CONNECT(id,info)
    CSL_MENU_EVENT_SKIP_EXTINFO(id,info)
    CSL_MENU_EVENT_SKIP_SRVMSG(id,info)
    CSL_MENU_EVENT_SKIP_NOTIFY(id,info)

    if (CSL_MENU_EVENT_IS_LOCATION(id))
    {
        event.SetClientData((void*)new wxString(NtoA(m_selected->Player.IP)));
        event.Skip();
        return;
    }
    else if (CSL_MENU_EVENT_IS_URICOPY(id))
    {
        VoidPointerArray *servers=new VoidPointerArray;
        servers->Add((void*)info);
        event.SetClientData((void*)servers);
        event.Skip();
        return;
    }

    switch (id)
    {
        case MENU_ADD:
            event.SetClientData((void*)info);
            event.Skip();
            break;

        default:
            break;
    }
}

void CslListCtrlPlayerSearch::GetToolTipText(wxInt32 row,CslToolTipEvent& event)
{
    if (row<GetItemCount())
    {
        wxInt32 i;
        wxListItem item,column;

        column.SetMask(wxLIST_MASK_TEXT);
        item.SetMask(wxLIST_MASK_TEXT);
        item.SetId(row);

        for (i=0;i<GetColumnCount();i++)
        {
            item.SetColumn(i);
            GetItem(item);
            GetColumn(i,column);

            const wxString& s=item.GetText();

            if (!s.IsEmpty())
            {
                event.Text.Add(column.GetText());
                event.Text.Add(s);
            }
        }

        event.Title=_("Player search result");
    }
}

void CslListCtrlPlayerSearch::ListClear()
{
    wxWindowUpdateLocker lock(this);

    DeleteAllItems();
    WX_CLEAR_ARRAY(m_entries);
}

void CslListCtrlPlayerSearch::AddResult(CslServerInfo *info, CslPlayerStatsData *player, bool rebuild)
{
    wxInt32 i=GetItemCount();

    ListSetItem(i, COLUMN_PLAYER, player->Name, GetCountryFlag(player->IP));
    ListSetItem(i, COLUMN_SERVER, info->GetBestDescription(), GetGameImage(info->GetGame().GetFourCC()));

    if (!rebuild)
    m_entries.Add(new CslPlayerSearchEntry(info,*player));

    SetItemPtrData(i, (wxUIntPtr)m_entries.Item(i));
}

void CslListCtrlPlayerSearch::RemoveServer(CslServerInfo *info)
{
    for (wxUint32 i=0;i<m_entries.GetCount();i++)
    {
        if (m_entries.Item(i)->Info==info)
        {
            m_entries.Item(i)->Info=NULL;
            return;
        }
    }
}
