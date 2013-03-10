/***************************************************************************
 *   Copyright (C) 2007-2013 by Glen Masgai                                *
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

#ifdef _MSC_VER
    #ifdef _UNICODE
        #ifdef _DEBUG
            #include "../../wx28/lib/vc_dll/mswud/wx/setup.h"
        #else
            #include "../../wx28/lib/vc_dll/mswu/wx/setup.h"
        #endif

        #ifdef _DEBUG
            #pragma comment(lib,"wxmsw28ud.lib")
        #else
            #pragma comment(lib,"wxmsw28u.lib")
        #endif
    #else
        #error "Ansi builds aren't supported."
    #endif
#else
    #error "This file should only be included when using Microsoft Visual C++"
#endif
