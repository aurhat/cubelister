/***************************************************************************
 *   Copyright (C) 2007 -2009 by Glen Masgai                               *
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

#include "CslListCtrl.h"
#include "CslSettings.h"
#include "CslGeoIP.h"
#include "CslFlags.h"
#include "engine/CslTools.h"
#include "img/sortasc_18_12.xpm"
#include "img/sortdsc_18_12.xpm"


BEGIN_EVENT_TABLE(CslListCtrl,wxListCtrl)
    #ifdef __WXMSW__
    EVT_ERASE_BACKGROUND(CslListCtrl::OnEraseBackground)
    #endif
    EVT_MOTION(CslListCtrl::OnMouseMove)
    EVT_LIST_ITEM_SELECTED(wxID_ANY,CslListCtrl::OnItem)
    EVT_LIST_ITEM_ACTIVATED(wxID_ANY,CslListCtrl::OnItem)
    CSL_EVT_TOOLTIP(wxID_ANY,CslListCtrl::OnToolTip)
END_EVENT_TABLE()


wxImageList CslListCtrl::ListImageList;

CslListCtrl::CslListCtrl(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size,
                         long style,const wxValidator& validator,const wxString& name) :
        wxListCtrl(parent,id,pos,size,style,validator,name),
        m_mouseLastMove(0),m_flickerFree(true)
{
    Connect(wxEVT_CONTEXT_MENU,wxContextMenuEventHandler(CslListCtrl::OnContextMenu),NULL,this);
}

CslListCtrl::~CslListCtrl()
{
    CslToolTip::ResetTip();
}

#ifdef __WXMSW__
void CslListCtrl::OnEraseBackground(wxEraseEvent& event)
{
    //to prevent flickering, erase only content *outside* of the actual items
    if (m_flickerFree && GetItemCount()>0)
    {
        long tItem,bItem;
        wxRect tRect,bRect;
        wxDC *dc=event.GetDC();
        wxSize imgSize=GetImageListSize();
        const wxRect& cRect=GetClientRect();

        tItem=GetTopItem();
        bItem=tItem+GetCountPerPage();

        if (bItem>=GetItemCount())
            bItem=GetItemCount()-1;

        GetItemRect(tItem,tRect,wxLIST_RECT_LABEL);
        GetItemRect(bItem,bRect,wxLIST_RECT_BOUNDS);

        //set the new clipping region and do erasing
        wxRegion region(cRect);
        region.Subtract(wxRect(tRect.GetLeftTop(),bRect.GetBottomRight()));

        if (imgSize!=wxDefaultSize)
        {
            GetItemRect(0,bRect,wxLIST_RECT_ICON);
            bRect.height-=3;
            wxRegion imgRegion(imgSize);
            imgRegion.Offset(bRect.x,bRect.y+1);
            region.Xor(imgRegion);

            for (wxInt32 i=1;i<GetItemCount() && i<=bItem;i++)
            {
                imgRegion.Offset(0,bRect.height+3);
                region.Xor(imgRegion);
            }
        }

        dc->DestroyClippingRegion();
        dc->SetClippingRegion(region);
#if 0
        static int c=0;
        wxBitmap bmp=region.ConvertToBitmap();
        if (bmp.Ok()) bmp.SaveFile(wxString::Format("%-2.2d.bmp",c++),wxBITMAP_TYPE_BMP);
#endif
        //do erasing
        dc->SetBackground(wxBrush(GetBackgroundColour(),wxSOLID));
        dc->Clear();

        //restore old clipping region
        dc->DestroyClippingRegion();
        dc->SetClippingRegion(cRect);
    }
    else
        event.Skip();
}
#endif

void CslListCtrl::OnMouseMove(wxMouseEvent& event)
{
    event.Skip();

    if (!g_cslSettings->tooltipDelay)
        return;

    wxUint32 ticks=GetTicks();

    if (ticks-m_mouseLastMove<CSL_TOOLTIP_DELAY_STEP)
        return;

    m_mouseLastMove=ticks;

    CslToolTip::InitTip(this);
}

void CslListCtrl::OnItem(wxListEvent& event)
{
    CslToolTip::ResetTip();
    event.Skip();
}

void CslListCtrl::OnContextMenu(wxContextMenuEvent& event)
{
    CslToolTip::ResetTip();
    event.Skip();
}

void CslListCtrl::OnToolTip(CslToolTipEvent& event)
{
    wxRect rect;
    wxListItem item;
    wxInt32 i,offset=0;
#ifdef __WXGTK__
    bool first=true;
#endif
    wxPoint spos=wxGetMousePosition();
    wxPoint wpos=ScreenToClient(spos);

    if (wpos!=wxDefaultPosition)
    {
        for (i=GetTopItem();i<GetItemCount();i++)
        {
            item.SetId(i);
            GetItemRect(item,rect,wxLIST_RECT_BOUNDS);

#ifdef __WXGTK__
            if (first)
            {
                offset=rect.y;
                first=false;
            }
#endif
            rect.y-=offset;

            if (!rect.Contains(wpos))
                continue;

            wxString title;
            wxArrayString text;

            GetToolTipText(i,event);

            if (!event.Text.IsEmpty())
            {
                event.Pos=spos;
                return;
            }
        }
    }

    event.Skip();
}

wxUint32 CslListCtrl::GetCountryFlag(wxUint32 ip)
{
    wxInt32 i;
    const char *country;

    if (ip && (country=CslGeoIP::GetCountryCodeByIPnum(ip)))
    {
        for (i=sizeof(codes)/sizeof(codes[0])-1;i>=0;i--)
        {
            if (!strcasecmp(country,codes[i]))
                return CSL_LIST_IMG_UNKNOWN+i+1;
        }
    }

    return CSL_LIST_IMG_UNKNOWN;
}

void CslListCtrl::CreateCountryFlagImageList()
{
#ifdef __WXMSW__
    ListImageList.Create(20,14,true);
    ListImageList.Add(AdjustIconSize(sortasc_18_12_xpm,wxNullIcon,wxSize(20,14),wxPoint(0,0)));
    ListImageList.Add(AdjustIconSize(sortdsc_18_12_xpm,wxNullIcon,wxSize(20,14),wxPoint(0,0)));
    ListImageList.Add(AdjustIconSize(unknown_xpm,wxNullIcon,wxSize(20,14),wxPoint(0,2)));

    wxInt32 i,c=sizeof(codes)/sizeof(codes[0])-1;
    for (i=0;i<c;i++)
        ListImageList.Add(AdjustIconSize(flags[i],wxNullIcon,wxSize(20,14),wxPoint(0,2)));
#else
    ListImageList.Create(18,12,true);
    ListImageList.Add(wxBitmap(sortasc_18_12_xpm));
    ListImageList.Add(wxBitmap(sortdsc_18_12_xpm));
    ListImageList.Add(wxBitmap(unknown_xpm));

    wxInt32 i,c=sizeof(codes)/sizeof(codes[0])-1;
    for (i=0;i<c;i++)
        ListImageList.Add(wxBitmap(flags[i]));
#endif
}
