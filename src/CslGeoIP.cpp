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

#include <wx/wxprec.h>
#ifdef __BORLANDC__
#pragma hdrstop
#endif
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include "CslGeoIP.h"
#include "CslTools.h"


GeoIP *s_geoIP=NULL;

bool CslGeoIP::Init()
{
    if (s_geoIP)
    {
        wxASSERT_MSG(!s_geoIP,wxT("GeoIP already initialised!"));
        return false;
    }
#ifdef CSL_EXTERNAL_GEOIP_DATABASE
    s_geoIP=GeoIP_open_type(GEOIP_COUNTRY_EDITION,GEOIP_MEMORY_CACHE);
#else
    wxString path=DATAPATH+wxString(wxT("/GeoIP.dat"));
    s_geoIP=GeoIP_open(U2A(path),GEOIP_MEMORY_CACHE);
#ifdef __WXGTK__
    if (!s_geoIP)
    {
        path=::g_basePath+wxT("/data/GeoIP.dat");
        s_geoIP=GeoIP_open(U2A(path),GEOIP_MEMORY_CACHE);
    }
#endif //__WXGTK__
#endif //CSL_EXTERNAL_GEOIP_DATABASE

    return s_geoIP!=NULL;
}

void CslGeoIP::Destroy()
{
    if (!s_geoIP)
    {
        wxASSERT_MSG(!s_geoIP,wxT("GeoIP not initialised!"));
        return;
    }

    GeoIP_delete(s_geoIP);
    s_geoIP=NULL;
}

bool CslGeoIP::IsOk()
{
    return s_geoIP!=NULL;
}

const char* CslGeoIP::GetCountryCodeByAddr(const char *host)
{
    if (!s_geoIP)
    {
        wxASSERT_MSG(!s_geoIP,wxT("GeoIP not initialised!"));
        return NULL;
    }
    return GeoIP_country_code_by_addr(s_geoIP,host);
}

const char* CslGeoIP::GetCountryCodeByIPnum(const unsigned long ipnum)
{
    if (!s_geoIP)
    {
        wxASSERT_MSG(!s_geoIP,wxT("GeoIP not initialised!"));
        return NULL;
    }
    return GeoIP_country_code_by_ipnum(s_geoIP,ipnum);
}

const char* CslGeoIP::GetCountryNameByAddr(const char *host)
{
    if (!s_geoIP)
    {
        wxASSERT_MSG(!s_geoIP,wxT("GeoIP not initialised!"));
        return NULL;
    }
    return GeoIP_country_name_by_addr(s_geoIP,host);
}
