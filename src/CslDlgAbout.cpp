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

#include <wx/mstream.h>
#include "engine/CslTools.h"
#include "engine/CslVersion.h"
#include "CslDlgAbout.h"
#include "CslLicense.h"
#include "csl_logo_png.h"

// begin wxGlade: ::extracode

// end wxGlade

BEGIN_EVENT_TABLE(CslPanelAboutImage, wxPanel)
    EVT_PAINT(CslPanelAboutImage::OnPaint)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(CslDlgAbout, wxDialog)
    EVT_BUTTON(wxID_ANY,CslDlgAbout::OnCommandEvent)
END_EVENT_TABLE()

const wxChar *csl_credits = wxT(\
                                "Application icon:\n"
                                "  Jakub 'SandMan' Uhlik\n"
                                "\n"
                                "Map previews:\n"
                                "  'K!NG' Berk Inan\n"
                                "  Bernd 'apflstrudl' Möller\n"
                                "  Clemens 'Hero' Wloczka\n"
                                "  'shmutzwurst'\n"
                                "  'ZuurKool'\n"
                                "\n"
                                "Translations:\n"
                                "  German: Glen Masgai"
                                "  Dutch: 'ZuurKool'"
                                "\n"
                                "Previous extended info server patches:\n"
                                "  noob\n"
                                "\n"
                                "Country Flags:\n"
                                "  http://flags.blogpotato.de\n");

const wxChar *csl_license_pre = wxT(\
                                    "Cube Server Lister is free software; you can redistribute it and/or\n"
                                    "modify it under the terms of the GNU General Public License version 2\n"
                                    "as published by the Free Software Foundation.\n\n\n");


CslPanelAboutImage::CslPanelAboutImage(wxWindow *parent,wxInt32 id) :
        wxPanel(parent,id)
{
    wxMemoryInputStream stream(csl_logo_png,sizeof(csl_logo_png));
    // see wx_wxbitmap.html
#ifdef __WXMSW__
    m_bitmap=new wxBitmap(stream,wxBITMAP_TYPE_PNG);
#else
    wxImage image(stream,wxBITMAP_TYPE_PNG);
    m_bitmap=new wxBitmap(image);
#endif
}

CslPanelAboutImage::~CslPanelAboutImage()
{
    delete m_bitmap;
}

void CslPanelAboutImage::OnPaint(wxPaintEvent& event)
{
    event.Skip();

    if (!m_bitmap->IsOk())
        return;

    wxMemoryDC memDC;

    wxPaintDC dc(this);
    PrepareDC(dc);

    memDC.SelectObject(*m_bitmap);
    dc.Blit(0,0,m_bitmap->GetWidth(),m_bitmap->GetHeight(),&memDC,0,0,wxCOPY,true);
}


CslDlgAbout::CslDlgAbout(wxWindow* parent,int id,const wxString& title,
                         const wxPoint& pos, const wxSize& size,long style):
        wxDialog(parent,id,title,pos,size,style)
{
    // begin wxGlade: CslDlgAbout::CslDlgAbout
    notebook = new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
    notebook_pane_license = new wxPanel(notebook, wxID_ANY);
    notebook_pane_credits = new wxPanel(notebook, wxID_ANY);
    panel_bitmap = new CslPanelAboutImage(this, wxID_ANY);
    label_name = new wxStaticText(this, wxID_ANY, wxEmptyString);
    label_version = new wxStaticText(this, wxID_ANY, wxEmptyString);
    label_desc = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);
    hyperlink_web = new wxHyperlinkCtrl(this, wxID_ANY, CSL_WEBADDRFULL_STR, CSL_WEBADDRFULL_STR);
    label_copyright = new wxStaticText(this, wxID_ANY, wxEmptyString);
    text_ctrl_credits = new wxTextCtrl(notebook_pane_credits, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY|wxHSCROLL);
    text_ctrl_license = new wxTextCtrl(notebook_pane_license, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY|wxHSCROLL);
    button_close = new wxButton(this, wxID_CLOSE, _("&Close"));

    set_properties();
    do_layout();
    // end wxGlade
}


