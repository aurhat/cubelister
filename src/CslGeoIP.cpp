
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

    wxString path=wxT(DATADIR)+wxString(wxT("/GeoIP.dat"));
    s_geoIP=GeoIP_open(U2A(path),GEOIP_MEMORY_CACHE);
#ifdef __WXGTK__
    if (!s_geoIP)
    {
        path=::g_basePath+wxT("/data/GeoIP.dat");
        s_geoIP=GeoIP_open(U2A(path),GEOIP_MEMORY_CACHE);
    }
#endif
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
