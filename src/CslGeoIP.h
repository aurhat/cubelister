
#ifndef CSLGEOIP_H
#define CSLGEOIP_H

/**
    @author Glen Masgai <mimosius@gmx.de>
*/

#include <GeoIP.h>

class CslGeoIP
{
    public:
        static bool Init();
        static void Destroy();
        static bool IsOk();
        static const char* GetCountryCodeByAddr(const char *host);
        static const char* GetCountryCodeByIPnum(const unsigned long ipnum);
        static const char* GetCountryNameByAddr(const char *host);

};

#endif
