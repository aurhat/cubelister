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

#ifndef CSLGEOIP_H
#define CSLGEOIP_H

class CSL_DLL_GUITOOLS CslGeoIPService
{
    public:
        CslGeoIPService(const wxString& name, const wxString& host, const wxString& path) :
                Name(name), Host(host), Path(path) { }

        wxString Name, Host, Path;
};

WX_DEFINE_USER_EXPORTED_ARRAY(CslGeoIPService*, CslGeoIPServices, class CSL_DLL_GUITOOLS);


class CSL_DLL_GUITOOLS CslGeoIP
{
    private:
        CslGeoIP();
        ~CslGeoIP();

        static CslGeoIP& GetInstance()
        {
            static CslGeoIP geoip;
            return geoip;
        }

    public:
        enum
        {
            GEOIP_COUNTRY,
            GEOIP_CITY
        };

        static bool IsOk();
        static wxInt32 GetType();
        static bool Load(wxInt32 type);
        static void Unload();
        static wxString GetFileName();
        static wxURI GetUpdateURI(wxInt32 type = -1);
        static const char* GetCountryCodeByAddr(const char *host);
        static const char* GetCountryCodeByIPnum(const unsigned long ipnum);
        static wxString GetCountryNameByAddr(const char *host);
        static wxString GetCountryNameByIPnum(const unsigned long ipnum);
        static wxString GetCityNameByAddr(const char *host);
        static wxString GetCityNameByIPnum(const unsigned long ipnum);
        static const wxArrayString& GetCountryCodes();
        static const wxArrayString& GetCountryNames();

        static void AddService(const wxString& name, const wxString& host, const wxString& path);
        static const CslGeoIPServices& GetServices();

    private:
        wxInt32 m_type;
        wxString m_fileName;
        CslGeoIPServices m_services;
        wxArrayString m_countryCodes, m_countryNames;
};

#endif
