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

#ifndef CSLDLGGENERIC_H
#define CSLDLGGENERIC_H

/**
 @author Glen Masgai <mimosius@users.sourceforge.net>
*/

#define CSL_DLG_GENERIC_DEFAULT     0
#define CSL_DLG_GENERIC_OK       1<<0
#define CSL_DLG_GENERIC_CLOSE    1<<1
#define CSL_DLG_GENERIC_URL      1<<2


class CslDlgGeneric: public wxDialog
{
    public:

        CslDlgGeneric(wxWindow *parent,wxUint32 type,
                      const wxString& title,
                      const wxString& text,
                      const wxBitmap& bitmap,
                      const wxString& url=wxEmptyString) :
                wxDialog(parent,wxID_ANY,title,wxDefaultPosition,
                         wxDefaultSize,wxDEFAULT_DIALOG_STYLE)
        {
            wxUint32 rowsMain=1,rowsRight=1;
            if (type&CSL_DLG_GENERIC_URL) rowsRight++;
            if (type&CSL_DLG_GENERIC_CLOSE) rowsMain++;

            wxFlexGridSizer* grid_sizer_main=new wxFlexGridSizer(rowsMain,1,0,0);
            wxFlexGridSizer* grid_sizer_top=new wxFlexGridSizer(1,2,0,0);
            wxFlexGridSizer* grid_sizer_right=new wxFlexGridSizer(rowsRight,1,0,0);

            grid_sizer_top->Add(new wxStaticBitmap(this,wxID_ANY,bitmap),0,wxTOP|wxLEFT|wxBOTTOM,12);

            grid_sizer_right->Add(new wxStaticText(this,wxID_ANY,text),
                                  0,wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL,10);

            if (type&CSL_DLG_GENERIC_URL)
                grid_sizer_right->Add(new wxHyperlinkCtrl(this,wxID_ANY,url,url),
                                      0,wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL,10);

            if (type&CSL_DLG_GENERIC_OK || type&CSL_DLG_GENERIC_CLOSE)
            {
                wxButton *button=new wxButton(this,type&CSL_DLG_GENERIC_OK ? wxID_OK : wxID_CLOSE);

                grid_sizer_main->Add(button,0,wxALL|wxALIGN_RIGHT,8);
                Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                        wxCommandEventHandler(CslDlgGeneric::OnCommandEvent),
                        NULL,this);
                button->SetDefault();
            }

            grid_sizer_top->Add(grid_sizer_right,1,wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM,4);
            grid_sizer_main->Insert(0,grid_sizer_top,1,0,0);

            grid_sizer_right->AddGrowableCol(0);
            grid_sizer_top->AddGrowableCol(1);
            grid_sizer_main->AddGrowableCol(0);

            SetSizer(grid_sizer_main);
            grid_sizer_main->Fit(this);
            Layout();

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
                    Destroy();
                    break;
                default:
                    break;
            }
        }
};

#endif // CSLDLGGENERIC_H
