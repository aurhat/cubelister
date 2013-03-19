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

#ifndef CSLPANELSEARCH_H
#define CSLPANELSEARCH_H

// begin wxGlade: ::extracode

// end wxGlade


class CslPanelSearch: public wxPanel {
    public:
        // begin wxGlade: CslPanelSearch::ids
        // end wxGlade

        CslPanelSearch(wxWindow* parent, int id = wxID_ANY,
                       const wxPoint& pos = wxDefaultPosition,
                       const wxSize& size = wxDefaultSize,
                       long style=0);

        bool IsServerSearch() const
            { return m_radio_server->GetValue(); }
        bool IsPlayerSearch() const
            { return m_radio_player->GetValue(); }

        wxString GetSearchText() const
            { return m_search_ctrl->GetValue(); }

        wxInt32 GetProgress() const
            { return m_gauge_progress->GetValue(); }

        void IncrementProgress()
            { SetProgress(GetProgress() + 1); }
        void SetProgress(wxInt32 value)
            { m_gauge_progress->SetValue(value); }
        void SetProgressRange(wxInt32 value)
            { m_gauge_progress->SetRange(value); }

        void SetResult(const wxString& text, const wxString& prefix = wxEmptyString);

        void SetSearchCtrlError(bool error)
            { ::SetSearchCtrlErrorState(m_search_ctrl, error); }

        // begin wxGlade: CslPanelSearch::attributes
        wxSearchCtrl* m_search_ctrl;
        wxRadioButton* m_radio_server;
        wxRadioButton* m_radio_player;
        wxGauge* m_gauge_progress;
        wxStaticText* m_label_result_prefix;
        wxStaticText* m_label_result;
        wxButton* m_button_search;
        // end wxGlade

        wxFlexGridSizer *m_sizer_result;

    private:
        // begin wxGlade: CslPanelSearch::methods
        void set_properties();
        void do_layout();
        // end wxGlade

        void OnKeypress(wxKeyEvent& event);
        void OnCommandEvent(wxCommandEvent& event);

        DECLARE_EVENT_TABLE()
}; // wxGlade: end class


#endif // CSLPANELSEARCH_H
