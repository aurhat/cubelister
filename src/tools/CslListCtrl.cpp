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
#include "CslSettings.h"
#include "CslGeoIP.h"
#include "CslMenu.h"
#include "CslListCtrl.h"

#include "img/ext_green_8.xpm"
#include "img/red_list_16.xpm"
#include "img/green_list_16.xpm"
#include "img/yellow_list_16.xpm"
#include "img/grey_list_16.xpm"
#include "img/red_ext_list_16.xpm"
#include "img/green_ext_list_16.xpm"
#include "img/yellow_ext_list_16.xpm"
#include "img/info_16.xpm"
#include "img/sortasc_16.xpm"
#include "img/sortdsc_16.xpm"
#include "img/sortasclight_16.xpm"
#include "img/sortdsclight_16.xpm"
#include "CslFlags.h"

// since wx>2.8.4 the list control items are getting deselected on wxGTK
// this happens too on wxMAC (maybe because of the generic list control?)
#if defined(__WXGTK__) || defined(__WXMAC__) || defined(__WXUNIVERSAL__)
#ifndef CSL_USE_WX_LIST_DESELECT_WORKAROUND
#if wxVERSION_NUMBER > 2804 || defined(__WXMAC__)
#define CSL_USE_WX_LIST_DESELECT_WORKAROUND
#endif
#endif
#endif

DEFINE_EVENT_TYPE(wxCSL_EVT_COMMAND_LIST_ALL_ITEMS_SELECTED)

BEGIN_EVENT_TABLE(CslListCtrl, wxListCtrl)
    #ifdef __WXMSW__
    EVT_ERASE_BACKGROUND(CslListCtrl::OnEraseBackground)
    #endif
    EVT_MOTION(CslListCtrl::OnMouseMove)
    EVT_KEY_DOWN(CslListCtrl::OnKeyDown)
    EVT_LIST_COL_CLICK(wxID_ANY, CslListCtrl::OnColumnLeftClick)
    EVT_LIST_COL_RIGHT_CLICK(wxID_ANY, CslListCtrl::OnColumnRightClick)
    CSL_EVT_TOOLTIP(wxID_ANY, CslListCtrl::OnToolTip)
END_EVENT_TABLE()


wxImageList CslListCtrl::ListImageList;

IMPLEMENT_DYNAMIC_CLASS(CslListCtrl, wxListCtrl)

CslListCtrl::CslListCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size,
                         long style, const wxValidator& validator, const wxString& name) :
        wxListCtrl(parent, id, pos, size, style, validator, name),
        m_processSelectEvent(1), m_mouseLastMove(0)
{
    CreateImageList();
    SetImageList(&ListImageList, wxIMAGE_LIST_SMALL);

    // don't use the static event tables, so the event is processed in the base class first
    Connect(wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler(CslListCtrl::OnItem), NULL, this);
    Connect(wxEVT_COMMAND_LIST_ITEM_DESELECTED, wxListEventHandler(CslListCtrl::OnItem), NULL, this);
    Connect(wxEVT_COMMAND_LIST_ITEM_ACTIVATED, wxListEventHandler(CslListCtrl::OnItem), NULL, this);
    Connect(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(CslListCtrl::OnMenu), NULL, this);
    Connect(wxEVT_CONTEXT_MENU, wxContextMenuEventHandler(CslListCtrl::OnContextMenu), NULL, this);
}

CslListCtrl::~CslListCtrl()
{
    CslToolTip::ResetTip();
}

