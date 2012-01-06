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

#ifndef CSLGUITOOLS_H
#define CSLGUITOOLS_H

/**
    @author Glen Masgai <mimosius@users.sourceforge.net>
*/

wxBitmap AdjustBitmapSize(const char **data, const wxSize& size, const wxPoint& origin);
wxBitmap AdjustBitmapSize(const wxBitmap& bitmap, const wxSize& size, const wxPoint& origin);
wxBitmap BitmapFromData(wxInt32 type, const unsigned char *data, wxInt32 size);
bool BitmapFromWindow(wxWindow *window, wxBitmap& bitmap);
wxImage& OverlayImage(wxImage& dst, const wxImage& src, wxInt32 offx, wxInt32 offy);

void RegisterEventsRecursively(wxInt32 id, wxWindow *parent, wxEvtHandler *handler,
                               wxEventType type, wxObjectEventFunction function);

wxWindow* GetParentWindow(wxWindow *window, wxInt32 depth);
wxWindow* GetParentWindowByClassInfo(wxWindow *child, wxClassInfo *classinfo);
wxWindow* GetChildWindowByClassInfo(wxWindow *parent, wxClassInfo *classinfo);
vector<wxWindow*>& GetChildWindowsByClassInfo(wxWindow *parent, wxClassInfo *classinfo, vector<wxWindow*>& list);
bool WindowHasChildWindow(wxWindow *parent, wxWindow *child);

void SetTextCtrlErrorState(wxTextCtrl *ctrl, bool error);

wxSize GetBestWindowSizeForText(wxWindow *window, const wxString& text,
                                wxInt32 minWidth, wxInt32 maxWidth,
                                wxInt32 minHeight, wxInt32 maxHeight,
                                wxInt32 scrollbar=0);

#endif //CSLGUITOOLS_H
