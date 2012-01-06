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

#include <Csl.h>

wxBitmap AdjustBitmapSize(const char **data, const wxSize& size, const wxPoint& origin)
{
    wxBitmap bitmap=wxBitmap(data);

    return AdjustBitmapSize(bitmap, size, origin);
}

wxBitmap AdjustBitmapSize(const wxBitmap& bitmap, const wxSize& size, const wxPoint& origin)
{
    if (!bitmap.IsOk())
        return wxNullBitmap;

    wxImage image=bitmap.ConvertToImage();
    image.Resize(size, origin);

    return wxBitmap(image);
}

wxBitmap BitmapFromData(wxInt32 type, const unsigned char *data, wxInt32 size)
{
    wxMemoryInputStream stream(data, size);
    // see wx_wxbitmap.html
#ifdef __WXMSW__
    return wxBitmap(stream, type);
#else
    wxImage image(stream, type);
    return wxBitmap(image);
#endif

    return wxNullBitmap;
}

wxImage& OverlayImage(wxImage& dst, const wxImage& src, wxInt32 offx, wxInt32 offy)
{
    unsigned char r, g, b, mr, mg, mb;
    wxUint32 i, c, wsrc, xsrc, ysrc, xdst, ydst;
    bool alpha=dst.HasAlpha();
    bool mask=src.GetOrFindMaskColour(&mr, &mg, &mb);

    wsrc=src.GetWidth();
    c=src.GetHeight()*wsrc;

    for (i=0; i<c; i++)
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

void RegisterEventsRecursively(wxInt32 id, wxWindow *parent, wxEvtHandler *handler,
                               wxEventType type, wxObjectEventFunction function)
{
    if (parent)
    {
        parent->Connect(id, type, function, NULL, handler);

        wxWindowList::compatibility_iterator node=parent->GetChildren().GetFirst();

        while (node)
        {
            wxWindow* child=node->GetData();
            RegisterEventsRecursively(id, child, handler, type, function);
            node=node->GetNext();
        }
    }
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

vector<wxWindow*>& GetChildWindows(wxWindow *parent, vector<wxWindow*>& list)
{
    wxWindowList::compatibility_iterator node=parent->GetChildren().GetFirst();

    while (node)
    {
        wxWindow* child=node->GetData();

        list.add(child);
        node=node->GetNext();
        GetChildWindows(child, list);
    }

    return list;
}

vector<wxWindow*>& GetChildWindowsByClassInfo(wxWindow *parent, wxClassInfo *classinfo, vector<wxWindow*>& list)
{
    if (!parent)
        parent=wxTheApp->GetTopWindow();

    GetChildWindows(parent, list);

    loopvrev(list)
    {
        wxWindow *child=list[i];

        if (!child->IsKindOf(classinfo))
            list.remove(i);
    }

    return list;
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
        ctrl->SetForegroundColour(SYSCOLOUR(wxSYS_COLOUR_WINDOWTEXT).Red()+1);
#else
        ctrl->SetBackgroundColour(SYSCOLOUR(wxSYS_COLOUR_WINDOW));
        ctrl->SetForegroundColour(SYSCOLOUR(wxSYS_COLOUR_WINDOWTEXT));
#endif
    }
#ifdef __WXMSW__
    ctrl->Refresh();
#endif
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

    border=SYSMETRIC(wxSYS_BORDER_X, window);
    scroll=SYSMETRIC(wxSYS_VSCROLL_X, window);
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