#ifdef __WXMSW__
void CslListCtrl::OnEraseBackground(wxEraseEvent& event)
{
    // to prevent flickering, erase only content *outside* of the actual items

    if (GetItemCount()>0)
    {
        long tItem, bItem;
        wxRect tRect, bRect;
        wxDC *dc=event.GetDC();
        wxSize imgSize=GetImageListSize();
        const wxRect& cRect=GetClientRect();

        tItem=GetTopItem();
        bItem=tItem+GetCountPerPage();

        if (bItem>=GetItemCount())
            bItem=GetItemCount()-1;

        GetItemRect(tItem, tRect, wxLIST_RECT_LABEL);
        GetItemRect(bItem, bRect, wxLIST_RECT_BOUNDS);

        //set the new clipping region and do erasing
        wxRegion region(cRect);
        region.Subtract(wxRect(tRect.GetLeftTop(), bRect.GetBottomRight()));

        if (imgSize!=wxDefaultSize)
        {
            GetItemRect(0, bRect, wxLIST_RECT_ICON);
            bRect.height-=3;
            wxRegion imgRegion(imgSize);
            imgRegion.Offset(bRect.x, bRect.y+1);
            region.Xor(imgRegion);

            for (wxInt32 i=1; i<GetItemCount() && i<=bItem; i++)
            {
                imgRegion.Offset(0, bRect.height+3);
                region.Xor(imgRegion);
            }
        }

        dc->DestroyClippingRegion();
        dc->SetClippingRegion(region);
#if 0
        static int c=0;
        wxBitmap bmp=region.ConvertToBitmap();
        if (bmp.Ok()) bmp.SaveFile(wxString::Format("%-2.2d.bmp", c++), wxBITMAP_TYPE_BMP);
#endif
        //do erasing
        dc->SetBackground(wxBrush(GetBackgroundColour(), wxSOLID));
        dc->Clear();

        //restore old clipping region
        dc->DestroyClippingRegion();
        dc->SetClippingRegion(cRect);
    }
    else
        event.Skip();
}
#endif //__WXMSW__

void CslListCtrl::OnMouseMove(wxMouseEvent& event)
{
    event.Skip();

    if (!CslGetSettings().TooltipDelay)
        return;

    wxUint32 ticks=GetTicks();

    if (ticks-m_mouseLastMove<CSL_TOOLTIP_DELAY_STEP)
        return;

    m_mouseLastMove=ticks;

    CslToolTip::InitTip(this, CslGetSettings().TooltipDelay);
}

void CslListCtrl::OnMenu(wxCommandEvent& event)
{
    wxInt32 id=event.GetId();

    if (id==MENU_SAVEIMAGE)
        CreateScreenShot();
    else if (CSL_MENU_EVENT_IS_COLUMN(id))
    {
        id-=MENU_LISTCTRL_COLUMN;

        ListToggleColumn(id);

        if (m_columns.IsEnabled(id))
            OnListUpdate();
    }

    event.Skip();
}

void CslListCtrl::OnContextMenu(wxContextMenuEvent& event)
{
    CslToolTip::ResetTip();
    event.Skip();
}

void CslListCtrl::OnKeyDown(wxKeyEvent &event)
{
    if (!(GetWindowStyle()&wxLC_SINGLE_SEL) && event.ControlDown() && event.GetKeyCode()=='A')
    {
        wxWindowUpdateLocker lock(this);

        for (wxInt32 i=0; i<GetItemCount(); i++)
            SetItemState(i, 0, wxLIST_STATE_SELECTED);

        m_processSelectEvent=-1;

        for (wxInt32 i=0; i<GetItemCount(); i++)
            SetItemState(i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);

        m_processSelectEvent=1;
    }

    event.Skip();
}

void CslListCtrl::OnColumnLeftClick(wxListEvent& event)
{
    ListSort(event.GetColumn());
    event.Skip();
}

void CslListCtrl::OnColumnRightClick(wxListEvent& event)
{
    CslToolTip::ResetTip();

    wxInt32 count=m_columns.GetCount();

    if (!count || count<=m_columns.GetLockedCount())
        return;

    wxMenu menu;
    wxMenuItem *item;
    const vector<CslListColumns::Column>& columns=m_columns.GetColumns();

    count=m_columns.GetCount(true);

    loopv(columns)
    {
        if (columns[i].Locked)
            continue;

        item=new wxMenuItem(&menu, MENU_LISTCTRL_COLUMN+i, columns[i].Name, wxART_NONE, wxITEM_CHECK);
        menu.Append(item);
        item->Check(m_columns.IsEnabled(i));

        if (count==1 && m_columns.IsEnabled(i))
            menu.Enable(MENU_LISTCTRL_COLUMN+i, false);
    }

    wxPoint point=wxDefaultPosition;
    //from keyboard
    if (point==wxDefaultPosition)
        point=wxGetMousePosition();
    point=ScreenToClient(point);

    PopupMenu(&menu, point);
}

