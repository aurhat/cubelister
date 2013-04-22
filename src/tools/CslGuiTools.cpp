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

#include <Csl.h>
#include <wx/mstream.h>

DEFINE_EVENT_TYPE(wxCSL_EVT_SETTINGS_CHANGED)

IMPLEMENT_DYNAMIC_CLASS(CslBufferedStaticBitmap, wxPanel)

CslBufferedStaticBitmap::CslBufferedStaticBitmap(wxWindow *parent, const wxSize& size,
                                                 const wxBitmap& bmp, long flags) :
        wxPanel(parent, wxID_ANY, wxDefaultPosition, size, flags),
        m_size(size)
{
    Connect(wxEVT_PAINT, wxPaintEventHandler(CslBufferedStaticBitmap::OnPaint), NULL, this);
    SetBitmap(bmp);
}

void CslBufferedStaticBitmap::SetBitmap(const wxBitmap& bmp)
{
    if (!bmp.IsNull())
    {
        wxSize bmpSize(bmp.GetWidth(), bmp.GetHeight());
        wxASSERT(m_size.x >= m_bmpSize.x || m_size.y >= m_bmpSize.y);

        m_bmp = bmp;
        m_bmpSize = bmpSize;
    }

    Refresh(false);
}

void CslBufferedStaticBitmap::OnPaint(wxPaintEvent& WXUNUSED(event))
{
    wxPaintDC dc(this);

    if (!m_bmp.IsNull())
    {
        wxPoint offset(m_size.x/2-m_bmpSize.x/2, m_size.y/2-m_bmpSize.y/2);

        dc.DrawBitmap(m_bmp, offset, true);
    }
}

wxBitmap AdjustBitmapSize(const char **data, const wxSize& size, const wxPoint& origin)
{
    wxBitmap bitmap=wxBitmap(data);

    return AdjustBitmapSize(bitmap, size, origin);
}

wxBitmap AdjustBitmapSize(const wxBitmap& bitmap, const wxSize& size, const wxPoint& origin)
{
    wxASSERT(bitmap.IsOk());

    if (!bitmap.IsOk())
        return wxNullBitmap;

    wxImage image=bitmap.ConvertToImage();
    image.Resize(size, origin);

    return image;
}

wxBitmap RescaleBitmap(const wxBitmap& bitmap, const wxSize& size, bool hq)
{
    wxASSERT(bitmap.IsOk());

    if (!bitmap.IsOk())
        return wxNullBitmap;

    wxImage image = bitmap.ConvertToImage();
    image.Rescale(size.x, size.y, hq ? wxIMAGE_QUALITY_HIGH : wxIMAGE_QUALITY_NORMAL);

    return image;
}

wxBitmap BitmapFromData(wxInt32 type, const void *data, wxInt32 size)
{
    if (data && size)
    {
        wxMemoryInputStream stream(data, size);
        wxImage image(stream, (wxBitmapType)type);
        return image;
    }

    return wxNullBitmap;
}

wxImage& OverlayImage(wxImage& dst, const wxImage& src, wxInt32 offx, wxInt32 offy)
{
    unsigned char r, g, b, mr, mg, mb;
    wxInt32 c, wsrc, xsrc, ysrc, xdst, ydst;
    bool alpha=dst.HasAlpha();
    bool mask=src.GetOrFindMaskColour(&mr, &mg, &mb);

    wsrc=src.GetWidth();
    c=src.GetHeight()*wsrc;

    loopi(c)
    {
        xsrc=i%wsrc;
        ysrc=i/wsrc;
        xdst=offx+i%wsrc;
        ydst=offy+i/wsrc;

        r=src.GetRed(xsrc, ysrc);
        g=src.GetGreen(xsrc, ysrc);
        b=src.GetBlue(xsrc, ysrc);

        if (mask && alpha)
        {
            if (r!=mr && g!=mg && b!=mb)
            {
                dst.SetRGB(xdst, ydst, r, g, b);
                dst.SetAlpha(xdst, ydst, 255);
            }
        }
        else
            dst.SetRGB(xdst, ydst, r, g, b);
    }

    return dst;
}

bool BitmapFromWindow(wxWindow *window, wxBitmap& bitmap)
{
    bool ret;
    wxMemoryDC mdc;
    wxClientDC cdc(window);
    const wxSize& size=window->GetClientSize();

    window->Raise();
    wxTheApp->Yield();

    bitmap.Create(size.x, size.y);
    mdc.SelectObject(bitmap);
    ret=mdc.Blit(0, 0, size.x, size.y, &cdc, 0, 0);
    mdc.SelectObject(wxNullBitmap);

    return ret;
}

void ConnectEventRecursive(wxInt32 id, wxWindow *parent, wxEvtHandler *handler,
                           wxEventType type, wxObjectEventFunction function)
{
    if (parent)
    {
        parent->Connect(id, type, function, NULL, handler);

        wxWindowList::compatibility_iterator node = parent->GetChildren().GetFirst();

        while (node)
        {
            wxWindow* child = node->GetData();
            ConnectEventRecursive(id, child, handler, type, function);
            node = node->GetNext();
        }
    }
}

