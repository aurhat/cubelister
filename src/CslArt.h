
#ifndef CSLART_H
#define CSLART_H

/**
    @author Glen Masgai <mimosius@gmx.de>
*/

#include <wx/artprov.h>

#define CSL_ART_CONNECT        wxART_MAKE_ART_ID(CSL_ART_CONNECT)

#define GET_ART_MENU(ART) CslArt::GetMenuBitmap(ART)

class CslArt : public wxArtProvider
{
    public:
        static wxBitmap GetMenuBitmap(const wxArtID& id);

    protected:
        wxBitmap CreateBitmap(const wxArtID& id,const wxArtClient& client,const wxSize& size);
};

#endif // CSLART_H
