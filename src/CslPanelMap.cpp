/***************************************************************************
 *   Copyright (C) 2007 by Glen Masgai                                     *
 *   mimosius@gmx.de                                                       *
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

#include <wx/wfstream.h>
#include "CslPanelMap.h"
#include "CslTools.h"

BEGIN_EVENT_TABLE(CslPanelMap, wxPanel)
    EVT_PAINT(CslPanelMap::OnPaint)
    EVT_ERASE_BACKGROUND(CslPanelMap::OnErase)
END_EVENT_TABLE()


bool CslMapInfo::GetMapConfigVersions(wxFileConfig& config,t_aInt32& array)
{
    wxUint32 i;
    long int index,val;
    wxString group;

    if (!config.GetFirstGroup(group,index))
        return false;

    do
    {
        if (!group.IsNumber())
            continue;
        if (!group.ToLong(&val))
            continue;

        for (i=0;i<array.GetCount();i++)
            if (array.Item(i)>val)
                break;
        array.Insert(val,i);
    }
    while (config.GetNextGroup(group,index));

    return array.GetCount()>0;
}

bool CslMapInfo::LoadMapConfig(wxFileConfig& config,const wxInt32 protVersion)
{
    long int val;
    wxInt32 x,y;
    wxUint32 i=0;

    config.SetPath(wxString::Format(wxT("/%d"),protVersion));
    if (!config.Read(wxT("Mapname"),&m_mapNameFull))
        return false;
    if (!config.Read(wxT("Author"),&m_author))
        return false;

    while (true)
    {
        if (!config.Read(wxString::Format(wxT("x%d"),i),&val))
            break;
        x=val;
        if (!config.Read(wxString::Format(wxT("y%d"),i),&val))
            break;
        y=val;

        m_bases.Add(new CslBaseInfo(x,y));
        i++;
    }

    m_version=protVersion;

    return true;
}

bool CslMapInfo::LoadMapData(const wxString& mapName,const wxString& gameName,
                             const wxInt32 protVersion)
{
    wxString path=wxString(wxT(DATADIR))+PATHDIV+wxT("maps")+PATHDIV+gameName+PATHDIV;
#ifdef __WXGTK__
    if (!::wxDirExists(path))
        path=::g_basePath+wxT("/data/maps/")+gameName+PATHDIV;
#endif
    Reset(mapName);

    if (::wxFileExists(path+mapName+wxT(".cfg")))
    {
        wxFileInputStream stream(path+mapName+wxT(".cfg"));
        if (!stream.IsOk())
            return false;

        wxFileConfig config(stream);

        wxInt32 version;
        t_aInt32 versions;

        if (!GetMapConfigVersions(config,versions))
            return false;

        if (versions.Index(protVersion)==wxNOT_FOUND)
            version=versions.Last();
        else
            version=protVersion;

        if (!LoadMapConfig(config,version))
            return false;

        path+=wxString::Format(wxT("%d/"),version)+mapName+wxT(".png");
        if (::wxFileExists(path))
        {
            if (!m_bitmap.LoadFile(path,wxBITMAP_TYPE_PNG))
                return false;
        }
        else
            return false;

        if (m_bases.GetCount())
            m_basesOk=true;

        return true;
    }

    return false;
}


void CslPanelMap::UpdateBases(const t_aBaseInfo& bases,const bool hasBases)
{
    wxUint32 i;
    wxRect rect;
    wxPoint point;
    CslBaseInfo *base;

    WX_CLEAR_ARRAY(m_bases);

    if (hasBases)
        for (i=0;i<bases.GetCount();i++)
        {
            base=new CslBaseInfo(*bases.Item(i));
            m_bases.Add(base);
        }

    if (!IsShown())
        return;

    Refresh(m_background);
    //TODO optimise drawing?
    /*for (i=0;i<bases.GetCount();i++)
    {
        wxPoint point=bases.Item(i)->m_point;
        wxRect rect(point.x-5,point.y-5,10,10);
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

    memDC.SelectObject(m_bitmap);
    dc.Blit(origin.x,origin.y,m_bitmap.GetWidth(),m_bitmap.GetHeight(),&memDC,0,0);

    for (i=0;i<m_bases.GetCount();i++)
    {
        CslBaseInfo *base=m_bases.Item(i);
        dc.SetPen(wxPen(base->m_colour));
        //dc.SetPen(wxPen(*wxWHITE));
        dc.SetBrush(wxBrush(base->m_colour));
        dc.DrawCircle(origin.x+base->m_point.x,origin.y+base->m_point.y,4);
    }
}
