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

#ifndef CSL_VERSION_H
#define CSL_VERSION_H

/**
 @author Glen Masgai <mimosius@users.sourceforge.net>
*/

#define __CSL_NAME_STR          "Cube Server Lister (CSL)"
#define __CSL_NAME_SHORT_STR    "csl"

#define __CSL_VERSION           0,8,1,75
#define __CSL_VERSION_STR       "0.8.1.75"
#define __CSL_VERSION_ADD_STR   ""

#define __CSL_COPYRIGHT_STR     "(C) 2007-2009, Glen Masgai <mimosius@users.sourceforge.net>"

#define CSL_NAME_STR            wxT(__CSL_NAME_STR)
#define CSL_NAME_SHORT_STR      wxT(__CSL_NAME_SHORT_STR)

#define CSL_VERSION_STR         wxT(__CSL_VERSION_STR)
#define CSL_VERSION_ADD_STR     wxT(__CSL_VERSION_ADD_STR)
#define CSL_VERSION_LONG_STR    wxString(CSL_VERSION_STR wxT(" ") CSL_VERSION_ADD_STR)

#define CSL_DESCRIPTION_STR     _("Tool to monitor cubeengine-based servers.")
#define CSL_COPYRIGHT_STR       wxT(__CSL_COPYRIGHT_STR)

#define CSL_WEBADDR_STR         wxT("cubelister.sourceforge.net")
#define CSL_WEBADDRFULL_STR     wxString(wxT("http://") CSL_WEBADDR_STR)

#ifdef UNICODE
#define __CSL_EMCODING__        wxT(" (Unicode)")
#else
#define __CSL_EMCODING__        wxT(" (ANSI)")
#endif
#define CSL_WXVERSION_STR       wxString(wxVERSION_STRING __CSL_EMCODING__)

#endif // CSL_VERSION_H
