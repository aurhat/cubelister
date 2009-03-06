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

#ifndef CSLGEOIP_H
#define CSLGEOIP_H

/**
    @author Glen Masgai <mimosius@users.sourceforge.net>
*/

#include "wx/wxprec.h"
#ifdef __BORLANDC__
#pragma hdrstop
#endif
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif
#include <GeoIP.h>


class CslGeoIPService
{
    public:
        CslGeoIPService(const wxString& name,const wxString& host,const wxString& path) :
                Name(name),Host(host),Path(path) {}

        wxString Name,Host,Path;
};

WX_DEFINE_ARRAY_PTR(CslGeoIPService*,CslGeoIPServices);

class CslGeoIP
{
    private:
        CslGeoIP();
        CslGeoIP(const CslGeoIP& geoip) {}
        ~CslGeoIP();

        static CslGeoIP& GetInstance();

    public:
        static bool IsOk();
        static const char* GetCountryCodeByAddr(const char *host);
        static const char* GetCountryCodeByIPnum(const unsigned long ipnum);
        static const char* GetCountryNameByAddr(const char *host);
        static const char* GetCountryNameByIPnum(const unsigned long ipnum);

        static void AddService(const wxString& name,const wxString& host,const wxString& path);
        static const CslGeoIPServices& GetServices();


    private:
        GeoIP *m_geoIP;
        CslGeoIPServices m_services;
};

#endif
