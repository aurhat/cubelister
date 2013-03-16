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
#include "CslSettings.h"
#include "CslGeoIP.h"
#include "CslMenu.h"
#include "CslListCtrl.h"

#include "img/ext_green_8.xpm"
#include "img/red_ext_list_16.xpm"
#include "img/green_ext_list_16.xpm"
#include "img/yellow_ext_list_16.xpm"
#include "img/info_16.xpm"
#include "img/sortasc_16.xpm"
#include "img/sortdsc_16.xpm"
#include "CslFlags.h"

// since wx 2.8.4 the list control items are getting deselected on wxGTK
// this also happens on wxMAC (maybe because of the generic list control?)
#if defined(__WXGTK__) || defined(__WXMAC__) || defined(__WXUNIVERSAL__)
#ifndef CSL_USE_WX_LIST_DESELECT_WORKAROUND
#if wxVERSION_NUMBER > 2804 || defined(__WXMAC__)
#define CSL_USE_WX_LIST_DESELECT_WORKAROUND
#endif
#endif
#endif

DEFINE_EVENT_TYPE(wxCSL_EVT_COMMAND_LIST_COLUMN_TOGGLED)
DEFINE_EVENT_TYPE(wxCSL_EVT_COMMAND_LIST_ALL_ITEMS_SELECTED)

BEGIN_EVENT_TABLE(CslListCtrl, wxListCtrl)
    #ifdef __WXMSW__
    EVT_ERASE_BACKGROUND(CslListCtrl::OnEraseBackground)
    #endif
    EVT_LIST_COL_BEGIN_DRAG(wxID_ANY, CslListCtrl::OnColumnDragStart)
    EVT_LIST_COL_END_DRAG(wxID_ANY, CslListCtrl::OnColumnDragEnd)
    EVT_SIZE(CslListCtrl::OnSize)
    EVT_MOTION(CslListCtrl::OnMouseMove)
    EVT_KEY_DOWN(CslListCtrl::OnKeyDown)
    EVT_LIST_COL_CLICK(wxID_ANY, CslListCtrl::OnColumnLeftClick)
    EVT_LIST_COL_RIGHT_CLICK(wxID_ANY, CslListCtrl::OnColumnRightClick)
    CSL_EVT_TOOLTIP(wxID_ANY, CslListCtrl::OnToolTip)
END_EVENT_TABLE()

IMPLEMENT_DYNAMIC_CLASS(CslListSort, wxObject)
IMPLEMENT_DYNAMIC_CLASS(CslListColumn, wxObject)
IMPLEMENT_DYNAMIC_CLASS(CslListColumns, wxObject)
IMPLEMENT_DYNAMIC_CLASS(CslListCtrl, wxListCtrl)

wxImageList CslListCtrl::ListImageList;


void CslListColumns::AddColumn(const wxString& name, wxListColumnFormat format,
                                   float weight, bool enabled, bool locked)
{
    CSL_FLAG_SET(m_columnMask, enabled ? 1<<GetCount() : 0);

    m_columns.push_back(new CslListColumn(name, format, weight, locked));

    if (locked)
        m_lockedCount++;
}

void CslListColumns::RemoveColumn(wxInt32 id)
{
    wxASSERT_MSG(id>=0 && id<(wxInt32)m_columns.size(), wxT("Invalid column id."));

    CslListColumn *c = m_columns[id];

    if (c->Locked)
        m_lockedCount--;

    CSL_FLAG_UNSET(m_columnMask, 1<<id);

    delete c;

    m_columns.RemoveAt(id);
}

void CslListColumns::ToggleColumn(wxInt32 id)
{
    wxASSERT_MSG(id>=0 && id<GetCount(), wxT("Invalid column id."));

    if (id<0 || id>=GetCount())
        return;

    if (CSL_FLAG_CHECK(m_columnMask, 1<<id))
        CSL_FLAG_UNSET(m_columnMask, 1<<id);
    else
        CSL_FLAG_SET(m_columnMask, 1<<id);
}