void CslListCtrl::OnItem(wxListEvent& event)
{
    //if (event.GetEventType()!=wxEVT_COMMAND_LIST_ITEM_DESELECTED)
    //    CslToolTip::ResetTip();

    if (!m_processSelectEvent)
        return;
    //all items selected via CTRL+A
    if (m_processSelectEvent<0)
        event.SetEventType(wxCSL_EVT_COMMAND_LIST_ALL_ITEMS_SELECTED);

    event.Skip();
}

void CslListCtrl::OnToolTip(CslToolTipEvent& event)
{
    wxRect rect;
    wxListItem item;
    wxInt32 i, offset=0;
#ifdef __WXGTK__
    bool first=true;
#endif
    const wxSize& size=GetClientSize();
    const wxPoint& pos=ScreenToClient(event.Pos);

    if (pos.x>=0 && pos.y>=0 && pos.x<=size.x && pos.y<=size.y)
    {
        for (i=GetTopItem(); i<GetItemCount(); i++)
        {
            item.SetId(i);
            GetItemRect(item, rect, wxLIST_RECT_BOUNDS);

#ifdef __WXGTK__
            if (first)
            {
                offset=rect.y;
                first=false;
            }
#endif
            rect.y-=offset;

            if (!rect.Contains(pos))
                continue;

            wxString title;
            wxArrayString text;

            GetToolTipText(i, event);

            if (!event.Text.IsEmpty())
                return;
        }
    }

    event.Skip();
}

void CslListCtrl::ListAddColumn(const wxString& name, wxListColumnFormat format,
                                   float weight, bool enabled, bool locked)
{
    m_columns.AddColumn(name, format, weight, enabled, locked);

    if (enabled)
    {
        wxListItem item;
        wxInt32 rcolumn=m_columns.GetColumnId(m_columns.GetCount()-1, false);

        item.SetMask(wxLIST_MASK_TEXT|wxLIST_MASK_IMAGE|wxLIST_MASK_FORMAT);
        item.SetAlign(format);
        item.SetText(name);

        InsertColumn(rcolumn, item);
        SetColumn(rcolumn, item);
    }
}

void CslListCtrl::ListDeleteColumn(wxInt32 column)
{
    m_columns.RemoveColumn(column);
    DeleteColumn(m_columns.GetColumnId(column, false));
}

wxUint32 CslListCtrl::ListToggleColumn(wxInt32 column)
{
    if (m_columns.IsEnabled(column))
        DeleteColumn(m_columns.GetColumnId(column, false));
    else
    {
        wxListItem item;
        wxInt32 colrel=m_columns.GetColumnId(column, false);
        CslListColumns::Column *c=m_columns.GetColumn(column);

        item.SetMask(wxLIST_MASK_FORMAT);
        item.SetImage(-1);
        item.SetAlign(c->Format);
        item.SetText(c->Name);

        InsertColumn(colrel, item);
        SetColumn(colrel, item);
    }

    m_columns.ToggleColumn(column);

    ListAdjustSize();

    return m_columns.GetEnabledColumns();
}

bool CslListCtrl::ListSetItem(wxInt32 row, wxInt32 column, const wxString& text, wxInt32 image)
{
    if (!m_columns.IsEnabled(column))
        return false;

    wxListItem item;
    wxInt32 l=GetItemCount();

    column=m_columns.GetColumnId(column, false);

    wxWindowUpdateLocker lock(this);

    while (l++<=row)
        InsertItem(l, -1);

    item.SetId(row);

    SetItem(row, column, text, image);

    return true;
}

bool CslListCtrl::ListSetColumn(wxInt32 column, const wxString& text)
{
    CslListColumns::Column *c=m_columns.GetColumn(column);

    if (!c)
        return false;

    c->Name=text;

    if (!m_columns.IsEnabled(column))
        return false;

    wxListItem item;

    column=m_columns.GetColumnId(column, false);
    GetColumn(column, item);

    wxWindowUpdateLocker lock(this);

    item.SetMask(wxLIST_MASK_TEXT);
    SetColumn(column, item);

    return true;
}

