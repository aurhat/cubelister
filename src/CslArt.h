
#ifndef CSLART_H
#define CSLART_H

/**
    @author Glen Masgai <mimosius@gmx.de>
*/

#include <wx/artprov.h>

#ifndef wxART_NONE
#define wxART_NONE             wxART_MAKE_ART_ID(wxART_NONE)
#endif
#ifndef wxART_CONNECT
#define wxART_CONNECT          wxART_MAKE_ART_ID(wxART_CONNECT)
#endif
#ifndef wxART_CONNECT_PW
#define wxART_CONNECT_PW       wxART_MAKE_ART_ID(wxART_CONNECT_PW)
#endif
#ifndef wxART_RELOAD
#define wxART_RELOAD           wxART_MAKE_ART_ID(wxART_RELOAD)
#endif
#ifndef wxART_FILTER
#define wxART_FILTER           wxART_MAKE_ART_ID(wxART_FILTER)
#endif
#ifndef wxART_SETTINGS
#define wxART_SETTINGS         wxART_MAKE_ART_ID(wxART_SETTINGS)
#endif
#ifndef wxART_ABOUT
#define wxART_ABOUT            wxART_MAKE_ART_ID(wxART_ABOUT)
#endif
#ifndef wxART_EXTINFO_MICRO
#define wxART_EXTINFO_MICRO    wxART_MAKE_ART_ID(wxART_EXTINFO_MICRO)
#endif
#ifndef wxART_EXTINFO_MINI
#define wxART_EXTINFO_MINI     wxART_MAKE_ART_ID(wxART_EXTINFO_MINI)
#endif
#ifndef wxART_EXTINFO_DEFAULT
#define wxART_EXTINFO_DEFAULT  wxART_MAKE_ART_ID(wxART_EXTINFO_DEFAULT)
#endif
#ifndef wxART_COUNTRY_UNKNOWN
#define wxART_COUNTRY_UNKNOWN  wxART_MAKE_ART_ID(wxART_COUNTRY_UNKNOWN)
#endif

class CslArt : public wxArtProvider
{
    public:
        static wxBitmap GetMenuBitmap(const wxArtID& id) { return wxArtProvider::GetBitmap(id,wxART_MENU); }

    protected:
        wxBitmap CreateBitmap(const wxArtID& id,
                              const wxArtClient& client,
                              const wxSize& size);
};

#define GET_ART_MENU(ART) CslArt::GetMenuBitmap(ART)

#endif // CSLART_H
