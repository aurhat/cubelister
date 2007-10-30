
#include "CslDlgAbout.h"
#include "CslVersion.h"
#include "CslLicense.h"
#ifndef _MSC_VER
#include "img/csl_64.xpm"
#endif

// begin wxGlade: ::extracode

// end wxGlade

BEGIN_EVENT_TABLE(CslDlgAbout, wxDialog)
    EVT_BUTTON(wxID_ANY,CslDlgAbout::OnCommandEvent)
END_EVENT_TABLE()

const wxChar *csl_credits = wxT(\
                                "Extended info server patches:\n"
                                "  }TC{noob\n"
                                "\n"
                                "Map previews:\n"
                                "  }TC{apflstrudl\n"
                                "  }TC{Hero\n"
                                "\n"
                                "Application icon:\n"
                                "  }TC{SandMan\n"
                                "\n"
                                "Country Flags:\n"
                                "  http://flags.blogpotato.de\n");

const wxChar *csl_license_pre = wxT(\
                                    "Cube Server Lister is free software; you can redistribute it and/or\n"
                                    "modify it under the terms of the GNU General Public License version 2\n"
                                    "as published by the Free Software Foundation.\n\n\n");

CslDlgAbout::CslDlgAbout(wxWindow* parent,int id,const wxString& title,
                         const wxPoint& pos, const wxSize& size,long style):
        wxDialog(parent,id,title,pos,size,style)
{
    // begin wxGlade: CslDlgAbout::CslDlgAbout
    notebook = new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
    notebook_pane_license = new wxPanel(notebook, wxID_ANY);
    notebook_pane_credits = new wxPanel(notebook, wxID_ANY);
    bitmap_logo = new wxStaticBitmap(this, wxID_ANY, wxNullBitmap);
    label_name = new wxStaticText(this, wxID_ANY, wxEmptyString);
    label_version = new wxStaticText(this, wxID_ANY, wxEmptyString);
    label_desc = new wxStaticText(this, wxID_ANY, wxEmptyString);
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

    SetTitle(wxT("About Cube Server Lister (CSL)"));

    wxFont font=label_copyright->GetFont();
    font.SetPointSize(font.GetPointSize()-1);
    font.SetStyle(wxFONTSTYLE_ITALIC);
    label_copyright->SetFont(font);

    bitmap_logo->SetBitmap(wxICON(csl_64));
    label_name->SetLabel(CSL_NAME_STR);
    label_version->SetLabel(CSL_VERSION_LONG_STR);
    label_desc->SetLabel(CSL_DESCRIPTION_STR);
    label_copyright->SetLabel(CSL_COPYRIGHT_STR);

    text_ctrl_credits->SetValue(csl_credits);
    text_ctrl_license->SetValue(csl_license_pre);
    text_ctrl_license->AppendText(csl_license);
    text_ctrl_credits->ShowPosition(0);
    text_ctrl_license->ShowPosition(0);
}


void CslDlgAbout::do_layout()
{
    // begin wxGlade: CslDlgAbout::do_layout
    wxFlexGridSizer* grid_sizer_main = new wxFlexGridSizer(7, 1, 0, 0);
    wxGridSizer* sizer_license = new wxGridSizer(1, 1, 0, 0);
    wxGridSizer* sizer_credits = new wxGridSizer(1, 1, 0, 0);
    grid_sizer_main->Add(bitmap_logo, 0, wxALIGN_CENTER_HORIZONTAL, 0);
    grid_sizer_main->Add(label_name, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 4);
    grid_sizer_main->Add(label_version, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 4);
    grid_sizer_main->Add(label_desc, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 4);
    grid_sizer_main->Add(label_copyright, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 4);
    sizer_credits->Add(text_ctrl_credits, 0, wxALL|wxEXPAND, 4);
    notebook_pane_credits->SetSizer(sizer_credits);
    sizer_license->Add(text_ctrl_license, 0, wxALL|wxEXPAND, 4);
    notebook_pane_license->SetSizer(sizer_license);
    notebook->AddPage(notebook_pane_credits, _("Credits"));
    notebook->AddPage(notebook_pane_license, _("License"));
    grid_sizer_main->Add(notebook, 1, wxALL|wxEXPAND, 8);
    grid_sizer_main->Add(button_close, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 4);
    SetSizer(grid_sizer_main);
    grid_sizer_main->Fit(this);
    grid_sizer_main->AddGrowableRow(5);
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
