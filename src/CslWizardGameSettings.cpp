/***************************************************************************
 *   Copyright (C) 2007-2011 by Glen Masgai                                *
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
#include "CslFileDirPicker.h"
#define CSL_WIZARD_DEFINE_PAGE_CLASSES
#include "CslWizardGameSettings.h"
#undef  CSL_WIZARD_DEFINE_PAGE_CLASSES


static const struct CslScreenResolution
{
    const wxInt32 Width, Height;
}
ASPECT_4_3[] =
{
    { 320,  240  },  { 640,  480  }, { 800,  600  }, { 1024, 768  }, { 1152, 864  },
    { 1280, 960  },  { 1400, 1050 }, { 1600, 1200 }, { 1792, 1344 }, { 1856, 1392 },
    { 1920, 1440 },  { 2048, 1536 }, { 2800, 2100 }, { 3200, 2400 }
},
ASPECT_16_10[] =
{
    { 320,  200  },  { 640,  400  }, { 1024, 640  }, { 1280, 800  }, { 1440, 900  },
    { 1600, 1000 },  { 1680, 1050 }, { 1920, 1200 }, { 2048, 1280 }, { 2560, 1600 },
    { 3840, 2400 }
},
ASPECT_16_9[] =
{
    { 1024, 600  }, { 1280,  720  }, { 1366,  768 }, { 1600, 900  }, { 1920, 1050 },
    { 2048, 1152 }, { 3840, 2160  }
},
ASPECT_5_4[] =
{
    { 600, 480   }, { 1280, 1024  }, { 1600, 1280 }, { 2560, 2048 }
},
ASPECT_5_3[] =
{
    { 800, 480   }, { 1280, 768   }
};


static const struct CslScreenRatio
{
    const wxInt32 Count;
    const wxChar *Name;
    const CslScreenResolution *Resolutions;
}
CSL_SCREEN_RATIOS[] =
{
    {  0, wxT("Default"), NULL         },
    {  0, wxT("Custom") , NULL         },
    { 14, wxT("4:3")    , ASPECT_4_3   },
    { 11, wxT("16:10")  , ASPECT_16_10 },
    {  7, wxT("16:9")   , ASPECT_16_9  },
    {  4, wxT("5:4")    , ASPECT_5_4   },
    {  2, wxT("5:3")    , ASPECT_5_3   }
};

#define CSL_MAX_SCREEN_RESOLUTIONS 16
#define CSL_SCREEN_RATIOS_LEN ((wxInt32)(sizeof(CSL_SCREEN_RATIOS)/sizeof(CSL_SCREEN_RATIOS[0])))


BEGIN_EVENT_TABLE(CslWizardGameSettingsPagePaths, wxWizardPageSimple)
    CSL_EVT_FILEDIRPICK(wxID_ANY, CslWizardGameSettingsPagePaths::OnPickEvent)
END_EVENT_TABLE()

CslWizardGameSettingsPagePaths::CslWizardGameSettingsPagePaths(CslWizardGameSettings *parent) :
        wxWizardPageSimple(parent),
        m_parent(parent)
{
    wxString s;
    CslGameClientSettings& settings=parent->GetClientSettings();

    m_sizer=new wxFlexGridSizer(4, 1, 0, 0);
    wxFlexGridSizer *sizer=new wxFlexGridSizer(4, 2, 0, 0);

    m_sizer->Add(new CslDescriptionSizer(this, _("Game paths")), 1, wxEXPAND);

    sizer->Add(new wxStaticText(this, PICK_GAME, _("Game directory:")), 0, wxALIGN_CENTER_VERTICAL);
    s=wxString::Format(_("Select %s game path"), m_parent->GetGame().GetName().c_str());
    m_pick_dir=new CslDirPickerCtrl(this, wxID_ANY, s, settings.GamePath, CSL_DP_OPEN_EXIST);
    sizer->Add(m_pick_dir, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 4);

    sizer->Add(1, 16); sizer->Add(1, 16);

    sizer->Add(new wxStaticText(this, PICK_EXE, _("Game executable:")), 0, wxALIGN_CENTER_VERTICAL);
    s=wxString::Format(_("Select %s executable"), m_parent->GetGame().GetName().c_str());
    m_pick_exe=new CslFilePickerCtrl(this, wxID_ANY, s, settings.Binary, wxEmptyString,
                                     CSL_EXE_EXTENSIONS, CSL_FP_OPEN_EXIST);
    sizer->Add(m_pick_exe, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 4);

    sizer->Add(new wxStaticText(this, PICK_CFG, _("Config directory:")), 0, wxALIGN_CENTER_VERTICAL);
    s=wxString::Format(_("Select %s config path"), m_parent->GetGame().GetName().c_str());
    m_pick_cfg=new CslDirPickerCtrl(this, wxID_ANY, s, settings.ConfigPath, CSL_DP_OPEN_EXIST);
    sizer->Add(m_pick_cfg, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 4);

    sizer->AddGrowableCol(1);

    m_sizer->Add(sizer, 1, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 8);
    m_sizer->Add(new wxStaticLine(this), 0, wxEXPAND|wxALL, 8);
    m_sizer->Add(new wxStaticText(this, wxID_ANY,
                                  _("Select the path where the game is installed. CSL tries to figure out\n"
                                    _L_"automatically all other necessary paths like game executeable and\n"
                                    _L_"config path. In case you need some special setup, you can adjust\n"
                                    _L_"the values manually.")),
                 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 8);

    m_sizer->AddGrowableCol(0);
    m_sizer->Layout();

    SetSizer(m_sizer);
    m_sizer->Fit(this);

    m_sizer->SetMinSize(m_sizer->GetSize());
}

void CslWizardGameSettingsPagePaths::OnPickEvent(CslFileDirPickerEvent& event)
{
#define PRINT_ERROR(_warn) \
    do { \
        if (!error.IsEmpty()) \
        { \
            if (!result.IsEmpty()) \
            { \
                error<<wxT("\n\n")<<_("Missing files/folders:"); \
                \
                for (size_t i=0; i<result.GetCount(); i++) \
                    error<<wxT(" ")<<result.Item(i); \
                \
                if (_warn) \
                    error<<wxT("\n\n")<<_("This is a warning only. If the game doesn't\n" \
                                          _L_"start properly, run the wizard again and try\n" \
                                          _L_"a different path."); \
            } \
            wxMessageBox(error, _warn ? _("Warning") : _("Error"), \
                         wxOK|(_warn ? wxICON_WARNING : wxICON_ERROR), this); \
            error.Empty(); \
        } \
    } while (0)

    wxString error;
    wxArrayString result;
    CslGameClientSettings& settings=m_parent->GetClientSettings();

    switch (event.GetId())
    {
        case PICK_GAME:
        {
            if (!event.Path.IsEmpty())
            {
                int ret;
                CslGame& game=m_parent->GetGame();

                if ((ret=game.GetClientPath(CSL_GAME_GET_PATH_GAME, event.Path, result, error))<0)
                    break;
                else if (ret>0)
                    PRINT_ERROR(true);
                settings.GamePath=event.Path;
                result.Empty();

                if ((ret=game.GetClientPath(CSL_GAME_GET_PATH_BINARY, settings.GamePath, result, error))<0)
                    break;
                else if (ret>0)
                    PRINT_ERROR(true);
                if (result.GetCount()>1)
                {
                    int w=0, h=0;

                    for (size_t i=0; i<result.GetCount(); i++)
                    {
                        const wxSize& size=GetBestWindowSizeForText(this, result.Item(i), 150, 640, 0, -1);
                        w=max(w, size.x);
                        h=min(h+size.y, 480);
                    }

                    settings.Binary=wxGetSingleChoice(_("Multiple clients found. Please select a client."),
                                                      _("Select a client"), result, this, -1, -1, true, w, h);
                    if (settings.Binary.IsEmpty())
                        break;
                }
                else if (result.GetCount())
                    settings.Binary=result.Item(0);
                else
                    break;
                result.Empty();

                if ((ret=game.GetClientPath(CSL_GAME_GET_PATH_CONFIG, event.Path, result, error))<0)
                    break;
                else if (ret>0)
                    PRINT_ERROR(true);
                if (result.GetCount())
                    settings.ConfigPath=result.Item(0);
                else
                    break;

                m_pick_exe->SetPath(settings.Binary);
                m_pick_cfg->SetPath(settings.ConfigPath);
            }

            return;
        }

        default:
            return;
    }

    switch (event.GetId())
    {
        case PICK_GAME:
            settings.GamePath=settings.Binary=settings.ConfigPath=wxEmptyString;
            break;

        default:
            break;
    }

    PRINT_ERROR(false);
}


BEGIN_EVENT_TABLE(CslWizardGameSettingsPageGFX, wxWizardPageSimple)
    EVT_CHOICE(wxID_ANY, CslWizardGameSettingsPageGFX::OnCommandEvent)
END_EVENT_TABLE()

CslWizardGameSettingsPageGFX::CslWizardGameSettingsPageGFX(CslWizardGameSettings *parent) :
        wxWizardPageSimple(parent),
        m_parent(parent)
{
    m_sizer=new wxFlexGridSizer(6, 1, 0, 0);

    m_sizer->Add(new CslDescriptionSizer(this, _("Game screen resolution")), 1, wxEXPAND);
    wxFlexGridSizer *sizer=new wxFlexGridSizer(1, 2, 0, 0);
    sizer->Add(new wxStaticText(this, wxID_ANY, _("Screen aspect ratio:")), 0, wxALIGN_CENTER_VERTICAL);
    wxChoice *choice=new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxArrayString());
    for (wxInt32 i=0; i<CSL_SCREEN_RATIOS_LEN; i++)
        choice->Append(CSL_SCREEN_RATIOS[i].Name);
    choice->SetSelection(0);
    sizer->Add(choice, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxLEFT, 4);
    sizer->AddGrowableCol(1);
    m_sizer->Add(sizer, 1, wxALIGN_CENTER_HORIZONTAL|wxLEFT|wxRIGHT|wxBOTTOM, 8);

    m_sizer_resolution=new wxFlexGridSizer(5, 4, 0, 0);
    m_static_width=new wxStaticText(this, wxID_ANY, _("Width:"));
    m_sizer_resolution->Add(m_static_width, 0, wxALIGN_CENTER_VERTICAL);
    m_text_width=new wxTextCtrl(this, wxID_ANY);
    m_sizer_resolution->Add(m_text_width, 0, wxALIGN_CENTER_VERTICAL|wxLEFT, 4);
    m_static_height=new wxStaticText(this, wxID_ANY, _("Height:"));
    m_sizer_resolution->Add(m_static_height, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL);
    m_text_height=new wxTextCtrl(this, wxID_ANY);
    m_sizer_resolution->Add(m_text_height, 0, wxALIGN_CENTER_VERTICAL|wxLEFT, 4);
    for (wxInt32 i=0; i<CSL_MAX_SCREEN_RESOLUTIONS; i++)
    {
        wxRadioButton *radio=new wxRadioButton(this, wxID_ANY, wxT("9999 x 9999"));
        m_sizer_resolution->Add(m_radio_buttons.add(radio), 0, wxALIGN_CENTER_VERTICAL|wxALL, 4);
    }
    m_sizer_resolution->AddGrowableCol(2);
    m_sizer->Add(m_sizer_resolution, 1, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 8);

    m_check_window_mode=new wxCheckBox(this, wxID_ANY, _("Window mode"));
    m_sizer->Add(m_check_window_mode, 0, wxALIGN_CENTER_HORIZONTAL|wxLEFT|wxRIGHT, 8);

    m_sizer->Add(new wxStaticLine(this), 0, wxEXPAND|wxALL, 8);
    m_sizer->Add(new wxStaticText(this, wxID_ANY,
                                  _("Select your prefered grahics settings or leave it to default to use\n"
                                    _L_"the last settings saved by the game.")),
                 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 8);

    m_sizer->AddGrowableCol(0);
    m_sizer->Layout();

    SetSizer(m_sizer);
    m_sizer->Fit(this);

    m_sizer->SetMinSize(m_sizer->GetSize());

    ToggleResolution(0);
}

void CslWizardGameSettingsPageGFX::OnCommandEvent(wxCommandEvent& event)
{
    ToggleResolution(event.GetSelection());
}

void CslWizardGameSettingsPageGFX::ToggleResolution(wxInt32 id)
{
    static bool growable=false;

    if (id<0 || id>=CSL_SCREEN_RATIOS_LEN)
        return;

    const CslScreenRatio& ratio=CSL_SCREEN_RATIOS[id];
    wxInt32 l=ratio.Count;

    for (wxInt32 i=0; i<CSL_MAX_SCREEN_RESOLUTIONS; i++)
    {
        wxRadioButton *radio=m_radio_buttons[i];

        if (i>=l)
        {
            if (radio->IsShown())
            {
                radio->Hide();
                m_sizer_resolution->Detach(radio);
            }
        }
        else
        {
            const CslScreenResolution& r=ratio.Resolutions[i];

            if (!radio->IsShown())
            {
                m_sizer_resolution->Add(radio, 0, wxALIGN_CENTER_VERTICAL|wxALL, 4);
                radio->Show();
            }
            radio->SetLabel(wxString::Format(wxT("%d x %d"), r.Width, r.Height));
        }
    }

    if (l)
    {
        if (m_text_width->IsShown())
        {
            m_static_width->Hide();
            m_text_width->Hide();
            m_static_height->Hide();
            m_text_height->Hide();
            m_sizer_resolution->Detach(m_static_width);
            m_sizer_resolution->Detach(m_text_width);
            m_sizer_resolution->Detach(m_static_height);
            m_sizer_resolution->Detach(m_text_height);
        }
        if (!growable)
        {
            growable=true;
            m_sizer_resolution->AddGrowableCol(0);
            m_sizer_resolution->AddGrowableCol(1);
            m_sizer_resolution->AddGrowableCol(3);
        }
    }
    else if (!l)
    {
        if (!m_text_width->IsShown())
        {
            m_static_width->Show();
            m_text_width->Show();
            m_static_height->Show();
            m_text_height->Show();

            m_sizer_resolution->Add(m_static_width, 0, wxALIGN_CENTER_VERTICAL);
            m_sizer_resolution->Add(m_text_width, 0, wxALIGN_CENTER_VERTICAL|wxLEFT, 4);
            m_sizer_resolution->Add(m_static_height, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL);
            m_sizer_resolution->Add(m_text_height, 0, wxALIGN_CENTER_VERTICAL|wxLEFT, 4);
        }

        m_text_width->Enable(id==1);
        m_text_height->Enable(id==1);

        if (growable)
        {
            growable=false;
            m_sizer_resolution->RemoveGrowableCol(0);
            m_sizer_resolution->RemoveGrowableCol(1);
            m_sizer_resolution->RemoveGrowableCol(3);
        }
    }

    m_sizer->Layout();
}

CslWizardGameSettingsPageScripts::CslWizardGameSettingsPageScripts(CslWizardGameSettings *parent) :
        wxWizardPageSimple(parent),
        m_parent(parent)
{
    CslGameClientSettings& settings=parent->GetClientSettings();

    wxFlexGridSizer *sizer=new wxFlexGridSizer(5, 1, 0, 0);

    sizer->Add(new CslDescriptionSizer(this, _("Pre connect script")), 1, wxEXPAND);
    m_text_pre=new wxTextCtrl(this, wxID_ANY, settings.PreScript, wxDefaultPosition, wxDefaultSize,
                              wxTE_MULTILINE|wxTE_BESTWRAP|wxTE_RICH|wxTE_RICH2);
    sizer->Add(m_text_pre, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 8);

    sizer->Add(new CslDescriptionSizer(this, _("Post connect script")), 1, wxEXPAND);
    m_text_post=new wxTextCtrl(this, wxID_ANY, settings.PreScript, wxDefaultPosition, wxDefaultSize,
                               wxTE_MULTILINE|wxTE_BESTWRAP|wxTE_RICH|wxTE_RICH2);
    sizer->Add(m_text_post, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 8);

    sizer->Add(new wxStaticText(this, wxID_ANY,
                                _("Here you can define some cubescript which is getting executed right\n"
                                  _L_"before connecting to the server and after the connect command.\n"
                                  _L_"The following templates are getting replaced: \n\n"
                                  _L_"    #csl_server_host#\n"
                                  _L_"    #csl_server_port#\n"
                                  _L_"    #csl_server_pass#\n"
                                  _L_"    #csl_server_adminpass#\n"
                                  _L_"    #csl_server_used_pass#\n")),
               0, wxALIGN_CENTER_HORIZONTAL|wxALL, 8);

    sizer->AddGrowableCol(0);
    sizer->AddGrowableRow(1);
    sizer->AddGrowableRow(3);
    sizer->Layout();

    SetSizer(sizer);
    sizer->Fit(this);

    sizer->SetMinSize(sizer->GetSize());
}

CslWizardGameSettings::CslWizardGameSettings(wxWindow *parent, CslGame *game,
        CslGameClientSettings *settings,
        const wxString &title, const wxBitmap &bitmap) :
        wxWizard(parent, wxID_ANY, title, bitmap, wxDefaultPosition, wxDEFAULT_DIALOG_STYLE),
        m_game(game), m_settings(settings)
{
    m_page=new wxWizardPageSimple(this);

    new wxStaticText(m_page, wxID_ANY,
                     _("This wizard will help you to setup the necessary game paths\n" \
                       _L_"and other game specific settings, so you can easily start\n" \
                       _L_"the game from within CSL.\n"), wxPoint(6, 6));

    CslWizardGameSettingsPagePaths *page2=new CslWizardGameSettingsPagePaths(this);
    m_page->SetNext(page2);
    page2->SetPrev(m_page);
    CslWizardGameSettingsPageGFX *page3=new CslWizardGameSettingsPageGFX(this);
    page2->SetNext(page3);
    page3->SetPrev(page2);
    CslWizardGameSettingsPageScripts *page4=new CslWizardGameSettingsPageScripts(this);
    page3->SetNext(page4);
    page4->SetPrev(page3);

    GetPageAreaSizer()->Add(m_page);
    GetPageAreaSizer()->Add(page2);
    GetPageAreaSizer()->Add(page3);
    GetPageAreaSizer()->Add(page4);

    CentreOnParent();
}
