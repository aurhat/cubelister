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

WX_DEFINE_ARRAY_INT(wxUint32, t_aUint);

class CslDlgOutput: public wxDialog
{
    public:
    // begin wxGlade: CslDlgOutput::ids
    // end wxGlade

        CslDlgOutput(wxWindow* parent,int id=wxID_ANY,const wxString& title=wxEmptyString,
                     const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize,
                     long style=wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER);

        static void AddOutput(char *text,wxUint32 size);
        static void Reset(const wxString& title);
        static void SaveFile(const wxString& path);

    private:
    // begin wxGlade: CslDlgOutput::methods
    void set_properties();
    void do_layout();
    // end wxGlade


        void OnClose(wxCloseEvent& event);
        void OnCommandEvent(wxCommandEvent& event);

        DECLARE_EVENT_TABLE()

    protected:
    // begin wxGlade: CslDlgOutput::attributes
    wxStaticBox* sizer_conv_filter_staticbox;
    wxStaticBox* sizer_search_staticbox;
    wxTextCtrl* text_ctrl_output;
    wxTextCtrl* text_ctrl_search;
    wxStaticText* label_matches;
    wxButton* button_search_prev;
    wxButton* button_search_next;
    wxCheckBox* checkbox_conv_filter;
    wxChoice* choice_conv_filter;
    wxButton* button_load;
    wxButton* button_save;
    wxButton* button_close;
    // end wxGlade

        static CslDlgOutput* m_self;
        wxTextAttr m_textDefaultStyle;

        wxString m_title;
        wxString m_text;
        wxUint32 m_filterLevel;
        t_aUint m_searchResults;
        wxUint32 m_searchPos;

        void HandleOutput(char *text,wxUint32 size);
        void SetSearchTextColour(wxUint32 pos,wxUint32 posOld);
        void SetSearchbarColour(wxInt32 count);
        wxInt32 Search(const wxString& needle);
        wxString Filter(wxUint32 start,wxUint32 end);
        void FixFilename(wxString *name);
}; // wxGlade: end class


#endif // CSLDLGOUTPUT_H
