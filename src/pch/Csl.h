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

#ifndef CSL_H
#define CSL_H

/**
    @author Glen Masgai <mimosius@gmx.de>
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif //HAVE_CONFIG_H

#include <wx/wx.h>
#include <wx/artprov.h>
#include <wx/aui/aui.h>
#include <wx/clipbrd.h>
#include <wx/colordlg.h>
#include <wx/fileconf.h>
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/filepicker.h>
#include <wx/hyperlink.h>
#include <wx/imaglist.h>
#include <wx/ipc.h>
#include <wx/listbook.h>
#include <wx/listctrl.h>
#include <wx/mstream.h>
#include <wx/notebook.h>
#include <wx/numdlg.h>
#include <wx/process.h>
#include <wx/protocol/http.h>
#include <wx/regex.h>
#include <wx/snglinst.h>
#include <wx/socket.h>
#include <wx/spinctrl.h>
#include <wx/splitter.h>
#include <wx/statline.h>
#include <wx/stdpaths.h>
#include <wx/taskbar.h>
#include <wx/tokenzr.h>
#include <wx/treectrl.h>
#include <wx/uri.h>
#include <wx/wfstream.h>
#include <wx/wupdlock.h>
#include <wx/wizard.h>
#ifdef __WXMAC__
#include <wx/sysopt.h>
#endif //__WXMAC__

#include "../engine/CslTools.h"
#include "../engine/CslVersion.h"

#if wxCHECK_VERSION(2,9,0)
typedef wxIntPtr IntPtr;
#else
typedef long IntPtr;
#endif

#endif //CSL_H
