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

#ifndef CSLMAPCFGTOOL_H
#define CSLMAPCFGTOOL_H

/**
 @author Glen Masgai <mimosius@gmx.de>
*/

#include <wx/wx.h>
#include <wx/image.h>
// begin wxGlade: ::dependencies
#include <wx/statline.h>
// end wxGlade

// begin wxGlade: ::extracode

// end wxGlade


class CslMapCfgTool: public wxDialog
{
    public:
        // begin wxGlade: CslMapCfgTool::ids
        // end wxGlade

        CslMapCfgTool(wxWindow* parent,int id,const wxString& title,
                      const wxPoint& pos=wxDefaultPosition,
                      const wxSize& size=wxDefaultSize,
                      long style=wxDEFAULT_DIALOG_STYLE);

    private:
        // begin wxGlade: CslMapCfgTool::methods
        void set_properties();
        void do_layout();
        // end wxGlade

    protected:
        // begin wxGlade: CslMapCfgTool::attributes
        wxStaticBox* sizer_control_staticbox;
        wxStaticBox* sizer_map_staticbox;
        wxPanel* panel_bitmap;
        wxChoice* choice_base;
        wxButton* button_base_add;
        wxButton* button_base_del;
        wxStaticLine* static_line_1;
        wxTextCtrl* text_ctrl_map_name;
        wxTextCtrl* text_ctrl_author;
        wxStaticLine* static_line_2;
        wxButton* button_load_image;
        wxButton* button_reset;
        wxButton* button_save;
        wxButton* button_close;
        // end wxGlade
}; // wxGlade: end class


class CslMapCfgToolApp: public wxApp
{
    public:
        bool OnInit();
};

#endif // CSLMAPCFGTOOL_H
