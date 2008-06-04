
/***************************************************************************
 *   Copyright (C) 2007 by Glen Masgai                                     *
 *   mimosius@gmx.de                                                       *
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

#include "CslMenu.h"

wxMenuBar* CslMenu::m_menuBar=NULL;

CslMenu::CslMenu(wxMenuBar *menuBar)
{
    m_menuBar=menuBar;

    EnableMenuItem(MENU_MASTER_ADD,false);
    EnableMenuItem(MENU_MASTER_DEL,false);
    EnableMenuItem(MENU_MASTER_UPDATE,false);

    CheckMenuItem(MENU_VIEW_FILTER,g_cslSettings->m_showFilter);
    CheckMenuItem(MENU_VIEW_AUTO_FIT,g_cslSettings->m_autoFitColumns);
    CheckMenuItem(MENU_VIEW_AUTO_SORT,g_cslSettings->m_autoSortColumns);
}

void CslMenu::EnableMenuItem(const wxInt32 id,const bool enable)
{
    m_menuBar->Enable(id,enable);
}

void CslMenu::CheckMenuItem(const wxInt32 id,const bool check)
{
    m_menuBar->Check(id,check);
}

wxMenuItem& CslMenu::AddItemToMenu(wxMenu *menu,const wxInt32 id,
                                   const wxString& text,const wxArtID& art,
                                   const wxItemKind kind,const wxString& help)
{
    wxMenuItem *item=new wxMenuItem(menu,id,text,help,kind);
    wxOperatingSystemId os=wxPlatformInfo().GetOperatingSystemId();
    if (id>wxID_HIGHEST || (os&wxOS_WINDOWS) || (os&wxOS_MAC))
        item->SetBitmap(GET_ART_MENU(art));
    menu->Append(item);

    return *item;
}
