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

#ifndef CSLDLGABOUT_H
#define CSLDLGABOUT_H

class CslPanelAboutImage : public wxPanel
{
    public:
        CslPanelAboutImage(wxWindow *parent,wxInt32 id);
        ~CslPanelAboutImage();

    private:
        void OnPaint(wxPaintEvent& event);
        DECLARE_EVENT_TABLE();
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

        void OnClose(wxCloseEvent& event);
        void OnCommandEvent(wxCommandEvent& event);

        DECLARE_EVENT_TABLE();

    protected:
        // begin wxGlade: CslDlgAbout::attributes
        CslPanelAboutImage* panel_bitmap;
        wxStaticText* m_labelName;
        wxStaticText* m_labelVersion;
        wxStaticText* m_labelDescription;
        wxHyperlinkCtrl* m_hlWeb;
        wxStaticText* m_labelCopyright;
        wxStaticText* m_labelwxVersion;
        wxTextCtrl* m_tcCredits;
        wxPanel* m_npCredits;
        wxTextCtrl* m_tcLicense;
        wxPanel* m_npLicense;
        wxNotebook* m_notebook;
        wxButton* m_btClose;
        // end wxGlade
}; // wxGlade: end class


#endif // CSLDLGABOUT_H
