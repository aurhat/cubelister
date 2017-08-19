/***************************************************************************
 *   Copyright (C) 2007-2014 by Glen Masgai                                *
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

#define CSL_STRINGIFY(x) #x
#define CSL_TO_STRING(x) CSL_STRINGIFY(x)
#define CSL_VERSION_TO_STRING(a, b, c) CSL_TO_STRING(a.b.c)

#define __CSL_VERSION_MAJOR        0
#define __CSL_VERSION_MINOR        8
#define __CSL_VERSION_PATCH        2

#define __CSL_NAME_STR         "Cube Server Lister (CSL)"
#define __CSL_NAME_SHORT_STR   "CSL"
#define __CSL_VERSION          __CSL_VERSION_MAJOR, \
                               __CSL_VERSION_MINOR, \
                               __CSL_VERSION_PATCH

#define __CSL_VERSION_STR      CSL_VERSION_TO_STRING(__CSL_VERSION_MAJOR, \
                                                     __CSL_VERSION_MINOR, \
                                                     __CSL_VERSION_PATCH)
#define __CSL_COPYRIGHT_STR    "(C) 2007-2014, Glen Masgai <mimosius@users.sourceforge.net>"
#define __CSL_DESCRIPTION_STR  "Tool to monitor cubeengine-based servers."

#define CSL_NAME_STR           wxT(__CSL_NAME_STR)
#define CSL_NAME_SHORT_STR     wxT(__CSL_NAME_SHORT_STR)

#define CSL_VERSION_STR        wxT(__CSL_VERSION_STR)

#define CSL_DESCRIPTION_STR    _("Tool to monitor cubeengine-based servers.")
#define CSL_COPYRIGHT_STR      _(__CSL_DESCRIPTION_STR)

#define CSL_WEBADDR_STR        wxT("http://cubelister.sourceforge.net")

#ifdef UNICODE
    #if wxUSE_UNICODE_UTF8
        #define __CSL_ENCODING__   wxT(" (UTF-8)")
    #else
        #define __CSL_ENCODING__   wxT(" (Unicode)")
    #endif
#else
    #define __CSL_ENCODING__       wxT(" (ANSI)")
#endif
#if wxUSE_STL
    #define __CSL_STL__            wxT(" (STL)")
#else
    #define __CSL_STL__
#endif
#define CSL_WXVERSION_STR      wxString(wxVERSION_STRING __CSL_ENCODING__ __CSL_STL__)

#endif // CSL_VERSION_H
