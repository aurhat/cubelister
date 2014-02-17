/***************************************************************************
 *   Copyright (C) 2007-2014 by Glen Masgai                                *
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
#include "CslGame.h"
#include "CslMenu.h"
#include "CslGeoIP.h"
#include "CslPanelCountryView.h"

enum
{
    COLUMN_NAME = 0,
    COLUMN_COUNT
};

BEGIN_EVENT_TABLE(CslPanelCountry,wxPanel)
    EVT_SIZE(CslPanelCountry::OnSize)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(CslListCtrlCountry,CslListCtrl)
    EVT_CONTEXT_MENU(CslListCtrlCountry::OnContextMenu)
END_EVENT_TABLE()


CslPanelCountry::CslPanelCountry(wxWindow* parent,long listStyle) :
        wxPanel(parent,wxID_ANY),
        m_mode(MODE_PLAYER_SINGLE)
{
    m_sizer=new wxFlexGridSizer(2,1,0,0);

    m_listCtrl=new CslListCtrlCountry(this,wxID_ANY,wxDefaultPosition,wxDefaultSize,listStyle);
    m_gauge=new wxGauge(this,wxID_ANY,0,wxDefaultPosition,wxSize(-1,16));
    m_gauge->Hide();

    m_sizer->Add(m_listCtrl,0,wxALL|wxEXPAND,0);
    m_sizer->Add(m_gauge,0,wxALL|wxEXPAND,2);

    m_sizer->AddGrowableRow(0);
    m_sizer->AddGrowableCol(0);

    SetSizer(m_sizer);
    m_sizer->Fit(this);
    m_sizer->Layout();
}

void CslPanelCountry::OnSize(wxSizeEvent& event)
{
#ifdef __WXMAC__
    wxSize size = event.GetSize();
    size.y -= m_gauge->GetBestSize().y+4;
    m_listCtrl->SetSize(size);
    //fixes flicker after resizing
#if !wxCHECK_VERSION(2, 9, 0) //TODO: possible replacement needed
    wxIdleEvent idle;
    wxTheApp->SendIdleEvents(this,idle);
#endif
#endif //__WXMAC__

    event.Skip();
}

void CslPanelCountry::Reset(wxUint32 mode,wxUint32 count)
{
    m_listCtrl->ListClear();

    if (m_mode==mode)
        return;

    if ((m_mode = mode)==MODE_PLAYER_MULTI)
    {
        m_gauge->Show();
        m_gauge->SetRange(count);
        m_gauge->SetValue(0);
#ifdef __WXMAC__
        wxSize size=GetSize();
        size.y-=m_gauge->GetBestSize().y+4;
        m_listCtrl->SetSize(size);
#endif
    }
    else
        m_gauge->Hide();

    m_sizer->Layout();
}

void CslPanelCountry::UpdateData(CslServerInfo *info)
{
    m_listCtrl->UpdateData(info);

    if (m_mode==MODE_PLAYER_MULTI)
        m_gauge->SetValue(m_gauge->GetValue()+1);
}


CslListCtrlCountry::CslListCtrlCountry(wxWindow* parent,wxWindowID id,const wxPoint& pos,
                                       const wxSize& size,long style,
                                       const wxValidator& validator,const wxString& name) :
        CslListCtrl(parent,id,pos,size,style,validator,name)
{
    wxListItem item;

    ListAddColumn(_("Country"), wxLIST_FORMAT_LEFT, 2.5f, true, true);
    ListAddColumn(_("Count"), wxLIST_FORMAT_LEFT, 1.0f, true, true);

    SetSortCallback(ListSortCompareFunc, CslListSort::SORT_DSC, COLUMN_COUNT);
}


CslListCtrlCountry::~CslListCtrlCountry()
{
    WX_CLEAR_ARRAY(m_entries);
}

void CslListCtrlCountry::OnContextMenu(wxContextMenuEvent& event)
{
    if (m_entries.IsEmpty())
        return;

    wxMenu menu;
    wxPoint point=event.GetPosition();

    CSL_MENU_CREATE_SAVEIMAGE(menu)

    //from keyboard
    if (point==wxDefaultPosition)
        point=wxGetMousePosition();
    point=ScreenToClient(point);
    PopupMenu(&menu,point);
}

void CslListCtrlCountry::ListClear()
{
    DeleteAllItems();
    WX_CLEAR_ARRAY(m_entries);
}

void CslListCtrlCountry::UpdateEntry(const wxString& country,wxInt32 img)
{
    wxInt32 c;
    wxListItem item;
    CslCountryEntry *entry=NULL;

    item.SetMask(wxLIST_MASK_TEXT|wxLIST_MASK_IMAGE|wxLIST_MASK_DATA);

    for (c=0;c<GetItemCount();c++)
    {
        item.SetId(c);
        GetItem(item);
        entry=(CslCountryEntry*)GetItemData(c);

        if (entry->Image==img)
        {
            entry->Count++;
            break;
        }
        else
            entry=NULL;
    }

    if (!entry)
    {
        item.SetId(c);
        InsertItem(item);
        entry=new CslCountryEntry(country.IsEmpty() ? wxString(_("Unknown")) : country, img);
        m_entries.Add(entry);
        SetItemPtrData(c, (wxUIntPtr)entry);
    }

    ListSetItem(c, COLUMN_NAME, entry->Country, img);
    ListSetItem(c, COLUMN_COUNT, wxString::Format(wxT("%d"), entry->Count));
}

void CslListCtrlCountry::UpdateData(CslServerInfo *info)
{
    //fixes flickering if scrollbar is shown
    wxWindowUpdateLocker lock(this);

    wxInt32 imgId;
    wxString country;

    if (((CslPanelCountry*)GetParent())->GetMode()==CslPanelCountry::MODE_SERVER)
    {
        if (!info->Pingable())
            return;

        wxUint32 ip = info->Address().GetIP();

        if (ip)
        {
            imgId=GetCountryFlag(::wxGetApp().GetCslEngine(), ip);
            country=CslGeoIP::GetCountryNameByIPnum(ip);
            UpdateEntry(country,imgId);
        }
    }
    else
    {
        CslPlayerStatsData *player;

        loopv(info->PlayerStats.m_stats)
        {
            if (!(player=info->PlayerStats.m_stats[i])->Ok)
                break;
            if (!(player->IP>>8))
                continue;

            imgId=GetCountryFlag(::wxGetApp().GetCslEngine(), player->IP);
            country=CslGeoIP::GetCountryNameByIPnum(player->IP);

            UpdateEntry(country,imgId);
        }
    }

    ListSort();
}

int wxCALLBACK CslListCtrlCountry::ListSortCompareFunc(long item1, long item2, long data)
{
    CslCountryEntry *data1=(CslCountryEntry*)item1;
    CslCountryEntry *data2=(CslCountryEntry*)item2;
    CslListSort *helper=(CslListSort*)data;

    wxInt32 mode=helper->Mode;

    if (helper->Column==COLUMN_COUNT)
    {
        wxInt32 ret=0;

        if ((ret=helper->Cmp(data1->Count,data2->Count)))
            return ret;
        else
            mode=CslListSort::SORT_ASC;
    }

    return helper->Cmp(data1->Country,data2->Country,mode);
}
