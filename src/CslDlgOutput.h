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
#if 0

#ifndef CSLDLGOUTPUT_H
#define CSLDLGOUTPUT_H

#include "wx/wxprec.h"
#ifdef __BORLANDC__
#pragma hdrstop
#endif
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif
#include <wx/image.h>


// begin wxGlade: ::dependencies
// end wxGlade


class CslDlgOutput: public wxDialog
{
    public:
        // begin wxGlade: CslDlgOutput::ids
        // end wxGlade

        CslDlgOutput(wxWindow* parent,int id=wxID_ANY,const wxString& title=wxEmptyString,
                     const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize,
                     long style=wxDEFAULT_DIALOG_STYLE);

    private:
        // begin wxGlade: CslDlgOutput::methods
        void set_properties();
        void do_layout();
        // end wxGlade

    protected:
        // begin wxGlade: CslDlgOutput::attributes
        wxTextCtrl* text_ctrl_output;
        wxCheckBox* checkbox_conversation;
        wxButton* button_save;
        wxButton* button_close;
        // end wxGlade
}; // wxGlade: end class


#endif // CSLDLGOUTPUT_H

#endif