wxInt32 CslListColumns::GetColumnId(wxInt32 id, bool absolute) const
{
    wxASSERT_MSG(id>=0 && id<GetCount(), wxT("invalid column"));

    if (absolute)
    {
        for (wxInt32 i=0; i<=id && i<GetCount(); i++)
        {
            if (!IsEnabled(i))
                id++;
        }
        id=min(id, GetCount()-1);
    }
    else
    {
        for (wxInt32 i = 0, c = id; i<c; i++)
        {
            if (!IsEnabled(i))
                id--;
        }
        id = max(0, id);
    }

    return id;
}


CslListCtrl::CslListCtrl(wxWindow* parent, wxWindowID id,
                         const wxPoint& pos, const wxSize& size,
                         long style, const wxValidator& validator,
                         const wxString& name) :
        wxListCtrl(parent, id, pos, size, style, validator, name),
        m_mouseLastMove(0),
        m_dontAdjustSize(false), m_userColumnSize(false),
        m_processSelectEvent(1),
        m_sortCallback(NULL)
{
    SetImageList(&ListImageList, wxIMAGE_LIST_SMALL);

    Connect(wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler(CslListCtrl::OnItem), NULL, this);
    Connect(wxEVT_COMMAND_LIST_ITEM_DESELECTED, wxListEventHandler(CslListCtrl::OnItem), NULL, this);
    Connect(wxEVT_COMMAND_LIST_ITEM_ACTIVATED, wxListEventHandler(CslListCtrl::OnItem), NULL, this);
    Connect(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(CslListCtrl::OnMenu), NULL, this);
    Connect(wxEVT_CONTEXT_MENU, wxContextMenuEventHandler(CslListCtrl::OnContextMenu), NULL, this);
}

CslListCtrl::~CslListCtrl()
{
    CslToolTip::Reset();
}

#ifdef __WXMSW__
void CslListCtrl::OnEraseBackground(wxEraseEvent& event)
{
    // to prevent flickering, erase only content *outside* of the actual items
    //
    // another solution is to set the LVS_EX_DOUBLEBUFFER style
    // (commctrl 6.0) within the ctor but it needs a little more cpu:
    //   HWND hwnd = (HWND)GetHWND();
    //   ListView_SetExtendedListViewStyle(hwnd, ListView_GetExtendedListViewStyle(hwnd)|LVS_EX_DOUBLEBUFFER);

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
            wxRegion imgRegion(0, 0, imgSize.x, imgSize.y);
            imgRegion.Offset(bRect.x, bRect.y+1);
            region.Xor(imgRegion);

            for (wxInt32 i=1; i<GetItemCount() && i<=bItem; i++)
            {
                imgRegion.Offset(0, bRect.height+3);
                region.Xor(imgRegion);
            }
        }

        dc->DestroyClippingRegion();
#if wxCHECK_VERSION(2, 9, 0)
        dc->SetDeviceClippingRegion(region);
#else
        dc->SetClippingRegion(region);
#endif
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

void CslListCtrl::OnColumnDragStart(wxListEvent& WXUNUSED(event))
{
    m_dontAdjustSize = m_userColumnSize = true;
}

void CslListCtrl::OnColumnDragEnd(wxListEvent& WXUNUSED(event))
{
    m_dontAdjustSize = false;
}

void CslListCtrl::OnSize(wxSizeEvent& event)
{
    if (m_dontAdjustSize)
        return;

    CslValueRestore<bool> noresize(m_dontAdjustSize, true);

    ListAdjustSize();

    event.Skip();
}

void CslListCtrl::OnMouseMove(wxMouseEvent& event)
{
    event.Skip();

    if (!CslGetSettings().TooltipDelay)
        return;

    wxUint32 ticks=GetTicks();

    if (ticks-m_mouseLastMove<CSL_TOOLTIP_DELAY_STEP)
        return;

    m_mouseLastMove=ticks;

    CslToolTip::Init(this, CslGetSettings().TooltipDelay);
}

