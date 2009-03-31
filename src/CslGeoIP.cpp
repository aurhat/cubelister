/***************************************************************************
 *   Copyright (C) 2007-2009 by Glen Masgai                                *
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
#include "CslGeoIP.h"


CslGeoIP::CslGeoIP()
{
#ifdef CSL_EXTERNAL_GEOIP_DATABASE
    m_geoIP=GeoIP_open_type(GEOIP_COUNTRY_EDITION,GEOIP_MEMORY_CACHE);
#else
    wxString path=DATAPATH+wxString(wxT("/GeoIP.dat"));
    m_geoIP=GeoIP_open(U2A(path),GEOIP_MEMORY_CACHE);
#ifdef __WXGTK__
    if (!m_geoIP)
    {
        path=::g_basePath+wxT("/data/GeoIP.dat");
        m_geoIP=GeoIP_open(U2A(path),GEOIP_MEMORY_CACHE);
    }
#endif //__WXGTK__
#endif //CSL_EXTERNAL_GEOIP_DATABASE
}

CslGeoIP::~CslGeoIP()
{
    if (!m_geoIP)
        return;

    GeoIP_delete(m_geoIP);
    m_geoIP=NULL;

    WX_CLEAR_ARRAY(m_services);
}

CslGeoIP& CslGeoIP::GetInstance()
{
    static CslGeoIP geoIP;

    return geoIP;
}

bool CslGeoIP::IsOk()
{
    return GetInstance().m_geoIP!=NULL;
}

const char* CslGeoIP::GetCountryCodeByAddr(const char *host)
{
    CslGeoIP& self=GetInstance();
    return self.m_geoIP ? GeoIP_country_code_by_addr(self.m_geoIP,host):NULL;
}

const char* CslGeoIP::GetCountryCodeByIPnum(const unsigned long ipnum)
{
    CslGeoIP& self=GetInstance();
    return self.m_geoIP ? GeoIP_country_code_by_ipnum(self.m_geoIP,ipnum):NULL;
}

const char* CslGeoIP::GetCountryNameByAddr(const char *host)
{
    CslGeoIP& self=GetInstance();
    return self.m_geoIP ? GeoIP_country_name_by_addr(self.m_geoIP,host):NULL;
}

const char* CslGeoIP::GetCountryNameByIPnum(const unsigned long ipnum)
{
    CslGeoIP& self=GetInstance();
    return self.m_geoIP ? GeoIP_country_name_by_ipnum(self.m_geoIP,ipnum):NULL;
}

void CslGeoIP::AddService(const wxString& name,const wxString& host,const wxString& path)
{
    GetInstance().m_services.Add(new CslGeoIPService(name,host,path));
}

const CslGeoIPServices& CslGeoIP::GetServices()
{
    return GetInstance().m_services;
}
