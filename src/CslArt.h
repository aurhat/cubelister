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

#ifndef CSLART_H
#define CSLART_H

/**
    @author Glen Masgai <mimosius@users.sourceforge.net>
*/

#ifndef wxART_NONE
#define wxART_NONE             wxART_MAKE_ART_ID(wxART_NONE)
#endif
#ifndef wxART_CSL
#define wxART_CSL              wxART_MAKE_ART_ID(wxART_CSL)
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
#ifndef wxART_GEO
#define wxART_GEO              wxART_MAKE_ART_ID(wxART_GEO)
#endif
#ifndef wxART_NOTIFICATION
#define wxART_NOTIFICATION     wxART_MAKE_ART_ID(wxART_NOTIFICATION)
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