void DisconnectEventRecursive(wxInt32 id, wxWindow *parent, wxEvtHandler *handler,
                              wxEventType type, wxObjectEventFunction function)
{
    if (parent)
    {
        parent->Disconnect(id, type, function, NULL, handler);

        wxWindowList::compatibility_iterator node = parent->GetChildren().GetFirst();

        while (node)
        {
            wxWindow* child = node->GetData();
            DisconnectEventRecursive(id, child, handler, type, function);
            node = node->GetNext();
        }
    }
}

CslArraywxWindow& GetChildWindows(wxWindow *parent, CslArraywxWindow& list)
{
    wxWindowList::compatibility_iterator node=parent->GetChildren().GetFirst();

    while (node)
    {
        wxWindow* child=node->GetData();

        list.push_back(child);
        node=node->GetNext();
        GetChildWindows(child, list);
    }

    return list;
}

wxWindow* GetParentWindow(wxWindow *window, wxInt32 depth)
{
    while (window && --depth>=0)
        window=window->GetParent();

    return window;
}

wxWindow* GetParentWindowByClassInfo(wxWindow *child, wxClassInfo *classinfo)
{
    wxWindow *window=child;

    while (window)
    {
        if (window->IsKindOf(classinfo))
            return window;

        window=window->GetParent();
    }

    return NULL;
}

wxWindow* GetChildWindowByClassInfo(wxWindow *parent, wxClassInfo *classinfo)
{
    wxWindowList::compatibility_iterator node=parent->GetChildren().GetFirst();

    while (node)
    {
        wxWindow* child=node->GetData();

        if (child->IsKindOf(classinfo))
            return child;

        node=node->GetNext();
    }

    return NULL;
}

wxUint32 GetChildWindowsByClassInfo(wxWindow *parent, wxClassInfo *classinfo, CslArraywxWindow& list)
{
    if (!parent)
        parent=wxTheApp->GetTopWindow();

    GetChildWindows(parent, list);

    loopvrev(list)
    {
        wxWindow *child=list[i];

        if (!child->IsKindOf(classinfo))
            list.RemoveAt(i);
    }

    return list.GetCount();
}

bool WindowHasChildWindow(wxWindow *parent, wxWindow *child)
{
    wxWindowList::compatibility_iterator node=parent->GetChildren().GetFirst();

    while (node)
    {
        wxWindow* window=node->GetData();

        if (window==child)
            return true;

        node=node->GetNext();
    }

    return false;
}

void SetTextCtrlErrorState(wxTextCtrl *ctrl, bool error)
{
    if (error)
    {
#ifdef __WXMAC__
        ctrl->SetForegroundColour(*wxRED);
#else
        ctrl->SetBackgroundColour(wxColour(255, 100, 100));
        ctrl->SetForegroundColour(*wxWHITE);
#endif
    }
    else
    {
#ifdef __WXMAC__
        // very ugly - setting back to black doesnt work, so add 1
        ctrl->SetForegroundColour(CSL_SYSCOLOUR(wxSYS_COLOUR_WINDOWTEXT).Red()+1);
#else
        ctrl->SetBackgroundColour(CSL_SYSCOLOUR(wxSYS_COLOUR_WINDOW));
        ctrl->SetForegroundColour(CSL_SYSCOLOUR(wxSYS_COLOUR_WINDOWTEXT));
#endif
    }
#ifdef __WXMSW__
    ctrl->Refresh();
#endif
}

void SetSearchCtrlErrorState(wxSearchCtrl *ctrl, bool error)
{
    wxTextCtrl *tctrl = NULL;

    if ((tctrl = (wxTextCtrl*)GetChildWindowByClassInfo(ctrl, CLASSINFO(wxTextCtrl))))
        SetTextCtrlErrorState(tctrl, error);
}

wxSize GetBestWindowSizeForText(wxWindow *window, const wxString& text,
                                wxInt32 minWidth, wxInt32 maxWidth,
                                wxInt32 minHeight, wxInt32 maxHeight,
                                wxInt32 scrollbar)
{
    wxCoord w, h, border, scroll;
    wxClientDC dc(window);
    wxFont font=window->GetFont();

    dc.GetMultiLineTextExtent(text, &w, &h, NULL, &font);

    if (maxWidth<0)
        maxWidth=w;
    if (maxHeight<0)
        maxHeight=h;

    w=clamp(w, minWidth, maxWidth);
    h=clamp(h, minHeight, maxHeight);

    border = CSL_SYSMETRIC(wxSYS_BORDER_X, window);
    scroll = CSL_SYSMETRIC(wxSYS_VSCROLL_X, window);
    //guess some border size on systems not supporting it (wxGTK)
    w+=(border=2*(border<0 ? 4 : border));
    //scrollbar is always shown on these systems
#if defined (__WXMSW__) || defined(__WXMAC__)
    if (scrollbar&wxVERTICAL)
        w+=scroll;
#else
    if (h>=maxHeight)
        w+=scroll;
#endif

#ifdef __WXMSW__
    border+=4;
#endif //__WXMSW__
    h+=border;

    return wxSize(w, h);
}
