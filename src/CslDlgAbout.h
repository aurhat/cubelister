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

#ifndef CSLDLGABOUT_H
#define CSLDLGABOUT_H

/**
    @author Glen Masgai <mimosius@users.sourceforge.net>
*/


class CslPanelAboutImage : public wxPanel
{
    public:
        CslPanelAboutImage(wxWindow *parent,wxInt32 id);
        ~CslPanelAboutImage();

    private:
        void OnPaint(wxPaintEvent& event);
        DECLARE_EVENT_TABLE();

    protected:
        wxBitmap *m_bitmap;
};


class CslDlgAbout: public wxDialog
{
    public:
        // begin wxGlade: CslDlgAbout::ids
        // end wxGlade
        CslDlgAbout(wxWindow* parent,int id=wxID_ANY,const wxString& title=wxEmptyString,
                    const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize,
                    long style=wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER);

    private:
        // begin wxGlade: CslDlgAbout::methods
        void set_properties();
        void do_layout();
        // end wxGlade

        void OnCommandEvent(wxCommandEvent& event);

        DECLARE_EVENT_TABLE();

    protected:
        // begin wxGlade: CslDlgAbout::attributes
        CslPanelAboutImage* panel_bitmap;
        wxStaticText* label_name;
        wxStaticText* label_version;
        wxStaticText* label_desc;
        wxHyperlinkCtrl* hyperlink_web;
        wxStaticText* label_copyright;
        wxStaticText* label_wxversion;
        wxTextCtrl* text_ctrl_credits;
        wxPanel* notebook_pane_credits;
        wxTextCtrl* text_ctrl_license;
        wxPanel* notebook_pane_license;
        wxNotebook* notebook;
        wxButton* button_close;
        // end wxGlade
}; // wxGlade: end class


#endif // CSLDLGABOUT_H
