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

#ifndef CSL_H
#define CSL_H

/**
    @author Glen Masgai <mimosius@gmx.de>
*/

#ifdef _WINDOWS
#if defined(_DEBUG) && defined(_MSC_VER)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif // _DEBUG && _MSC_VER
#include <ws2tcpip.h>
#else
#include "config.h"
#endif //_WINDOWS
#include <wx/wx.h>
#if wxUSE_GUI
#include <wx/artprov.h>
#include <wx/aui/aui.h>
#include <wx/clipbrd.h>
#include <wx/colordlg.h>
#include <wx/datectrl.h>
#include <wx/dateevt.h>
#include <wx/filename.h>
#include <wx/filepicker.h>
#include <wx/hyperlink.h>
#include <wx/imaglist.h>
#include <wx/listbook.h>
#include <wx/listctrl.h>
#include <wx/notebook.h>
#include <wx/numdlg.h>
#include <wx/srchctrl.h>
#include <wx/spinctrl.h>
#include <wx/splitter.h>
#include <wx/statline.h>
#include <wx/taskbar.h>
#include <wx/treectrl.h>
#include <wx/wupdlock.h>
#include <wx/wizard.h>
#else
#if wxCHECK_VERSION(2, 9, 0)
#include <wx/evtloop.h>
#endif //wxCHECK_VERSION(2, 9, 0)
#endif //wxUSE_GUI
#include <wx/dir.h>
#include <wx/dynlib.h>
#include <wx/fileconf.h>
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/ipc.h>
#include <wx/mstream.h>
#include <wx/process.h>
#include <wx/protocol/http.h>
#include <wx/snglinst.h>
#include <wx/socket.h>
#include <wx/stdpaths.h>
#include <wx/tokenzr.h>
#include <wx/uri.h>
#include <wx/wfstream.h>
#ifdef __WXMAC__
#include <wx/sysopt.h>
#endif //__WXMAC__

#include <CslTools.h>
#include <CslVersion.h>
#if wxUSE_GUI
#include <CslGuiTools.h>
#endif //wxUSE_GUI

#endif //CSL_H
