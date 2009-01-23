
#include "CslArt.h"

#include "img/csl_16.xpm"
#include "img/connect_16.xpm"
#include "img/connect_pw_16.xpm"
#include "img/filter_16.xpm"
#include "img/about_16.xpm"
#include "img/extinfo_micro_16.xpm"
#include "img/extinfo_mini_16.xpm"
#include "img/extinfo_default_16.xpm"
#include "img/flags/unknown.xpm"
#if defined(__WXMSW__) || defined(__WXMAC__)
#include "img/add_16.xpm"
#include "img/delete_16.xpm"
#include "img/reload_16.xpm"
#include "img/settings_16.xpm"
#endif // __WXMSW__ || __WXMAC__

wxBitmap CslArt::CreateBitmap(const wxArtID& id,const wxArtClient& client,const wxSize& size)
{
    if (id!=wxART_NONE)
    {
        if (client==wxART_MENU)
        {
            if (id==wxART_CSL)
                return wxBitmap(csl_16_xpm);
            else if (id==wxART_CONNECT)
                return wxBitmap(connect_16_xpm);
            else if (id==wxART_CONNECT_PW)
                return wxBitmap(connect_pw_16_xpm);
            else if (id==wxART_FILTER)
                return wxBitmap(filter_16_xpm);
            else if (id==wxART_ABOUT)
                return wxBitmap(about_16_xpm);
            else if (id==wxART_EXTINFO_MICRO)
                return wxBitmap(extinfo_micro_16_xpm);
            else if (id==wxART_EXTINFO_MINI)
                return wxBitmap(extinfo_mini_16_xpm);
            else if (id==wxART_EXTINFO_DEFAULT)
                return wxBitmap(extinfo_default_16_xpm);
            else if (id==wxART_COUNTRY_UNKNOWN)
                return wxBitmap(unknown_xpm);

#if defined(__WXMSW__) || defined(__WXMAC__)
            else if (id==wxART_ADD_BOOKMARK)
                return wxBitmap(add_16_xpm);
            else if (id==wxART_DEL_BOOKMARK)
                return wxBitmap(delete_16_xpm);
            else if (id==wxART_RELOAD)
                return wxBitmap(reload_16_xpm);
            else if (id==wxART_SETTINGS)
                return wxBitmap(settings_16_xpm);
#endif //__WXMSW__ || __WXMAC__
        }
    }
    return wxNullBitmap;
}
