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

#ifndef CSLGUITOOLS_H
#define CSLGUITOOLS_H

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EXPORTED_EVENT_TYPE(CSL_DLL_GUITOOLS, wxCSL_EVT_SETTINGS_CHANGED, wxID_ANY)
END_DECLARE_EVENT_TYPES()

#define CSL_EVT_SETTINGS_CHANGED(fn)                                            \
    DECLARE_EVENT_TABLE_ENTRY(                                                  \
                               wxCSL_EVT_SETTINGS_CHANGED, wxID_ANY, wxID_ANY,  \
                               (wxObjectEventFunction)(wxEventFunction)         \
                               wxStaticCastEvent(wxNotifyEventFunction, &fn),   \
                               (wxObject*)NULL                                  \
                             ),


inline wxBitmapType Csl2wxBitmapType(wxInt32 type)
{
    static const wxBitmapType types[] = {
        wxBITMAP_TYPE_BMP,
        wxBITMAP_TYPE_GIF,
        wxBITMAP_TYPE_JPEG,
        wxBITMAP_TYPE_PNG,
        wxBITMAP_TYPE_PCX,
        wxBITMAP_TYPE_PNM,
        wxBITMAP_TYPE_TIF,
        wxBITMAP_TYPE_XPM,
        wxBITMAP_TYPE_ICO,
        wxBITMAP_TYPE_CUR,
        wxBITMAP_TYPE_ANI,
        wxBITMAP_TYPE_ANY
    };
    return (type<0 || (size_t)type>sizeof(type)/sizeof(types[0])) ?
                wxBITMAP_TYPE_ANY : types[type];
}

#define CSL_SET_WINDOW_ICON() \
    SetIcon(((wxTopLevelWindow*)wxTheApp->GetTopWindow())->GetIcon())

#define CSL_CENTRE_DIALOG()                              \
    do {                                                 \
        wxWindow *parent = GetParent();                  \
                                                         \
        if (!parent)                                     \
            parent = wxTheApp->GetTopWindow();           \
        if (parent && parent!=this && parent->IsShown()) \
            CentreOnParent();                            \
        else                                             \
            CentreOnScreen();                            \
    } while (0)

#define CSL_CENTER_DIALOG() CSL_CENTRE_DIALOG()

#define COLOUR2INT(col) ((col.Red()<<16)|(col.Green()<<8)|col.Blue())
#define INT2COLOUR(val) wxColour((val>>16)&0xFF, (val>>8)&0xFF, val&0xFF)

#define CSL_SYSCOLOUR(x) wxSystemSettings::GetColour(x)
#define CSL_SYSMETRIC(x, w) wxSystemSettings::GetMetric(x, w)

WX_DEFINE_ARRAY(wxWindow*, CslArraywxWindow);

class CSL_DLL_GUITOOLS CslBufferedStaticBitmap : public wxPanel
{
    private:
        DECLARE_DYNAMIC_CLASS_NO_COPY(CslBufferedStaticBitmap)

        CslBufferedStaticBitmap() { }
    public:
        CslBufferedStaticBitmap(wxWindow *parent,
                                const wxSize& size = wxSize(16, 16),
                                const wxBitmap& bmp = wxNullBitmap,
                                long flags = wxNO_BORDER);

        void SetBitmap(const wxBitmap& bmp);
        wxBitmap GetBitmap() const { return m_bmp; }

    private:
        void OnPaint(wxPaintEvent& event);

        wxBitmap m_bmp;
        wxSize m_size;
        wxSize m_bmpSize;
};

template<size_t N>
inline void EnableWindows(wxWindow* (&t)[N], bool enable)
{
    loopi(N) t[i]->Enable(enable);
}
CSL_DLL_GUITOOLS wxBitmap AdjustBitmapSize(const char **data, const wxSize& size, const wxPoint& origin);
CSL_DLL_GUITOOLS wxBitmap AdjustBitmapSize(const wxBitmap& bitmap, const wxSize& size, const wxPoint& origin);

CSL_DLL_GUITOOLS wxBitmap BitmapFromData(wxInt32 type, const void *data, wxInt32 size);
template<size_t N>
inline wxBitmap BitmapFromData(wxInt32 type, const unsigned char (&t)[N])
{
    return BitmapFromData(type, (void*)t, (wxInt32)N);
}
CSL_DLL_GUITOOLS bool BitmapFromWindow(wxWindow *window, wxBitmap& bitmap);
CSL_DLL_GUITOOLS wxBitmap RescaleBitmap(const wxBitmap& bitmap, const wxSize& size, bool high);
CSL_DLL_GUITOOLS wxImage& OverlayImage(wxImage& dst, const wxImage& src, wxInt32 offx, wxInt32 offy);

CSL_DLL_GUITOOLS void ConnectEventRecursive(wxInt32 id, wxWindow *parent, wxEvtHandler *handler,
                                            wxEventType type, wxObjectEventFunction function);
CSL_DLL_GUITOOLS void DisconnectEventRecursive(wxInt32 id, wxWindow *parent, wxEvtHandler *handler,
                                               wxEventType type, wxObjectEventFunction function);

CSL_DLL_GUITOOLS CslArraywxWindow& GetChildWindows(wxWindow *parent, CslArraywxWindow& list);
CSL_DLL_GUITOOLS wxWindow* GetParentWindow(wxWindow *window, wxInt32 depth);
CSL_DLL_GUITOOLS wxWindow* GetParentWindowByClassInfo(wxWindow *child, wxClassInfo *classinfo);
CSL_DLL_GUITOOLS wxWindow* GetChildWindowByClassInfo(wxWindow *parent, wxClassInfo *classinfo);
CSL_DLL_GUITOOLS wxUint32 GetChildWindowsByClassInfo(wxWindow *parent, wxClassInfo *classinfo, CslArraywxWindow& list);
CSL_DLL_GUITOOLS bool WindowHasChildWindow(wxWindow *parent, wxWindow *child);

CSL_DLL_GUITOOLS void SetTextCtrlErrorState(wxTextCtrl *ctrl, bool error);
CSL_DLL_GUITOOLS void SetSearchCtrlErrorState(wxSearchCtrl *ctrl, bool error);

CSL_DLL_GUITOOLS wxSize GetBestWindowSizeForText(wxWindow *window, const wxString& text,
                                                 wxInt32 minWidth, wxInt32 maxWidth,
                                                 wxInt32 minHeight, wxInt32 maxHeight,
                                                 wxInt32 scrollbar=0);

#endif //CSLGUITOOLS_H
