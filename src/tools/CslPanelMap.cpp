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
#include "CslPanelMap.h"
#include <wx/wfstream.h>

BEGIN_EVENT_TABLE(CslPanelMap, wxPanel)
    EVT_PAINT(CslPanelMap::OnPaint)
    #ifdef __WXMSW__
    EVT_ERASE_BACKGROUND(CslPanelMap::OnErase)
    #endif
END_EVENT_TABLE()


bool CslMapInfo::GetMapConfigVersions(wxFileConfig& config, wxArrayInt& array)
{
    wxUint32 i;
    long int index, val;
    wxString group;

    if (!config.GetFirstGroup(group, index))
        return false;

    do
    {
        if (!group.IsNumber())
            continue;
        if (!group.ToLong(&val))
            continue;

        for (i=0; i<array.GetCount(); i++)
            if (array.Item(i)>val)
                break;
        array.Insert(val, i);
    }
    while (config.GetNextGroup(group, index));

    return array.GetCount()>0;
}

bool CslMapInfo::LoadMapConfig(wxFileConfig& config, wxInt32 protocol)
{
    long int val;
    wxInt32 x, y;
    wxUint32 i=0;

    config.SetPath(wxString::Format(wxT("/%d"), protocol));
    if (!config.Read(wxT("Mapname"), &m_mapNameFull))
        return false;
    if (!config.Read(wxT("Author"), &m_author))
        return false;

    while (true)
    {
        if (!config.Read(wxString::Format(wxT("x%d"), i), &val))
            break;
        x=val;
        if (!config.Read(wxString::Format(wxT("y%d"), i), &val))
            break;
        y=val;

        m_bases.Add(new CslBaseInfo(x, y));
        i++;
    }

    m_version=protocol;

    return true;
}

bool CslMapInfo::LoadMapImage(const wxString& map, const wxString& path)
{
    if (!::wxDirExists(path))
        return false;

    bool ok=false;
    wxString file=path+map;

    Reset(map);

    if (::wxFileExists(file+wxT(".jpg")))
    {
        if (m_bitmap.LoadFile(file+wxT(".jpg"), wxBITMAP_TYPE_JPEG))
            ok=true;
    }
    else if (::wxFileExists(file+wxT(".png")))
    {
        if (!m_bitmap.LoadFile(file+wxT(".png"), wxBITMAP_TYPE_PNG))
            ok=true;
    }

    if (!ok || m_bitmap.GetHeight()>400 || m_bitmap.GetHeight()>300)
    {
        Reset();
        return false;
    }

    return true;
}

bool CslMapInfo::LoadMapData(const wxString& map, const wxString& game, wxInt32 protocol)
{
    wxString file, path;

    path<<wxT("data/maps/")<<game<<wxT("/");
    file<<path<<map<<wxT(".cfg");

    Reset(map);

    if ((file=FindPackageFile(file)).IsEmpty())
        return false;

    wxFileInputStream stream(file);
    if (!stream.IsOk())
        return false;

    wxFileConfig config(stream);

    wxInt32 version;
    wxArrayInt versions;

    if (!GetMapConfigVersions(config, versions))
        return false;

    if (versions.Index(protocol)==wxNOT_FOUND)
        version=versions.Last();
    else
        version=protocol;

    if (!LoadMapConfig(config, version))
        return false;

    file.Empty();
    file<<path<<wxString::Format(wxT("%d/"), version)<<map<<wxT(".png");

    if (!(file=FindPackageFile(file)).IsEmpty())
    {
        if (!m_bitmap.LoadFile(file, wxBITMAP_TYPE_PNG))
            return false;
    }
    else
        return false;

    if (m_bases.GetCount())
        m_basesOk=true;

    return true;
}


void CslPanelMap::UpdateBases(const CslArrayCslBaseInfo& bases, const bool hasBases)
{
    wxUint32 i;
    wxRect rect;
    wxPoint point;
    CslBaseInfo *base;

    WX_CLEAR_ARRAY(m_bases);

    if (hasBases)
    {
        for (i=0; i<bases.GetCount(); i++)
        {
            base=new CslBaseInfo(*bases.Item(i));
            m_bases.Add(base);
        }
    }

    if (!IsShown())
        return;

    Refresh(m_background);
    //TODO optimise drawing?
    /*for (i=0; i<bases.GetCount(); i++)
    {
        wxPoint point=bases.Item(i)->m_point;
        wxRect rect(point.x-5, point.y-5, 10, 10);
        RefreshRect(rect);
    }*/
}

void CslPanelMap::OnPaint(wxPaintEvent& event)
{
    event.Skip();

    if (!m_ok)
        return;

    wxUint32 i;
    wxMemoryDC memDC;
    wxPoint origin=GetBitmapOrigin();

    wxPaintDC dc(this);
    PrepareDC(dc);

    memDC.SelectObjectAsSource(m_bitmap);
    dc.Blit(origin.x, origin.y, m_bitmap.GetWidth(), m_bitmap.GetHeight(), &memDC, 0, 0);
    memDC.SelectObjectAsSource(wxNullBitmap);

    for (i=0; i<m_bases.GetCount(); i++)
    {
        CslBaseInfo *base=m_bases.Item(i);
        dc.SetPen(wxPen(base->m_colour));
        dc.SetBrush(wxBrush(base->m_colour));
        dc.DrawCircle(origin.x+base->m_point.x, origin.y+base->m_point.y, 4);
    }
}
