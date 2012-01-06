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

#include <GeoIP.h>
#include <GeoIPCity.h>
#include "Csl.h"
#include "CslGeoIP.h"

static GeoIP *g_geoIP=NULL;

bool LoadDatabase(const wxString& name, GeoIPDBTypes type, GeoIPOptions options)
{
#ifdef CSL_EXTERNAL_GEOIP_DATABASE
    if (GeoIP_db_avail(type))
    {
        if ((g_geoIP=GeoIP_open_type(type, options)))
        {
            LOG_DEBUG("Loaded external database: %s\n", U2A(name));
            return true;
        }
    }
#else
    if (!name.IsEmpty())
    {
        wxString path=FindPackageFile(wxT("data/")+name);

        if (!path.IsEmpty())
        {
            if ((g_geoIP=GeoIP_open(U2A(path), options)))
            {
                LOG_DEBUG("Loaded GeoIP database: %s\n", U2A(path));
                return true;
            }
        }
    }
#endif //CSL_EXTERNAL_GEOIP_DATABASE

    return false;
}

CslGeoIP::CslGeoIP() : m_type(GEOIP_TYPE_UNKNOWN)
{
    if (LoadDatabase(wxT("GeoCity.dat"), GEOIP_CITY_EDITION_REV1, GEOIP_MEMORY_CACHE))
        m_type=GEOIP_TYPE_CITY;
    else if (LoadDatabase(wxT("GeoLiteCity.dat"), GEOIP_CITY_EDITION_REV1, GEOIP_MEMORY_CACHE))
        m_type=GEOIP_TYPE_CITY;
    else if (LoadDatabase(wxT("GeoIP.dat"), GEOIP_COUNTRY_EDITION, GEOIP_MEMORY_CACHE))
        m_type=GEOIP_TYPE_COUNTRY;
}

CslGeoIP::~CslGeoIP()
{
    if (!g_geoIP)
        return;

    GeoIP_delete(g_geoIP);
    g_geoIP=NULL;

    WX_CLEAR_ARRAY(m_services);
}

bool CslGeoIP::IsOk()
{
    return g_geoIP!=NULL;
}

const char* CslGeoIP::GetCountryCodeByAddr(const char *host)
{
    CslGeoIP& self=GetInstance();

    if (!self.m_type)
        return NULL;

    GeoIPRecord *r=NULL;
    const char *code=NULL;

    switch (self.m_type)
    {
        case GEOIP_TYPE_COUNTRY:
            code=GeoIP_country_code_by_addr(g_geoIP, host);
            break;
        case GEOIP_TYPE_CITY:
            if ((r=GeoIP_record_by_addr(g_geoIP, host)))
            {
                code=r->country_code;
                GeoIPRecord_delete(r);
            }
            break;
    }

    return code;
}

const char* CslGeoIP::GetCountryCodeByIPnum(const unsigned long ipnum)
{
    CslGeoIP& self=GetInstance();

    if (!self.m_type)
        return NULL;

    GeoIPRecord *r=NULL;
    const char *code=NULL;

    switch (self.m_type)
    {
        case GEOIP_TYPE_COUNTRY:
            code=GeoIP_country_code_by_ipnum(g_geoIP, wxUINT32_SWAP_ON_LE(ipnum));
            break;
        case GEOIP_TYPE_CITY:
            if ((r=GeoIP_record_by_ipnum(g_geoIP, wxUINT32_SWAP_ON_LE(ipnum))))
            {
                code=r->country_code;
                GeoIPRecord_delete(r);
            }
            break;
    }

    return code;
}

wxString CslGeoIP::GetCountryNameByAddr(const char *host)
{
    CslGeoIP& self=GetInstance();

    if (!self.m_type)
        return wxEmptyString;

    GeoIPRecord *r=NULL;
    const char *country=NULL;

    switch (self.m_type)
    {
        case GEOIP_TYPE_COUNTRY:
            country=GeoIP_country_name_by_addr(g_geoIP, host);
            break;
        case GEOIP_TYPE_CITY:
            if ((r=GeoIP_record_by_addr(g_geoIP, host)))
            {
                country=r->country_name;
                GeoIPRecord_delete(r);
            }
            break;
    }

    return country ? A2U(country) : wxString(wxEmptyString);
}

wxString CslGeoIP::GetCountryNameByIPnum(const unsigned long ipnum)
{
    CslGeoIP& self=GetInstance();

    if (!self.m_type)
        return wxEmptyString;

    GeoIPRecord *r=NULL;
    const char *country=NULL;

    switch (self.m_type)
    {
        case GEOIP_TYPE_COUNTRY:
            country=GeoIP_country_name_by_ipnum(g_geoIP, wxUINT32_SWAP_ON_LE(ipnum));
            break;
        case GEOIP_TYPE_CITY:
            if ((r=GeoIP_record_by_ipnum(g_geoIP, wxUINT32_SWAP_ON_LE(ipnum))))
            {
                country=r->country_name;
                GeoIPRecord_delete(r);
            }
            break;
    }

    return country ? A2U(country) : wxString(wxEmptyString);
}

wxString CslGeoIP::GetCityNameByAddr(const char *host)
{
    wxString city;
    CslGeoIP& self=GetInstance();

    if (self.m_type==GEOIP_TYPE_CITY)
    {
        GeoIPRecord *r=GeoIP_record_by_addr(g_geoIP, host);

        if (r)
        {
            city=A2U(r->city);
            GeoIPRecord_delete(r);
        }
    }

    return city;
}

wxString CslGeoIP::GetCityNameByIPnum(const unsigned long ipnum)
{
    wxString city;
    CslGeoIP& self=GetInstance();

    if (self.m_type==GEOIP_TYPE_CITY)
    {
        GeoIPRecord *r=GeoIP_record_by_ipnum(g_geoIP, wxUINT32_SWAP_ON_LE(ipnum));

        if (r)
        {
            city=A2U(r->city);
            GeoIPRecord_delete(r);
        }
    }

    return city;
}

void CslGeoIP::AddService(const wxString& name, const wxString& host, const wxString& path)
{
    GetInstance().m_services.Add(new CslGeoIPService(name, host, path));
}

const CslGeoIPServices& CslGeoIP::GetServices()
{
    return GetInstance().m_services;
}
