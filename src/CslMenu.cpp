
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

#include "Csl.h"
#include "CslArt.h"
#include "CslSettings.h"
#include "CslMenu.h"


CslMenu::CslMenu() : m_mainMenu(NULL)
{
}

CslMenu& CslMenu::GetInstance()
{
    static CslMenu menu;
    return menu;
}

void CslMenu::SetMainMenu(wxMenuBar *menu)
{
    CslMenu& self=GetInstance();
    self.m_mainMenu=menu;
}

void CslMenu::EnableItem(wxInt32 id,bool enable)
{
    CslMenu& self=GetInstance();

    if (self.m_mainMenu)
        self.m_mainMenu->Enable(id,enable);
}

void CslMenu::EnableItem(wxMenu& menu,wxInt32 id,bool enable)
{
    wxMenuItem *item;

    if ((item=menu.FindItem(id)))
        item->Enable(enable);
}

void CslMenu::CheckItem(wxInt32 id,bool check)
{
    CslMenu& self=GetInstance();

    if (self.m_mainMenu)
        self.m_mainMenu->Check(id,check);
}

void CslMenu::CheckItem(wxMenu& menu,wxInt32 id,bool check)
{
    wxMenuItem *item;

    if ((item=menu.FindItem(id)))
        item->Check(check);
}

wxMenuItem& CslMenu::AddItem(wxMenu *menu,const wxInt32 id,
                             const wxString& text,const wxArtID& art,
                             const wxItemKind kind,const wxString& help)
{
    wxMenuItem *item=new wxMenuItem(menu,id,text,help,kind);

    wxOperatingSystemId os=wxPlatformInfo().GetOperatingSystemId();
    if (art!=wxART_NONE && (id>wxID_HIGHEST || (os&wxOS_WINDOWS) || (os&wxOS_MAC)))
        item->SetBitmap(GET_ART_MENU(art));

    menu->Append(item);

    return *item;
}