void CslDlgAbout::set_properties()
{
    // begin wxGlade: CslDlgAbout::set_properties
    label_name->SetFont(wxFont(16, wxDECORATIVE, wxNORMAL, wxBOLD, 0, wxT("")));
    label_version->SetFont(wxFont(12, wxDEFAULT, wxNORMAL, wxBOLD, 0, wxT("")));
    button_close->SetDefault();
    // end wxGlade

    SetTitle(_("About Cube Server Lister (CSL)"));

    wxFont font=label_copyright->GetFont();
    //font.SetPointSize(font.GetPointSize()-1);
    font.SetStyle(wxFONTSTYLE_ITALIC);
    label_copyright->SetFont(font);

    font=hyperlink_web->GetFont();
    font.SetWeight(wxFONTWEIGHT_BOLD);
    hyperlink_web->SetFont(font);

    label_name->SetLabel(CSL_NAME_STR);
    label_version->SetLabel(CSL_VERSION_LONG_STR);
    label_desc->SetLabel(CSL_DESCRIPTION_STR);
    label_copyright->SetLabel(CSL_COPYRIGHT_STR);

    text_ctrl_credits->SetValue(csl_credits);
    text_ctrl_license->SetValue(csl_license_pre);
    text_ctrl_license->AppendText(csl_license);
    text_ctrl_credits->ShowPosition(0);
    text_ctrl_license->ShowPosition(0);

    panel_bitmap->SetMinSize(wxSize(256,256));
#ifdef __WXMAC__
    notebook->SetMinSize(wxSize(400,210));
#else
    notebook->SetMinSize(wxSize(400,180));
#endif //__WXMAC__
}


void CslDlgAbout::do_layout()
{
    // begin wxGlade: CslDlgAbout::do_layout
    wxFlexGridSizer* grid_sizer_main = new wxFlexGridSizer(8, 1, 0, 0);
    wxGridSizer* sizer_license = new wxGridSizer(1, 1, 0, 0);
    wxGridSizer* sizer_credits = new wxGridSizer(1, 1, 0, 0);
    grid_sizer_main->Add(panel_bitmap, 1, wxALIGN_CENTER_HORIZONTAL, 0);
    grid_sizer_main->Add(label_name, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 4);
    grid_sizer_main->Add(label_version, 0, wxALIGN_CENTER_HORIZONTAL, 2);
    grid_sizer_main->Add(label_desc, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 4);
    grid_sizer_main->Add(hyperlink_web, 1, wxTOP|wxALIGN_CENTER_HORIZONTAL, 8);
    grid_sizer_main->Add(label_copyright, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 4);
    sizer_credits->Add(text_ctrl_credits, 0, wxALL|wxEXPAND, 4);
    notebook_pane_credits->SetSizer(sizer_credits);
    sizer_license->Add(text_ctrl_license, 0, wxALL|wxEXPAND, 4);
    notebook_pane_license->SetSizer(sizer_license);
    notebook->AddPage(notebook_pane_credits, _("Credits"));
    notebook->AddPage(notebook_pane_license, _("License"));
    grid_sizer_main->Add(notebook, 1, wxALL|wxEXPAND, 8);
    grid_sizer_main->Add(button_close, 0, wxBOTTOM|wxALIGN_CENTER_HORIZONTAL, 8);
    SetSizer(grid_sizer_main);
    grid_sizer_main->Fit(this);
    grid_sizer_main->AddGrowableRow(6);
    grid_sizer_main->AddGrowableCol(0);
    Layout();
    // end wxGlade

    SetMinSize(wxSize(380,300));
    grid_sizer_main->SetSizeHints(this);

    CentreOnParent();
}

void CslDlgAbout::OnCommandEvent(wxCommandEvent& event)
{
    this->Destroy();
}
