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

#include "Csl.h"
#include "CslLicense.h"
#include "CslDlgAbout.h"
#include "img/csl_256_png.h"

// begin wxGlade: ::extracode

// end wxGlade

BEGIN_EVENT_TABLE(CslPanelAboutImage, wxPanel)
    EVT_PAINT(CslPanelAboutImage::OnPaint)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(CslDlgAbout, wxDialog)
    EVT_CLOSE(CslDlgAbout::OnClose)
    EVT_BUTTON(wxID_CLOSE, CslDlgAbout::OnCommandEvent)
END_EVENT_TABLE()


static const wxChar *g_csl_license_pre = wxT("Cube Server Lister is free software; you can redistribute it and/or\n")
                                         wxT("modify it under the terms of the GNU General Public License version 2\n")
                                         wxT("as published by the Free Software Foundation.\n\n\n");


CslPanelAboutImage::CslPanelAboutImage(wxWindow *parent,wxInt32 id) :
        wxPanel(parent,id)
{
}

CslPanelAboutImage::~CslPanelAboutImage()
{
}

void CslPanelAboutImage::OnPaint(wxPaintEvent& event)
{
    static const wxBitmap bmp = BitmapFromData(wxBITMAP_TYPE_PNG,csl_256_png);

    wxPaintDC dc(this);
    PrepareDC(dc);

    dc.DrawBitmap(bmp,0,0,true);

    event.Skip();
}


CslDlgAbout::CslDlgAbout(wxWindow* parent,int id,const wxString& title,
                         const wxPoint& pos, const wxSize& size,long style):
        wxDialog(parent,id,title,pos,size,style)
{
    // begin wxGlade: CslDlgAbout::CslDlgAbout
    m_notebook = new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
    m_npLicense = new wxPanel(m_notebook, wxID_ANY);
    m_npCredits = new wxPanel(m_notebook, wxID_ANY);
    panel_bitmap = new CslPanelAboutImage(this, wxID_ANY);
    m_labelName = new wxStaticText(this, wxID_ANY, wxEmptyString);
    m_labelVersion = new wxStaticText(this, wxID_ANY, wxEmptyString);
    m_labelDescription = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);
    m_hlWeb = new wxHyperlinkCtrl(this, wxID_ANY, CSL_WEBADDR_STR, CSL_WEBADDR_STR);
    m_labelCopyright = new wxStaticText(this, wxID_ANY, wxEmptyString);
    m_labelwxVersion = new wxStaticText(this, wxID_ANY, wxEmptyString);
    m_tcCredits = new wxTextCtrl(m_npCredits, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY|wxHSCROLL);
    m_tcLicense = new wxTextCtrl(m_npLicense, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY|wxHSCROLL);
    m_btClose = new wxButton(this, wxID_CLOSE, wxEmptyString);

    set_properties();
    do_layout();
    // end wxGlade
}


