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

#ifndef CSLDLGGENERIC_H
#define CSLDLGGENERIC_H

#define CSL_DLG_GENERIC_DEFAULT     0
#define CSL_DLG_GENERIC_RESIZE   1<<0
#define CSL_DLG_GENERIC_OK       1<<1
#define CSL_DLG_GENERIC_CLOSE    1<<2
#define CSL_DLG_GENERIC_URL      1<<3
#define CSL_DLG_GENERIC_TEXT     1<<4


class CslDlgGeneric: public wxDialog
{
    public:

        CslDlgGeneric(wxWindow *parent,wxUint32 type,
                      const wxString& title,
                      const wxString& text,
                      const wxBitmap& bitmap,
                      const wxString& url=wxEmptyString) :
                wxDialog(parent,wxID_ANY,title,wxDefaultPosition,wxDefaultSize,
                         wxDEFAULT_DIALOG_STYLE|(type&CSL_DLG_GENERIC_RESIZE ? wxRESIZE_BORDER : 0))
        {
            wxUint32 rowsMain=1,rowsRight=1;
            if (type&CSL_DLG_GENERIC_URL) rowsRight++;
            if (type&CSL_DLG_GENERIC_CLOSE) rowsMain++;

            wxFlexGridSizer* grid_sizer_main=new wxFlexGridSizer(rowsMain,1,0,0);
            wxFlexGridSizer* grid_sizer_top=new wxFlexGridSizer(1,2,0,0);
            wxFlexGridSizer* grid_sizer_right=new wxFlexGridSizer(rowsRight,1,0,0);

            wxWindow *window;
            if (type&CSL_DLG_GENERIC_TEXT)
            {
                window=new wxTextCtrl(this,wxID_ANY,text,wxDefaultPosition,wxSize(450,230),
                                      wxTE_MULTILINE|wxTE_READONLY|wxTE_AUTO_URL|wxTE_RICH|wxTE_RICH2);

                if (!text.IsEmpty())
                {
                    wxSize size=GetBestWindowSizeForText(window,text,200,600,100,300,wxVERTICAL);
                    window->SetMinSize(size);
                }
            }
            else
                window=new wxStaticText(this,wxID_ANY,text);

            grid_sizer_top->Add(new wxStaticBitmap(this,wxID_ANY,bitmap),0,wxLEFT|wxTOP|wxBOTTOM,12);
            grid_sizer_top->Add(grid_sizer_right,1,wxEXPAND|wxTOP|wxBOTTOM,4);
            grid_sizer_right->Add(window,0,wxEXPAND|wxALL,10);
            grid_sizer_main->Add(grid_sizer_top,1,wxEXPAND);

            if (type&CSL_DLG_GENERIC_URL)
                grid_sizer_right->Add(new wxHyperlinkCtrl(this,wxID_ANY,url,url),0,
                                      wxALIGN_CENTER_HORIZONTAL|wxLEFT|wxRIGHT|wxBOTTOM,10);

            if (type&CSL_DLG_GENERIC_OK || type&CSL_DLG_GENERIC_CLOSE)
            {
                wxButton *button=new wxButton(this,type&CSL_DLG_GENERIC_OK ? wxID_OK : wxID_CLOSE);

                grid_sizer_main->Add(button,0,wxALIGN_RIGHT|wxLEFT|wxRIGHT|wxBOTTOM,8);
                Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                        wxCommandEventHandler(CslDlgGeneric::OnCommandEvent),
                        NULL,this);
                button->SetDefault();
            }

            grid_sizer_right->AddGrowableRow(0);
            grid_sizer_right->AddGrowableCol(0);
            grid_sizer_top->AddGrowableRow(0);
            grid_sizer_top->AddGrowableCol(1);
            grid_sizer_main->AddGrowableRow(0);
            grid_sizer_main->AddGrowableCol(0);

            SetSizer(grid_sizer_main);
            grid_sizer_main->Fit(this);
            Layout();
            grid_sizer_main->SetSizeHints(this);

            CentreOnParent();

            Connect(wxEVT_CLOSE_WINDOW,wxCloseEventHandler(CslDlgGeneric::OnClose),NULL,this);
        }

    private:
        void OnClose(wxCloseEvent& event)
        {
            Destroy();
        }

        void OnCommandEvent(wxCommandEvent& event)
        {
            switch (event.GetId())
            {
                case wxID_OK:
                case wxID_CLOSE:
                case wxID_CANCEL:
                    Destroy();
                    break;
            }
        }
};

#endif // CSLDLGGENERIC_H
