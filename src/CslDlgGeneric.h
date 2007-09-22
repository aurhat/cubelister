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

#ifndef CSLDLGGENERIC_H
#define CSLDLGGENERIC_H

/**
 @author Glen Masgai <mimosius@gmx.de>
*/

#include <wx/wx.h>
#include <wx/image.h>


class CslDlgGeneric: public wxDialog
{
    public:

        CslDlgGeneric(wxWindow *parent,const wxString& title,const wxString& text,
                      const wxBitmap& bitmap,const wxPoint& pos,
                      const long style=wxDEFAULT_DIALOG_STYLE) :
                wxDialog(parent,wxID_ANY,title,pos,wxDefaultSize,style)
        {
            wxFlexGridSizer* grid_sizer_main=new wxFlexGridSizer(1,2,0,0);
            grid_sizer_main->Add(new wxStaticBitmap(this,wxID_ANY,bitmap),0,wxALL,8);
            grid_sizer_main->Add(new wxStaticText(this,wxID_ANY,text),0,
                                 wxTOP|wxBOTTOM|wxRIGHT|wxALIGN_CENTER_VERTICAL,8);
            SetSizer(grid_sizer_main);
            grid_sizer_main->Fit(this);
            Layout();
            CentreOnParent();
        }
};

#endif // CSLDLGGENERIC_H