void CslListCtrl::OnMenu(wxCommandEvent& event)
{
    wxInt32 id = event.GetId();

    if (id==MENU_SAVEIMAGE)
        CreateScreenShot();
    else if (id==MENU_LISTCTRL_COLUMN_ADJUST)
    {
        m_userColumnSize = false;

        ListAdjustSize();
    }
    else if (CSL_MENU_EVENT_IS_COLUMN(id))
    {
        id -= MENU_LISTCTRL_COLUMN;

        ListToggleColumn(id);
    }

    event.Skip();
}

void CslListCtrl::OnContextMenu(wxContextMenuEvent& event)
{
    CslToolTip::Reset();
    event.Skip();
}

void CslListCtrl::OnKeyDown(wxKeyEvent &event)
{
    if (!(GetWindowStyle()&wxLC_SINGLE_SEL) && event.ControlDown() && event.GetKeyCode()=='A')
    {
        if (!IsFrozen())
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

void CslListCtrl::OnColumnRightClick(wxListEvent& WXUNUSED(event))
{
    CslToolTip::Reset();

    wxMenu menu;
    wxMenuItem *item = NULL;

    wxInt32 count = m_columns.GetCount();

    if (count && count>m_columns.GetLockedCount())
    {
        CslArrayListColumn& columns = m_columns.GetColumns();

        count = m_columns.GetCount(true);

        loopv(columns)
        {
            const CslListColumn& c = *columns[i];

            if (c.Locked)
                continue;

            item = new wxMenuItem(&menu, MENU_LISTCTRL_COLUMN+i, c.Name, wxART_NONE, wxITEM_CHECK);
            menu.Append(item);
            item->Check(m_columns.IsEnabled(i));

            if (count==1 && m_columns.IsEnabled(i))
                 menu.Enable(MENU_LISTCTRL_COLUMN+i, false);
        }
    }

    if (HasHeader())
    {
        if (item)
            menu.AppendSeparator();

        item = new wxMenuItem(&menu, MENU_LISTCTRL_COLUMN_ADJUST, _("Adjust columns"));
        menu.Append(item);
    }

    if (!item)
        return;

    wxPoint point = wxDefaultPosition;
    //from keyboard
    if (point==wxDefaultPosition)
        point = wxGetMousePosition();
    point = ScreenToClient(point);

    PopupMenu(&menu, point);
}

void CslListCtrl::OnItem(wxListEvent& event)
{
    //if (event.GetEventType()!=wxEVT_COMMAND_LIST_ITEM_DESELECTED)
    //    CslToolTip::Reset();

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
    wxInt32 offset = GetHeaderHeight();
    const wxSize& size = GetClientSize();
    const wxPoint& pos = ScreenToClient(event.Pos);

    if (pos.x>=0 && pos.y>=0 && pos.x<=size.x && pos.y<=size.y)
    {
        for (wxInt32 i = GetTopItem(); i<GetItemCount(); i++)
        {
            item.SetId(i);
            GetItemRect(item, rect, wxLIST_RECT_BOUNDS);

            rect.y -= offset;

            if (!rect.Contains(pos))
                continue;

            wxString title;
            wxArrayString text;

            GetToolTipText(i, event);
            return;
        }
    }

    event.Skip();
}

void CslListCtrl::ListAddColumn(const wxString& name, wxListColumnFormat format,
                                float weight, bool enabled, bool locked)
{
    m_columns.AddColumn(name, format, weight, enabled, locked);

    if (enabled && HasHeader())
    {
        wxListItem item;
        wxInt32 rcolumn = m_columns.GetColumnId(m_columns.GetCount()-1, false);

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
        CslListColumn *c=m_columns.GetColumn(column);

        item.SetMask(wxLIST_MASK_FORMAT);
        item.SetImage(-1);
        item.SetAlign(c->Format);
        item.SetText(c->Name);

        InsertColumn(colrel, item);
        SetColumn(colrel, item);
    }

    m_columns.ToggleColumn(column);

    // process the event immediately
    wxListEvent evt(wxCSL_EVT_COMMAND_LIST_COLUMN_TOGGLED);
    evt.m_col = column;
    ProcessEvent(evt);

    ListAdjustSize();

    return m_columns.GetColumnMask();
}

bool CslListCtrl::ListSetColumn(wxInt32 column, const wxString& name)
{
    CslListColumn *c=m_columns.GetColumn(column);

    if (!c)
        return false;

    c->Name=name;

    if (!m_columns.IsEnabled(column))
        return false;

    wxListItem item;

    column=m_columns.GetColumnId(column, false);
    GetColumn(column, item);

    item.SetMask(wxLIST_MASK_TEXT);
    item.SetText(name);

    SetColumn(column, item);

    return true;
}

bool CslListCtrl::ListSetItem(wxInt32 row, wxInt32 column, const wxString& text, wxInt32 image)
{
    if (!m_columns.IsEnabled(column))
        return false;

    wxListItem item;
    wxInt32 l=GetItemCount();

    column=m_columns.GetColumnId(column, false);

    while (l++<=row)
        InsertItem(l, -1);

    item.SetId(row);

    SetItem(row, column, text, image);

    return true;
}

void CslListCtrl::ListLockColumn(wxInt32 column, bool lock)
{
    CslListColumn *c = m_columns.GetColumn(column);

    if (c)
        c->Locked = lock;
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

void CslListCtrl::SetSortCallback(wxListCtrlCompare fn, wxInt32 mode, wxInt32 column)
{
    m_sortCallback=fn;
    m_sortHelper.Init(mode, column);

    wxListItem item;
    wxInt32 img, id;

    if (m_sortHelper.Mode==CslListSort::SORT_ASC)
        img=CSL_LIST_IMG_SORT_ASC;
    else
        img=CSL_LIST_IMG_SORT_DSC;

    id=m_columns.GetColumnId(column, false);

    GetColumn(id, item);
    item.SetImage(img);
    SetColumn(id, item);
}

void CslListCtrl::ListSort(wxInt32 column)
{
    if (!m_sortCallback || !m_columns.GetCount())
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

    SortItems(m_sortCallback, (long)&m_sortHelper);

#ifdef CSL_USE_WX_LIST_DESELECT_WORKAROUND
    loopi(m_selected.GetCount())
    {
        wxInt32 v;

        if ((v=ListFindItem(m_selected.Item(i)))==wxNOT_FOUND)
            continue;

        SetItemState(v, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
    }
#endif // CSL_USE_WX_LIST_DESELECT_WORKAROUND

    m_processSelectEvent=1;

#ifndef __WXMSW__
#if !wxCHECK_VERSION(2, 9, 0) //TODO: replacement needed?
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
        dlg.SetPath(CslGetSettings().ScreenOutputPath+CSL_PATHDIV_WX+file);
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

wxInt32 CslListCtrl::GetGameImage(CslEngine* engine, wxUint32 fourcc, wxInt32 offset)
{
    CslArrayCslGame& games=engine->GetGames();

    loopv(games)
    {
        if (games[i]->GetFourCC()==fourcc)
            return CSL_LIST_IMG_GAMES_START+offset+i*2;
    }

    return -1;
}

wxInt32 CslListCtrl::GetCountryFlag(CslEngine* engine, wxUint32 ip)
{
    wxInt32 i;
    const char *country;
    static wxInt32 offset=0;

    if (!offset)
        offset=CSL_LIST_IMG_GAMES_START+engine->GetGames().size()*2;

    if (ip && ip!=(wxUint32)-1)
    {
        if (!(country=CslGeoIP::GetCountryCodeByIPnum(ip)))
        {
            if (IsLocalIPV4(ip))
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

void CslListCtrl::CreateImageList(CslEngine* engine)
{
    wxSize size(18, 14);
    wxPoint offset(0, 0);

#ifdef __WXMSW__
    size.x = 20;
#endif
    ListImageList.Create(size.x, size.y, true);

    ListImageList.Add(AdjustBitmapSize(GET_ART_MENU(wxART_BUBBLE_GREEN), size, offset));
    ListImageList.Add(AdjustBitmapSize(GET_ART_MENU(wxART_BUBBLE_YELLOW), size, offset));
    ListImageList.Add(AdjustBitmapSize(GET_ART_MENU(wxART_BUBBLE_RED), size, offset));
    ListImageList.Add(AdjustBitmapSize(GET_ART_MENU(wxART_BUBBLE_GREY), size, offset));
    ListImageList.Add(AdjustBitmapSize(green_ext_list_16_xpm, size, offset));
    ListImageList.Add(AdjustBitmapSize(yellow_ext_list_16_xpm, size, offset));
    ListImageList.Add(AdjustBitmapSize(red_ext_list_16_xpm, size, offset));
    ListImageList.Add(AdjustBitmapSize(info_16_xpm, size, offset));
    offset.y=2;
    ListImageList.Add(AdjustBitmapSize(sortasc_16_xpm, size, offset));
    ListImageList.Add(AdjustBitmapSize(sortdsc_16_xpm, size, offset));

    const CslArrayCslGame& games = engine->GetGames();
    const wxImage imgExt = wxBitmap(ext_green_8_xpm).ConvertToImage();

    offset.y = 0;

    loopv(games)
    {
        const CslGameIcon *icon=games[i]->GetIcon(16);

        wxBitmap bmpGame = icon ?
            BitmapFromData(Csl2wxBitmapType(icon->Type), icon->Data, icon->DataSize) :
            wxNullBitmap;

        wxBitmap bmp=bmpGame.IsOk() ?
            AdjustBitmapSize(bmpGame, size, offset) :
            wxBitmap(size.x, size.y);

        ListImageList.Add(bmp);
        wxImage img = bmp.ConvertToImage();

        ListImageList.Add(wxBitmap(OverlayImage(img, imgExt, size.x-8, size.y-8)));
    }

#ifdef __WXMSW__
    offset.y = 2;
#else
    offset.y = 1;
#endif //__WXMSW__

    ListImageList.Add(AdjustBitmapSize(local_xpm, size, offset));
    ListImageList.Add(AdjustBitmapSize(unknown_xpm, size, offset));

    loopi(sizeof(country_codes)/sizeof(country_codes[0]))
        ListImageList.Add(AdjustBitmapSize(country_flags[i], size, offset));
}

void CslListCtrl::ListAdjustSize(const wxSize& size)
{
    if (m_userColumnSize)
        return;

    wxSize cs = size==wxDefaultSize ? GetClientSize() : size;

    if (cs.x<=0 || cs.y<=0)
        return;

#if !defined(__WXMSW__) && !wxCHECK_VERSION(2, 9, 0)
    // header and scrollbars aren't excluded in GetClientSize()
    // when using the *generic* wxListCtrl on wx 2.8
    // check the height of all items and substract the vertical
    // scrollbar width if necessary
    wxInt32 count = GetItemCount();

    if (count>0)
    {
        wxRect rect;
        GetItemRect(0, rect);
        wxInt32 sw = wxSystemSettings::GetMetric(wxSYS_HSCROLL_Y) + 1;

        if (rect.height*count>cs.y-GetHeaderHeight())
            cs.x = max(cs.x-sw, sw);
    }
#endif

    wxInt32 c = m_columns.GetCount(true);

    if (c>1)
    {
        wxInt32 s = 0;
        CslArrayListColumn& columns = m_columns.GetColumns();

        loopi(m_columns.GetCount())
        {
            if (m_columns.IsEnabled(i))
                s += (columns[i]->Width = cs.x/c*columns[i]->Weight);
        }

        if (s>cs.x || cs.x-s>1)
        {
            float scale = cs.GetWidth()/(float)s;

            loopi(m_columns.GetCount()) if (m_columns.IsEnabled(i))
            {
                columns[i]->Width *= scale;

                SetColumnWidth(m_columns.GetColumnId(i, false), columns[i]->Width);
            }
        }
    }
    else if (c==1)
        SetColumnWidth(0, cs.x);
}
