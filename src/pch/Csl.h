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

#ifndef CSL_H
#define CSL_H

#ifdef _WINDOWS
#include <ws2tcpip.h>
#endif //_WINDOWS
#include <wx/wx.h>
#ifdef _WINDOWS
#include <wx/msw/msvcrt.h>
#endif
#if wxUSE_GUI
#include <wx/artprov.h>
#include <wx/aui/aui.h>
#include <wx/clipbrd.h>
#include <wx/colordlg.h>
#include <wx/datectrl.h>
#include <wx/dateevt.h>
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
#else
#if wxCHECK_VERSION(2, 9, 0)
#include <wx/evtloop.h>
#endif //wxCHECK_VERSION(2, 9, 0)
#endif //wxUSE_GUI
#include <wx/dir.h>
#include <wx/dynlib.h>
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/socket.h>

#include <CslTools.h>
#include <CslVersion.h>
#if wxUSE_GUI
#include <CslGuiTools.h>
#endif //wxUSE_GUI

#endif //CSL_H