void CslDlgAbout::set_properties()
{
    // begin wxGlade: CslDlgAbout::set_properties
    m_labelName->SetFont(wxFont(16, wxDECORATIVE, wxNORMAL, wxBOLD, 0, wxT("")));
    m_labelVersion->SetFont(wxFont(12, wxDEFAULT, wxNORMAL, wxBOLD, 0, wxT("")));
    m_btClose->SetDefault();
    // end wxGlade

    wxString s;

    s << _("Compiled using:") << wxT(" ") << CSL_WXVERSION_STR;

    SetTitle(_("About Cube Server Lister (CSL)"));

    wxFont font = m_labelCopyright->GetFont();
    //font.SetPointSize(font.GetPointSize()-1);
    font.SetStyle(wxFONTSTYLE_ITALIC);
    m_labelCopyright->SetFont(font);

    font = m_hlWeb->GetFont();
    font.SetWeight(wxFONTWEIGHT_BOLD);
    m_hlWeb->SetFont(font);

    m_labelName->SetLabel(CSL_NAME_STR);
    m_labelVersion->SetLabel(CSL_VERSION_STR);
    m_labelDescription->SetLabel(CSL_DESCRIPTION_STR);
    m_labelCopyright->SetLabel(CSL_COPYRIGHT_STR);
    m_labelwxVersion->SetLabel(s);

    s = wxString(_(
                     "Application icon:\n"
                ))+
        wxString(wxT(
                     "  Jakub 'SandMan' Uhlik\n\n"
                 ))+
        wxString(_(
                     "Map previews:\n"
                ))+
        wxString(
                 wxT("  'K!NG' Berk Inan\n")
                 wxT("  Bernd 'apflstrudl' Moeller\n")
                 wxT("  Clemens 'Hero' Wloczka\n")
                 wxT("  'shmutzwurst'\n")
                 wxT("  'ZuurKool'\n\n")
                )+
        wxString(_(
                     "Translations:\n"
                 ))+
        wxString(
                 wxT("  Czech: Jakub 'SandMan' Uhlik\n"
                 wxT("  Dutch: 'ZuurKool'\n\n")
                )+
        wxString(_(
                     "Cube & Cube2 Engine Developers:\n"
                ))+
        wxString(
                 wxT("  Wouter 'Aardappel' van Oortmerssen\n")
                 wxT("  Lee 'Eihrul' Salzman\n\n")
                ))+
        wxString(_(
                     "Country Flags:\n"
                ))+
        wxString(
                 wxT("  http://flags.blogpotato.de\n")
                );

    m_tcCredits->SetValue(s);
    m_tcLicense->SetValue(g_csl_license_pre);
    m_tcLicense->AppendText(csl_license);
    m_tcCredits->ShowPosition(0);
    m_tcLicense->ShowPosition(0);

    panel_bitmap->SetMinSize(wxSize(256,256));
#ifdef __WXMAC__
    m_notebook->SetMinSize(wxSize(400,210));
#else
    m_notebook->SetMinSize(wxSize(400,180));
#endif //__WXMAC__

    CSL_SET_WINDOW_ICON();
}


void CslDlgAbout::do_layout()
{
    // begin wxGlade: CslDlgAbout::do_layout
    wxFlexGridSizer* grid_sizer_main = new wxFlexGridSizer(9, 1, 0, 0);
    wxGridSizer* sizer_license = new wxGridSizer(1, 1, 0, 0);
    wxGridSizer* sizer_credits = new wxGridSizer(1, 1, 0, 0);
    grid_sizer_main->Add(panel_bitmap, 1, wxALIGN_CENTER_HORIZONTAL, 0);
    grid_sizer_main->Add(m_labelName, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 4);
    grid_sizer_main->Add(m_labelVersion, 0, wxALIGN_CENTER_HORIZONTAL, 2);
    grid_sizer_main->Add(m_labelDescription, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 4);
    grid_sizer_main->Add(m_hlWeb, 1, wxTOP|wxALIGN_CENTER_HORIZONTAL, 8);
    grid_sizer_main->Add(m_labelCopyright, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 4);
    grid_sizer_main->Add(m_labelwxVersion, 0, wxTOP|wxALIGN_CENTER_HORIZONTAL, 8);
    sizer_credits->Add(m_tcCredits, 0, wxALL|wxEXPAND, 4);
    m_npCredits->SetSizer(sizer_credits);
    sizer_license->Add(m_tcLicense, 0, wxALL|wxEXPAND, 4);
    m_npLicense->SetSizer(sizer_license);
    m_notebook->AddPage(m_npCredits, _("Credits"));
    m_notebook->AddPage(m_npLicense, _("License"));
    grid_sizer_main->Add(m_notebook, 1, wxALL|wxEXPAND, 8);
    grid_sizer_main->Add(m_btClose, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 4);
    SetSizer(grid_sizer_main);
    grid_sizer_main->Fit(this);
    grid_sizer_main->AddGrowableRow(7);
    grid_sizer_main->AddGrowableCol(0);
    Layout();
    // end wxGlade

    SetMinSize(wxSize(380,300));
    grid_sizer_main->SetSizeHints(this);

    CSL_CENTRE_DIALOG();
}

void CslDlgAbout::OnClose(wxCloseEvent& event)
{
    Destroy();
}

void CslDlgAbout::OnCommandEvent(wxCommandEvent& event)
{
    Close();
}
