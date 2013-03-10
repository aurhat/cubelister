
#include "Csl.h"
#include "CslArt.h"
// small (menu)
#include "img/about_16.xpm"
#include "img/close_14.xpm"
#include "img/close_high_14.xpm"
#include "img/close_press_14.xpm"
#include "img/connect_16.xpm"
#include "img/connect_pw_16.xpm"
#include "img/csl_16_png.h"
#include "img/extinfo_micro_16.xpm"
#include "img/extinfo_mini_16.xpm"
#include "img/extinfo_default_16.xpm"
#include "img/filter_16.xpm"
#include "img/geo_16.xpm"
#include "img/notification_16.xpm"
#include "img/grey_list_16.xpm"
#include "img/green_list_16.xpm"
#include "img/red_list_16.xpm"
#include "img/yellow_list_16.xpm"
#include "img/flags/unknown.xpm"
#if defined(__WXMSW__) || defined(__WXMAC__)
#include "img/add_16.xpm"
#include "img/delete_16.xpm"
#include "img/reload_16.xpm"
#include "img/settings_16.xpm"
#endif // __WXMSW__ || __WXMAC__
// medium (toolbar)
#include "img/csl_24_png.h"
#include "img/master_24.xpm"
// big (frame)
#include "img/csl_48_png.h"

wxBitmap CslArt::CreateBitmap(const wxArtID& id,
                              const wxArtClient& client,
                              const wxSize& WXUNUSED(size))
{
    if (id!=wxART_NONE)
    {
        if (client==wxART_MENU)
        {
            if (id==wxART_ABOUT)
                return wxBitmap(about_16_xpm);
            else if (id==wxART_CSL)
                return BitmapFromData(wxBITMAP_TYPE_PNG, csl_16_png);
            else if (id==wxART_CLOSE)
                return wxBitmap(close_14_xpm);
            else if (id==wxART_CLOSE_HOVER)
                return wxBitmap(close_high_14_xpm);
            else if (id==wxART_CLOSE_PRESSED)
                return wxBitmap(close_press_14_xpm);
            else if (id==wxART_CONNECT)
                return wxBitmap(connect_16_xpm);
            else if (id==wxART_CONNECT_PW)
                return wxBitmap(connect_pw_16_xpm);
            else if (id==wxART_FILTER)
                return wxBitmap(filter_16_xpm);
            else if (id==wxART_GEO)
                return wxBitmap(geo_16_xpm);
            else if (id==wxART_EXTINFO_MICRO)
                return wxBitmap(extinfo_micro_16_xpm);
            else if (id==wxART_EXTINFO_MINI)
                return wxBitmap(extinfo_mini_16_xpm);
            else if (id==wxART_EXTINFO_DEFAULT)
                return wxBitmap(extinfo_default_16_xpm);
            else if (id==wxART_NOTIFICATION)
                return wxBitmap(notification_16_xpm);
            else if (id==wxART_BUBBLE_GREY)
                return wxBitmap(grey_list_16_xpm);
            else if (id==wxART_BUBBLE_GREEN)
                return wxBitmap(green_list_16_xpm);
            else if (id==wxART_BUBBLE_YELLOW)
                return wxBitmap(yellow_list_16_xpm);
            else if (id==wxART_BUBBLE_RED)
                return wxBitmap(red_list_16_xpm);
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
        else if (client==wxART_TOOLBAR)
        {
            if (id==wxART_CSL)
                return BitmapFromData(wxBITMAP_TYPE_PNG, csl_24_png);
            else if (id==wxART_MASTER)
                return wxBitmap(master_24_xpm);
        }
        else if (client==wxART_FRAME_ICON)
        {
            if (id==wxART_CSL)
                return BitmapFromData(wxBITMAP_TYPE_PNG, csl_48_png);
        }
    }

    return wxNullBitmap;
}
