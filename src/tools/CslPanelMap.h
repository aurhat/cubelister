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

#ifndef CSLPANELMAP_H
#define CSLPANELMAP_H

#include <wx/fileconf.h>

class CSL_DLL_GUITOOLS CslBaseInfo
{
    public:
        CslBaseInfo(wxInt32 x, wxInt32 y) :
                m_point(wxPoint(x, y)), m_colour(*wxWHITE) { }

        wxPoint m_point;
        wxColour m_colour;
};

WX_DEFINE_USER_EXPORTED_ARRAY(CslBaseInfo*, CslArrayCslBaseInfo, class CSL_DLL_GUITOOLS);

class CSL_DLL_GUITOOLS CslMapInfo
{
    public:
        CslMapInfo(const wxString& name=wxT(""), wxInt32 version=-1)
        {
            Reset(name, version);
        }
        ~CslMapInfo() { Reset(); }

        CslMapInfo& operator=(const CslMapInfo& info)
        {
            m_version=info.m_version;
            m_mapName=info.m_mapName;
            m_mapNameFull=info.m_mapNameFull;
            m_author=info.m_author;
            m_bitmap=info.m_bitmap;

            WX_CLEAR_ARRAY(m_bases);
            for (wxUint32 i=0; i<info.m_bases.GetCount(); i++)
                m_bases.Add(new CslBaseInfo(*info.m_bases.Item(i)));

            m_basesOk=info.m_basesOk;
            return *this;
        }

        bool GetMapConfigVersions(wxFileConfig& config, wxArrayInt& array);
        bool LoadMapConfig(wxFileConfig& config, wxInt32 protocol);
        bool LoadMapImage(const wxString& map, const wxString& path);
        bool LoadMapData(const wxString& map, const wxString& game, wxInt32 protocol);
        void Reset(const wxString& mapName=wxEmptyString, wxInt32 version=-1)
        {
            m_version=version;
            m_mapName=mapName;
            m_mapNameFull.Empty();
            m_author.Empty();
            m_basesOk=false;
            WX_CLEAR_ARRAY(m_bases);
        }

        void ResetBasesColour()
        {
            for (wxUint32 i=0; i<m_bases.GetCount(); i++)
                m_bases.Item(i)->m_colour=*wxWHITE;
        }

        wxInt32 m_version;
        wxString m_mapName, m_mapNameFull, m_author;
        wxBitmap m_bitmap;
        CslArrayCslBaseInfo m_bases;
        bool m_basesOk;
};


class CSL_DLL_GUITOOLS CslPanelMap : public wxPanel
{
    public:
        CslPanelMap(wxWindow *parent, wxInt32 id,
                    const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
                    long style = wxTAB_TRAVERSAL, const wxString& name = wxT("panel")) :
                wxPanel(parent, id, pos, size, style, name),
                m_ok(false), m_background(false) { }
        ~CslPanelMap() { Reset(); }

        void SetMap(const wxBitmap& bitmap, const bool refresh = true)
        {
            m_bitmap=bitmap;
            m_ok=true;

            SetMinSize(wxSize(bitmap.GetWidth(), bitmap.GetHeight()));

            if (refresh)
                Refresh(m_background);
        }

        void SetEraseBackGround(bool erase=true) { m_background=erase; }
        void SetOk(const bool ok=true, const bool refresh=false)
        {
            m_ok=ok;
            if (refresh)
                Refresh(m_background);
        }
        bool IsOk() { return m_ok; }

        void UpdateBases(const CslArrayCslBaseInfo& bases, const bool hasBases);

        void Reset()
        {
            SetOk(false, true);
            WX_CLEAR_ARRAY(m_bases);
        }

    private:
        bool m_ok;
        bool m_background;

        void OnPaint(wxPaintEvent& event);
#ifdef __WXMSW__
        void OnErase(wxEraseEvent& event) { if (m_background) event.Skip(); }
#endif

        DECLARE_EVENT_TABLE();

    protected:
        wxBitmap m_bitmap;
        CslArrayCslBaseInfo m_bases;

        wxPoint GetBitmapOrigin()
        {
            wxSize cSize=GetClientSize();
            wxSize bSize(m_bitmap.GetWidth(), m_bitmap.GetHeight());
            return wxPoint(cSize.x/2-bSize.x/2, cSize.y/2-bSize.y/2);
        }
};


#endif // CSLPANELMAP_H
