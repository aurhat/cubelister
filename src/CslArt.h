
#ifndef CSLART_H
#define CSLART_H

/**
    @author Glen Masgai <mimosius@gmx.de>
*/

#include <wx/artprov.h>

#ifndef wxART_NONE
 #define wxART_NONE       wxART_MAKE_ART_ID(wxART_NONE)
#endif
#ifndef wxART_CONNECT
 #define wxART_CONNECT    wxART_MAKE_ART_ID(wxART_CONNECT)
#endif
#ifndef wxART_RELOAD
 #define wxART_RELOAD     wxART_MAKE_ART_ID(wxART_RELOAD)
#endif
#ifndef wxART_SETTINGS
 #define wxART_SETTINGS   wxART_MAKE_ART_ID(wxART_SETTINGS)
#endif
#ifndef wxART_ABOUT
 #define wxART_ABOUT      wxART_MAKE_ART_ID(wxART_ABOUT)
#endif

class CslArt : public wxArtProvider
{
    public:
        static wxBitmap GetMenuBitmap(const wxArtID& id);

    protected:
        wxBitmap CreateBitmap(const wxArtID& id,const wxArtClient& client,
                              const wxSize& size);
};

#define GET_ART_MENU(ART) CslArt::GetMenuBitmap(ART)

#endif // CSLART_H
