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

#ifndef CSLGEOIP_H
#define CSLGEOIP_H

/**
    @author Glen Masgai <mimosius@users.sourceforge.net>
*/

#ifdef __WXMSW__
#ifndef _WINSOCK2API_
#define _WINSOCK2API_
#endif //_WINSOCK2API_
#ifndef _WS2TCPIP_H_
#define _WS2TCPIP_H_
#endif //_WS2TCPIP_H_
#endif //__WXMSW__

class CslGeoIPService
{
    public:
        CslGeoIPService(const wxString& name, const wxString& host, const wxString& path) :
                Name(name), Host(host), Path(path) { }

        wxString Name, Host, Path;
};

WX_DEFINE_ARRAY_PTR(CslGeoIPService*, CslGeoIPServices);

class CslGeoIP
{
    private:
        CslGeoIP();
        ~CslGeoIP();

        static CslGeoIP& GetInstance()
        {
            static CslGeoIP geoIP;
            return geoIP;
        }

    public:
        enum { GEOIP_TYPE_UNKNOWN, GEOIP_TYPE_COUNTRY, GEOIP_TYPE_CITY};

        static bool IsOk();
        static const char* GetCountryCodeByAddr(const char *host);
        static const char* GetCountryCodeByIPnum(const unsigned long ipnum);
        static wxString GetCountryNameByAddr(const char *host);
        static wxString GetCountryNameByIPnum(const unsigned long ipnum);
        static wxString GetCityNameByAddr(const char *host);
        static wxString GetCityNameByIPnum(const unsigned long ipnum);
        static const vector<wxString>& GetCountryCodes();
        static const vector<wxString>& GetCountryNames();

        static void AddService(const wxString& name, const wxString& host, const wxString& path);
        static const CslGeoIPServices& GetServices();

    private:
        wxInt32 m_type;
        CslGeoIPServices m_services;
        vector<wxString> m_country_codes, m_country_names;
};

#endif
