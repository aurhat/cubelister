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

#include "Csl.h"
#include "CslGeoIP.h"
#include <GeoIP.h>
#include <GeoIPCity.h>

static GeoIP *g_geoIP = NULL;

wxString LoadDatabase(const wxString& path, GeoIPDBTypes type, GeoIPOptions options)
{
    if (::wxFileExists(path) && (g_geoIP = GeoIP_open(U2C(path), options)))
    {
        CSL_LOG_DEBUG("loaded GeoIP database: %s\n", U2C(path));
        g_geoIP->charset = GEOIP_CHARSET_UTF8;
    }

    return g_geoIP ? path : wxT("");
}

CslGeoIP::CslGeoIP() : m_type(GEOIP_COUNTRY)
{
    for (wxInt32 i = 0;; i++)
    {
        const char *code = GeoIP_code_by_id(i);
        const char *name = GeoIP_name_by_id(i);

        if (!code || !name)
            break;

        m_countryCodes.push_back(C2U(code));
        m_countryNames.push_back(C2U(name));
    }
}

CslGeoIP::~CslGeoIP()
{
    Unload();

    WX_CLEAR_ARRAY(m_services);
}

bool CslGeoIP::IsOk()
{
    return g_geoIP!=NULL;
}

wxInt32 CslGeoIP::GetType()
{
    CslGeoIP& self = GetInstance();
    return self.m_type;
}

bool CslGeoIP::Load(wxInt32 type)
{
    CslGeoIP& self = GetInstance();

    Unload();

    switch (type)
    {
        case GEOIP_COUNTRY:
            self.m_fileName = LoadDatabase(FindPackageFile(wxT("data/GeoIP.dat")),
                                           GEOIP_COUNTRY_EDITION, GEOIP_MEMORY_CACHE);
            self.m_type = GEOIP_COUNTRY;
            break;

        case GEOIP_CITY:
            if ((self.m_fileName = LoadDatabase(FindPackageFile(wxT("data/GeoCity.dat")),
                                                GEOIP_CITY_EDITION_REV1, GEOIP_STANDARD)).IsEmpty())
                self.m_fileName = LoadDatabase(FindPackageFile(wxT("data/GeoLiteCity.dat")),
                                               GEOIP_CITY_EDITION_REV1, GEOIP_STANDARD);
                self.m_type = GEOIP_CITY;
            break;
    }

    return g_geoIP!=NULL;
}

void CslGeoIP::Unload()
{
    if (g_geoIP)
    {
        GeoIP_delete(g_geoIP);
        g_geoIP = NULL;
    }
}

wxString CslGeoIP::GetFileName()
{
    CslGeoIP& self = GetInstance();
    return self.m_fileName;
}

wxURI CslGeoIP::GetUpdateURI(wxInt32 type)
{
    static const wxChar* uris[] =
    {
        wxT("http://geolite.maxmind.com/download/geoip/database/GeoLiteCountry/GeoIP.dat.gz"),
        wxT("http://geolite.maxmind.com/download/geoip/database/GeoLiteCity.dat.gz")
    };

    wxInt32 t = type < 0 ? GetType() : clamp(type, 0, 1);

    return wxString(uris[t]);
}

const char* CslGeoIP::GetCountryCodeByAddr(const char *host)
{
    CslGeoIP& self = GetInstance();

    if (!g_geoIP)
        return NULL;

    GeoIPRecord *r = NULL;
    const char *code = NULL;

    switch (self.m_type)
    {
        case GEOIP_COUNTRY:
            code = GeoIP_country_code_by_addr(g_geoIP, host);
            break;
        case GEOIP_CITY:
            if ((r = GeoIP_record_by_addr(g_geoIP, host)))
            {
                code = r->country_code;
                GeoIPRecord_delete(r);
            }
            break;
    }

    return code;
}

const char* CslGeoIP::GetCountryCodeByIPnum(const unsigned long ipnum)
{
    CslGeoIP& self = GetInstance();

    if (!g_geoIP)
        return NULL;

    GeoIPRecord *r = NULL;
    const char *code = NULL;

    switch (self.m_type)
    {
        case GEOIP_COUNTRY:
            code = GeoIP_country_code_by_ipnum(g_geoIP, wxUINT32_SWAP_ON_LE(ipnum));
            break;
        case GEOIP_CITY:
            if ((r = GeoIP_record_by_ipnum(g_geoIP, wxUINT32_SWAP_ON_LE(ipnum))))
            {
                code = r->country_code;
                GeoIPRecord_delete(r);
            }
            break;
    }

    return code;
}

wxString CslGeoIP::GetCountryNameByAddr(const char *host)
{
    CslGeoIP& self = GetInstance();

    if (!g_geoIP)
        return wxEmptyString;

    GeoIPRecord *r = NULL;
    const char *country = NULL;

    switch (self.m_type)
    {
        case GEOIP_COUNTRY:
            country = GeoIP_country_name_by_addr(g_geoIP, host);
            break;
        case GEOIP_CITY:
            if ((r = GeoIP_record_by_addr(g_geoIP, host)))
            {
                country = r->country_name;
                GeoIPRecord_delete(r);
            }
            break;
    }

    return country ? C2U(country) : wxString(wxEmptyString);
}

wxString CslGeoIP::GetCountryNameByIPnum(const unsigned long ipnum)
{
    CslGeoIP& self = GetInstance();

    if (!g_geoIP)
        return wxEmptyString;

    GeoIPRecord *r = NULL;
    const char *country = NULL;

    switch (self.m_type)
    {
        case GEOIP_COUNTRY:
            country = GeoIP_country_name_by_ipnum(g_geoIP, wxUINT32_SWAP_ON_LE(ipnum));
            break;
        case GEOIP_CITY:
            if ((r = GeoIP_record_by_ipnum(g_geoIP, wxUINT32_SWAP_ON_LE(ipnum))))
            {
                country = r->country_name;
                GeoIPRecord_delete(r);
            }
            break;
    }

    return country ? C2U(country) : wxString(wxEmptyString);
}

wxString CslGeoIP::GetCityNameByAddr(const char *host)
{
    CslGeoIP& self = GetInstance();

    wxString city;

    if (g_geoIP && self.m_type==GEOIP_CITY)
    {
        GeoIPRecord *r = GeoIP_record_by_addr(g_geoIP, host);

        if (r)
        {
            city = C2U(r->city);
            GeoIPRecord_delete(r);
        }
    }

    return city;
}

wxString CslGeoIP::GetCityNameByIPnum(const unsigned long ipnum)
{
    CslGeoIP& self = GetInstance();

    wxString city;

    if (g_geoIP && self.m_type==GEOIP_CITY)
    {
        GeoIPRecord *r = GeoIP_record_by_ipnum(g_geoIP, wxUINT32_SWAP_ON_LE(ipnum));

        if (r)
        {
            city = C2U(r->city);
            GeoIPRecord_delete(r);
        }
    }

    return city;
}

const wxArrayString& CslGeoIP::GetCountryCodes()
{
    CslGeoIP& self = GetInstance();

    return self.m_countryCodes;
}

const wxArrayString& CslGeoIP::GetCountryNames()
{
    CslGeoIP& self = GetInstance();

    return self.m_countryNames;
}

void CslGeoIP::AddService(const wxString& name, const wxString& host, const wxString& path)
{
    GetInstance().m_services.Add(new CslGeoIPService(name, host, path));
}

const CslGeoIPServices& CslGeoIP::GetServices()
{
    return GetInstance().m_services;
}
