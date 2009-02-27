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

#include "CslListCtrlPlayerSearch.h"
#include "CslListCtrlPlayer.h"
#include "CslGameConnection.h"
#include "engine/CslTools.h"
#include "CslApp.h"
#include "CslGeoIP.h"
#include "CslFlags.h"
#include "CslMenu.h"


BEGIN_EVENT_TABLE(CslListCtrlPlayerSearch,CslListCtrl)
    EVT_SIZE(CslListCtrlPlayerSearch::OnSize)
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
    FlickerFree(false);
    ListInit();
}


CslListCtrlPlayerSearch::~CslListCtrlPlayerSearch()
{
    WX_CLEAR_ARRAY(m_entries);
}

void CslListCtrlPlayerSearch::ListInit()
{
    wxListItem item;

    CreateImageList();

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
        CslMenu::AddItem(&menu,MENU_ADD,MENU_SERVER_FAV_ADD_STR,wxART_ADD_BOOKMARK).Enable(!info->IsFavourite());
        menu.AppendSeparator();
        CSL_MENU_CREATE_URICOPY(menu)
        menu.AppendSeparator();
        CSL_MENU_CREATE_NOTIFY(menu,info)
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
    wxInt32 id=event.GetId();

    if (!(info=m_selected->Info))
        return;

    event.SetEventObject(this);

    CSL_MENU_EVENT_SKIP_CONNECT(id,info)
    CSL_MENU_EVENT_SKIP_EXTINFO(id,info)
    CSL_MENU_EVENT_SKIP_NOTIFY(id,info)
    CSL_MENU_EVENT_SKIP_SAVEIMAGE(id)

    if (CSL_MENU_EVENT_IS_URICOPY(id))
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
    DeleteAllItems();
    WX_CLEAR_ARRAY(m_entries);
}

void CslListCtrlPlayerSearch::AddResult(CslServerInfo *info,CslPlayerStatsData *player)
{
    wxListItem item;
    wxInt32 i=GetItemCount();

    item.SetId(i);
    item.SetMask(wxLIST_MASK_TEXT|wxLIST_MASK_DATA);
    InsertItem(item);
    SetItem(i,0,player->Name,GetCountryFlag(player->IP));
    SetItem(i,1,info->GetBestDescription(),info->GetGame().GetId()-1);

    m_entries.Add(new CslPlayerSearchEntry(info,player->Name));
    SetItemData(i,(long)m_entries.Last());
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

wxUint32 CslListCtrlPlayerSearch::GetCountryFlag(wxUint32 ip)
{
    wxInt32 i;
    const char *country;

    if (ip && (country=CslGeoIP::GetCountryCodeByIPnum(ip)))
    {
        for (i=sizeof(codes)/sizeof(codes[0])-1;i>=0;i--)
        {
            if (!strcasecmp(country,codes[i]))
                return ::wxGetApp().GetCslEngine()->GetGames().length()+i+1;
        }
    }

    return ::wxGetApp().GetCslEngine()->GetGames().length();
}

void CslListCtrlPlayerSearch::CreateImageList()
{
#ifdef __WXMSW__
    m_imgList.Create(20,16,true);

    vector<CslGame*>& games=::wxGetApp().GetCslEngine()->GetGames();
    loopv(games)
    {
        const char **icon=games[i]->GetIcon(16);
        wxBitmap bmpGame=icon ? (AdjustIconSize(icon,wxNullIcon,wxSize(20,16),wxPoint(4,0))):wxBitmap(20,16);
        m_imgList.Add(bmpGame);
    }

    m_imgList.Add(AdjustIconSize(unknown_xpm,wxNullIcon,wxSize(20,16),wxPoint(0,3)));
    wxInt32 i,c=sizeof(codes)/sizeof(codes[0])-1;
    for (i=0;i<c;i++)
        m_imgList.Add(AdjustIconSize(flags[i],wxNullIcon,wxSize(20,16),wxPoint(0,3)));
#else
    m_imgList.Create(18,16,true);

    vector<CslGame*>& games=::wxGetApp().GetCslEngine()->GetGames();
    loopv(games)
    {
        const char **icon=games[i]->GetIcon(16);
        wxBitmap bmpGame=icon ? (AdjustIconSize(icon,wxNullIcon,wxSize(18,16),wxPoint(1,0))):wxBitmap(18,16);
        m_imgList.Add(bmpGame);
    }

    m_imgList.Add(AdjustIconSize(unknown_xpm,wxNullIcon,wxSize(18,16),wxPoint(0,2)));
    wxInt32 i,c=sizeof(codes)/sizeof(codes[0])-1;
    for (i=0;i<c;i++)
        m_imgList.Add(AdjustIconSize(flags[i],wxNullIcon,wxSize(18,16),wxPoint(0,2)));
#endif

    SetImageList(&m_imgList,wxIMAGE_LIST_SMALL);
}
