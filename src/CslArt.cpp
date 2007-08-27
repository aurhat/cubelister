
#include "CslArt.h"

#ifdef __WXMSW__
#endif // __WXMSW__

#include "img/connect_16.xpm"

wxBitmap CslArt::CreateBitmap(const wxArtID& id,const wxArtClient& client,const wxSize& size)
{
    if (client==wxART_MENU)
    {
        if (id==CSL_ART_CONNECT)
            return wxBitmap(connect_16_xpm);
#ifdef __WXMSW__
#endif //__WXMSW__
    }
    return wxNullBitmap;
}

wxBitmap CslArt::GetMenuBitmap(const wxArtID& id)
{
    return wxArtProvider::GetBitmap(id,wxART_MENU);
}