wxInt32 CslListCtrl::ListFindItem(void *data)
{
    wxListItem item;
    wxInt32 i, j=GetItemCount();

    for (i=0; i<j; i++)
    {
        item.SetId(i);

        if (ListFindItemCompare(data, (void*)GetItemData(item)))
            return i;
    }

    return wxNOT_FOUND;
}

void CslListCtrl::ListSort(wxInt32 column)
{
    if (!m_columns.GetCount())
        return;

    wxListItem item;

    if (column>=0)
    {
        item.SetMask(wxLIST_MASK_IMAGE);

        wxInt32 id=m_columns.GetColumnId(column);

        if (m_sortHelper.Column!=id)
        {
            if (m_columns.IsEnabled(m_sortHelper.Column))
            {
                item.SetImage(-1);
                SetColumn(m_columns.GetColumnId(m_sortHelper.Column, false), item);
            }

            m_sortHelper.Column=id;
        }

        GetColumn(column, item);

        if (item.GetImage()==-1 || item.GetImage()==CSL_LIST_IMG_SORT_DSC)
        {
            item.SetImage(CSL_LIST_IMG_SORT_ASC);
            m_sortHelper.Mode=CslListSort::SORT_ASC;
        }
        else
        {
            item.SetImage(CSL_LIST_IMG_SORT_DSC);
            m_sortHelper.Mode=CslListSort::SORT_DSC;
        }

        SetColumn(column, item);
    }

    if (!GetItemCount())
        return;

    m_processSelectEvent=0;

    OnListSort();

#ifdef CSL_USE_WX_LIST_DESELECT_WORKAROUND
    wxInt32 i, j, v;

    for (i=0, j=(wxInt32)m_selected.GetCount(); i<j; i++)
    {
        if ((v=ListFindItem(m_selected.Item(i)))==wxNOT_FOUND)
            continue;

        SetItemState(v, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
    }
#endif // CSL_USE_WX_LIST_DESELECT_WORKAROUND

    m_processSelectEvent=1;

#ifndef __WXMSW__
#if !wxCHECK_VERSION(2, 9, 0) //TODO: possible replacement needed
    //removes flicker on autosort for wxGTK and wxMAC
    wxIdleEvent idle;
    wxTheApp->SendIdleEvents(this, idle);
#endif
#endif
}

void CslListCtrl::CreateScreenShot()
{
    wxString file;
    wxBitmap bitmap;
    wxWindow *window=GetScreenShotWindow();

    if (!BitmapFromWindow(window, bitmap))
    {
        wxMessageBox(_("Error creating the screenshot!"),
                     _("Error"), wxOK|wxICON_ERROR, window);
        return;
    }

    file=GetScreenShotFileName();

    wxFileDialog dlg(window, _("Save screenshot"), wxEmptyString, file, _("PNG files (*.png)|*.png"),
#if wxCHECK_VERSION(2, 9, 0)
                     wxFD_SAVE|wxFD_OVERWRITE_PROMPT
#else
                     wxSAVE|wxOVERWRITE_PROMPT
#endif
                    );
    // wxGTK: hmm, doesn't work in the ctor?!
    if (wxDirExists(CslGetSettings().ScreenOutputPath))
        dlg.SetPath(CslGetSettings().ScreenOutputPath+PATHDIV+file);
    if (dlg.ShowModal()!=wxID_OK)
        return;

    file=dlg.GetPath();
    CslGetSettings().ScreenOutputPath=::wxPathOnly(file);

    bitmap.SaveFile(file, wxBITMAP_TYPE_PNG);
}

wxString CslListCtrl::GetScreenShotFileName()
{
    return wxDateTime::Now().Format(wxT("%Y%m%d_%H%M%S"))+wxT(".png");
}

wxInt32 CslListCtrl::GetGameImage(wxUint32 fourcc, wxInt32 offset)
{
    CslGames& games=::wxGetApp().GetCslEngine()->GetGames();

    loopv(games)
    {
        if (games[i]->GetFourCC()==fourcc)
            return CSL_LIST_IMG_GAMES_START+offset+i*2;
    }

    return -1;
}

wxInt32 CslListCtrl::GetCountryFlag(wxUint32 ip)
{
    wxInt32 i;
    const char *country;
    static wxInt32 offset=0;

    if (!offset)
        offset=CSL_LIST_IMG_GAMES_START+::wxGetApp().GetCslEngine()->GetGames().length()*2;

    if (ip && ip!=(wxUint32)-1)
    {
        if (!(country=CslGeoIP::GetCountryCodeByIPnum(ip)))
        {
            if (IsLocalIP(ip))
                return offset; // local_xpm
        }
        else
        {
            for (i=sizeof(country_codes)/sizeof(country_codes[0])-1; i>=0; i--)
            {
                if (!strcasecmp(country, country_codes[i]))
                    return offset+i+2;
            }
        }
    }

    return offset+1; // unknown_xpm
}

void CslListCtrl::CreateImageList()
{
    if (ListImageList.GetImageCount())
        return;

    wxInt32 i, l;

    wxSize size(18, 14);
    wxPoint offset(0, 0);

#ifdef __WXMSW__
    size.x=20;
#endif
    ListImageList.Create(size.x, size.y, true);

    ListImageList.Add(AdjustBitmapSize(green_list_16_xpm, size, offset));
    ListImageList.Add(AdjustBitmapSize(yellow_list_16_xpm, size, offset));
    ListImageList.Add(AdjustBitmapSize(red_list_16_xpm, size, offset));
    ListImageList.Add(AdjustBitmapSize(grey_list_16_xpm, size, offset));
    ListImageList.Add(AdjustBitmapSize(green_ext_list_16_xpm, size, offset));
    ListImageList.Add(AdjustBitmapSize(yellow_ext_list_16_xpm, size, offset));
    ListImageList.Add(AdjustBitmapSize(red_ext_list_16_xpm, size, offset));
    ListImageList.Add(AdjustBitmapSize(info_16_xpm, size, offset));
    offset.y=2;
    ListImageList.Add(AdjustBitmapSize(sortasc_16_xpm, size, offset));
    ListImageList.Add(AdjustBitmapSize(sortdsc_16_xpm, size, offset));
    ListImageList.Add(AdjustBitmapSize(sortasclight_16_xpm, size, offset));
    ListImageList.Add(AdjustBitmapSize(sortdsclight_16_xpm, size, offset));

    const CslGames& games=::wxGetApp().GetCslEngine()->GetGames();
    const wxImage imgExt=wxBitmap(ext_green_8_xpm).ConvertToImage();

    offset.y=0;

    loopv(games)
    {
        const wxBitmap& icon=games[i]->GetIcon(16);
        wxBitmap bmpGame=icon.IsOk() ? AdjustBitmapSize(icon, size, offset) : wxBitmap(size.x, size.y);

        ListImageList.Add(bmpGame);
        wxImage img=bmpGame.ConvertToImage();
        ListImageList.Add(wxBitmap(OverlayImage(img, imgExt, size.x-8, size.y-8)));
    }

    ListImageList.Add(AdjustBitmapSize(local_xpm, size, offset));
    ListImageList.Add(AdjustBitmapSize(unknown_xpm, size, offset));

    offset.y=1;

    for (i=0, l=sizeof(country_codes)/sizeof(country_codes[0]); i<l; i++)
        ListImageList.Add(AdjustBitmapSize(country_flags[i], size, offset));
}

void CslListCtrl::ListAdjustSize(const wxSize& size)
{
    wxInt32 w=size==wxDefaultSize ? GetClientSize().x-12 : size.x-12;

    if (w<0)
        return;

    wxInt32 c=m_columns.GetCount(true);

    if (c>1)
    {
        wxInt32 i, s=0, l=m_columns.GetCount();
        vector<CslListColumns::Column>& columns=m_columns.GetColumns();

        for (i=0; i<l; i++)
        {
            if (m_columns.IsEnabled(i))
                s+=(columns[i].Width=w/c*columns[i].Weight);
        }

        if (s!=w)
        {
            float scale=w/(float)s;

            for (i=0; i<l; i++)
            {
                if (m_columns.IsEnabled(i))
                {
                    columns[i].Width*=scale;
                    SetColumnWidth(m_columns.GetColumnId(i, false), columns[i].Width);
                }
            }
        }
    }
    else if (c==1)
        SetColumnWidth(0, w);
}
