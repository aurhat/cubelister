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

#ifndef CSLDYNLIB_H
#define CSLDYNLIB_H

/**
    @author Glen Masgai <mimosius@users.sourceforge.net>
*/

#ifdef CSL_DLL_BUILD
#    ifdef CSL_TOOLS_EXPORTS
#        define CSL_DLL_TOOLS  WXEXPORT
#    else
#        define CSL_DLL_TOOLS  WXIMPORT
#    endif
#    ifdef CSL_PLUGIN_EXPORTS
#        define CSL_DLL_PLUGIN  WXEXPORT
#    else
#        define CSL_DLL_PLUGIN  WXIMPORT
#    endif
#    ifdef CSL_ENGINE_EXPORTS
#        define CSL_DLL_ENGINE  WXEXPORT
#    else
#        define CSL_DLL_ENGINE  WXIMPORT
#    endif
#    ifdef CSL_GUITOOLS_EXPORTS
#        define CSL_DLL_GUITOOLS  WXEXPORT
#    else
#        define CSL_DLL_GUITOOLS  WXIMPORT
#    endif
#    ifdef CSL_IRCENGINE_EXPORTS
#        define CSL_DLL_IRCENGINE  WXEXPORT
#    else
#        define CSL_DLL_IRCENGINE  WXIMPORT
#    endif
#    ifdef CSL_PLUGINS_EXPORT
#        define CSL_DLL_PLUGINS  WXEXPORT
#    else
#        define CSL_DLL_PLUGINS  WXIMPORT
#    endif
#else
#    define CSL_DLL_TOOLS
#    define CSL_DLL_PLUGIN
#    define CSL_DLL_ENGINE
#    define CSL_DLL_GUITOOLS
#    define CSL_DLL_IRCENGINE
#    define CSL_DLL_PLUGINS
#endif

#endif //CSLDYNLIB_H
