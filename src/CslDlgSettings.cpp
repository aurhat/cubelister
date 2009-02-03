/***************************************************************************
 *   Copyright (C) 2007 -2009 by Glen Masgai                               *
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

#include <wx/colordlg.h>
#include <wx/imaglist.h>
#include "CslDlgSettings.h"
#include "CslApp.h"
#include "engine/CslGame.h"
#include "engine/CslTools.h"


BEGIN_EVENT_TABLE(CslGamePage,wxPanel)
    EVT_FILEPICKER_CHANGED(wxID_ANY,CslGamePage::OnPicker)
    EVT_DIRPICKER_CHANGED(wxID_ANY,CslGamePage::OnPicker)
    EVT_CHECKBOX(wxID_ANY,CslGamePage::OnCommandEvent)
END_EVENT_TABLE()


BEGIN_EVENT_TABLE(CslDlgSettings,wxDialog)
    EVT_BUTTON(wxID_ANY,CslDlgSettings::OnCommandEvent)
    EVT_RADIOBUTTON(wxID_ANY,CslDlgSettings::OnCommandEvent)
    EVT_SPINCTRL(wxID_ANY,CslDlgSettings::OnSpinCtrl)
    EVT_CHECKBOX(wxID_ANY,CslDlgSettings::OnCommandEvent)
    EVT_DIRPICKER_CHANGED(wxID_ANY,CslDlgSettings::OnPicker)
END_EVENT_TABLE()


enum
{
    CHECK_GAME_EXPERT = wxID_HIGHEST + 1,

    BUTTON_COLOUR_EMPTY,
    BUTTON_COLOUR_OFF,
    BUTTON_COLOUR_FULL,
    BUTTON_COLOUR_MM1,
    BUTTON_COLOUR_MM2,
    BUTTON_COLOUR_MM3,

    SPIN_CLEANUP_SERVERS,
    SPIN_TOOLTIP_DELAY,

    SPIN_PING_GOOD,
    SPIN_PING_BAD,

    CHECK_SYSTRAY,
    RADIO_SYSTRAY_ALWAYS,
    RADIO_SYSTRAY_MINIMIZE,

    CHECK_GAME_OUTPUT,

    FILE_PICKER,
    DIR_PICKER_GAME,
    DIR_PICKER_CFG
};


CslSettings *g_cslSettings;


CslGamePage::CslGamePage(wxWindow *parent,CslGame *game) :
        wxPanel(parent,wxID_ANY),m_game(game)
{
    wxString s;
#ifdef __WXMSW__
    wxString extensions=_("Executables (*.exe; *.bat; *.cmd)|*.exe;*.bat;*.cmd");
#else
    wxString extensions=wxT("*");
#endif
    wxInt32 dpickBorder;
    const CslGameClientSettings& settings=game->GetClientSettings();

#ifdef __WXMAC__
    dpickBorder=-1;
#else
    dpickBorder=4;
#endif
    sizer=new wxFlexGridSizer(5,2,0,0);

    label_exe=new wxStaticText(this,wxID_ANY,_("Game executable:"));
    sizer->Add(label_exe,0,wxLEFT|wxALIGN_CENTER_VERTICAL,8);

    s=wxString::Format(_("Select %s executable"),game->GetName().c_str());
    filepicker=new wxFilePickerCtrl(this,FILE_PICKER,settings.Binary,
                                    s,extensions,wxDefaultPosition,wxDefaultSize,
                                    wxFLP_DEFAULT_STYLE|wxFLP_USE_TEXTCTRL|wxFLP_FILE_MUST_EXIST);
    sizer->Add(filepicker,0,wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL,dpickBorder);
    filepicker->SetPath(settings.Binary);

    label_gamepath=new wxStaticText(this,wxID_ANY,_("Game directory:"));
    sizer->Add(label_gamepath,0,wxLEFT|wxALIGN_CENTER_VERTICAL,8);

    s=wxString::Format(_("Select %s game path"),game->GetName().c_str());
    dirpickergame=new wxDirPickerCtrl(this,DIR_PICKER_GAME,settings.GamePath,
                                      s,wxDefaultPosition,wxDefaultSize,
                                      wxDIRP_DEFAULT_STYLE|wxDIRP_USE_TEXTCTRL|wxDIRP_DIR_MUST_EXIST);
    sizer->Add(dirpickergame,0,wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL,dpickBorder);

    label_cfgpath=new wxStaticText(this,wxID_ANY,_("Config directory:"));
    sizer->Add(label_cfgpath,0,wxLEFT|wxALIGN_CENTER_VERTICAL,8);

    s=wxString::Format(_("Select %s config path"),game->GetName().c_str());
    dirpickercfg=new wxDirPickerCtrl(this,DIR_PICKER_CFG,settings.ConfigPath,
                                     s,wxDefaultPosition,wxDefaultSize,
                                     wxDIRP_DEFAULT_STYLE|wxDIRP_USE_TEXTCTRL|wxDIRP_DIR_MUST_EXIST);
    sizer->Add(dirpickercfg,0,wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL,dpickBorder);

    wxStaticText* label_options=new wxStaticText(this,wxID_ANY,_("Game parameters:"));
    sizer->Add(label_options,0,wxLEFT|wxALIGN_CENTER_VERTICAL,8);

    text_ctrl_options=new wxTextCtrl(this,wxID_ANY,wxEmptyString);
    sizer->Add(text_ctrl_options,0,wxALL|wxEXPAND,4);
    text_ctrl_options->SetValue(settings.Options);

    label_expert = new wxStaticText(this,wxID_ANY,_("Configuration:"));
    sizer->Add(label_expert,0,wxLEFT|wxALIGN_CENTER_VERTICAL,8);

    checkbox_expert=new wxCheckBox(this,CHECK_GAME_EXPERT,_("Expert configuration"));
    sizer->Add(checkbox_expert,0,wxALL|wxALIGN_CENTER_VERTICAL,4);
    checkbox_expert->SetValue(settings.Expert);

    ToggleView(settings.Expert);

    SetSizer(sizer);
    sizer->AddGrowableCol(1);
}

CslGamePage::~CslGamePage()
{
    delete filepicker;
    delete dirpickergame;
    delete dirpickercfg;
}

bool CslGamePage::SaveSettings(wxString *message)
{
    wxString gamepath=dirpickergame->GetPath();
    wxString configpath=dirpickercfg->GetPath();

    if (!gamepath.IsEmpty() && !gamepath.EndsWith(wxString(PATHDIV)))
        gamepath+=PATHDIV;
    if (!configpath.IsEmpty() && !configpath.EndsWith(wxString(PATHDIV)))
        configpath+=PATHDIV;

    CslGameClientSettings settings(filepicker->GetPath(),gamepath,configpath,
                                   text_ctrl_options->GetValue(),checkbox_expert->GetValue());

    if (!m_game->ValidateClientSettings(&settings,message))
        return false;

    m_game->SetClientSettings(settings);

    return true;
}

void CslGamePage::ToggleView(bool expertView)
{
    static bool expert=false;
    expert=expertView;

    if (!CSL_CAP_CUSTOM_CONFIG(m_game->GetCapabilities()))
    {
        label_cfgpath->Show(false);
        dirpickercfg->Show(false);
    }
    else
    {
        label_cfgpath->Show(expert);
        dirpickercfg->Show(expert);
    }

    if (m_game->GetConfigType()==CslGame::CSL_CONFIG_DIR)
    {
        label_exe->Show(expert);
        filepicker->Show(expert);
    }
    else if (m_game->GetConfigType()==CslGame::CSL_CONFIG_EXE)
    {
        label_gamepath->Show(expert);
        dirpickergame->Show(expert);
    }

    sizer->Layout();
}

void CslGamePage::OnPicker(wxFileDirPickerEvent& event)
{
    switch (event.GetId())
    {
        case FILE_PICKER:
#ifdef __WXMAC__
            if (m_game->GetConfigType()==CslGame::CSL_CONFIG_DIR && dirpickergame->GetPath().IsEmpty())
#else
            if (dirpickergame->GetPath().IsEmpty())
#endif
                dirpickergame->SetPath(event.GetPath().BeforeLast(PATHDIVA));
            if (dirpickercfg->GetPath().IsEmpty())
                dirpickercfg->SetPath(event.GetPath().BeforeLast(PATHDIVA));
            break;
        case DIR_PICKER_GAME:
            if (dirpickergame->GetPath().IsEmpty())
                dirpickergame->SetPath(event.GetPath());
            if (dirpickercfg->GetPath().IsEmpty())
                dirpickercfg->SetPath(event.GetPath());
            break;
        case DIR_PICKER_CFG:
            if (dirpickercfg->GetPath().IsEmpty())
                dirpickercfg->SetPath(event.GetPath());
            break;
    }
}

void CslGamePage::OnCommandEvent(wxCommandEvent& event)
{
    switch (event.GetId())
    {
        case CHECK_GAME_EXPERT:
            ToggleView(event.IsChecked());
            break;
        default:
            break;
    }
}


CslDlgSettings::CslDlgSettings(wxWindow* parent,int id,const wxString& title,
                               const wxPoint& pos,const wxSize& size,long style):
        wxDialog(parent,id,title,pos,size,style),
        m_settings(*g_cslSettings)
{
    // begin wxGlade: CslDlgSettings::CslDlgSettings
    notebook_settings = new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
    notebook_pane_irc = new wxPanel(notebook_settings, wxID_ANY);
    notebook_pane_other = new wxPanel(notebook_settings, wxID_ANY);
    notebook_pane_colour = new wxPanel(notebook_settings, wxID_ANY);
    sizer_colours_staticbox = new wxStaticBox(notebook_pane_colour, -1, _("Server lists"));
    sizer_times_staticbox = new wxStaticBox(notebook_pane_other, -1, _("Times && Intervals"));
    sizer_threshold_staticbox = new wxStaticBox(notebook_pane_other, -1, _("Ping thresholds"));
    sizer_systray_staticbox = new wxStaticBox(notebook_pane_other, -1, _("System tray"));
    sizer_output_staticbox = new wxStaticBox(notebook_pane_other, -1, _("Game output"));
    sizer_network_staticbox = new wxStaticBox(notebook_pane_irc, -1, _("Networks"));
    sizer_1_staticbox = new wxStaticBox(notebook_pane_irc, -1, _("Server"));
    sizer_2_staticbox = new wxStaticBox(notebook_pane_irc, -1, _("Name"));
    sizer_3_staticbox = new wxStaticBox(notebook_pane_irc, -1, _("Autojoin channels"));
    notebook_pane_games = new wxPanel(notebook_settings, wxID_ANY);
    notebook_games = new wxListbook(notebook_pane_games, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
    button_colour_empty = new wxBitmapButton(notebook_pane_colour, BUTTON_COLOUR_EMPTY, wxNullBitmap);
    button_colour_mm1 = new wxBitmapButton(notebook_pane_colour, BUTTON_COLOUR_MM1, wxNullBitmap);
    button_colour_off = new wxBitmapButton(notebook_pane_colour, BUTTON_COLOUR_OFF, wxNullBitmap);
    button_colour_mm2 = new wxBitmapButton(notebook_pane_colour, BUTTON_COLOUR_MM2, wxNullBitmap);
    button_colour_full = new wxBitmapButton(notebook_pane_colour, BUTTON_COLOUR_FULL, wxNullBitmap);
    button_colour_mm3 = new wxBitmapButton(notebook_pane_colour, BUTTON_COLOUR_MM3, wxNullBitmap);
    spin_ctrl_update = new wxSpinCtrl(notebook_pane_other, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS|wxTE_AUTO_URL, 0, 100);
    checkbox_play_update = new wxCheckBox(notebook_pane_other, wxID_ANY, _("Don't update when playing"));
    spin_ctrl_wait = new wxSpinCtrl(notebook_pane_other, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS|wxTE_AUTO_URL, 0, 100);
    spin_ctrl_min_playtime = new wxSpinCtrl(notebook_pane_other, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS|wxTE_AUTO_URL, 0, 100);
    spin_ctrl_server_cleanup = new wxSpinCtrl(notebook_pane_other, SPIN_CLEANUP_SERVERS, wxT(""), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS|wxTE_AUTO_URL, 0, 100);
    checkbox_server_cleanup_favourites = new wxCheckBox(notebook_pane_other, wxID_ANY, _("Keep favourites"));
    checkbox_server_cleanup_stats = new wxCheckBox(notebook_pane_other, wxID_ANY, _("Keep servers with statistics"));
    spin_ctrl_tooltip_delay = new wxSpinCtrl(notebook_pane_other, SPIN_TOOLTIP_DELAY, wxT(""), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS|wxTE_AUTO_URL, 0, 100);
    spin_ctrl_ping_good = new wxSpinCtrl(notebook_pane_other, SPIN_PING_GOOD, wxT(""), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100);
    spin_ctrl_ping_bad = new wxSpinCtrl(notebook_pane_other, SPIN_PING_BAD, wxT(""), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100);
    checkbox_systray = new wxCheckBox(notebook_pane_other, CHECK_SYSTRAY, _("Use system tray icon"));
    checkbox_systray_close = new wxCheckBox(notebook_pane_other, wxID_ANY, _("Minimise on close button"));
    checkbox_game_output = new wxCheckBox(notebook_pane_other, CHECK_GAME_OUTPUT, _("Auto &save game output to:"));
    dirpicker_game_output = new wxDirPickerCtrl(notebook_pane_other, wxID_ANY, m_settings.gameOutputPath, _("Select game output path"), wxDefaultPosition, wxDefaultSize, wxDIRP_DEFAULT_STYLE|wxDIRP_USE_TEXTCTRL|wxDIRP_DIR_MUST_EXIST);
    tree_ctrl_network = new wxTreeCtrl(notebook_pane_irc, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_HAS_BUTTONS|wxTR_NO_LINES|wxTR_LINES_AT_ROOT|wxTR_DEFAULT_STYLE|wxSUNKEN_BORDER);
    label_7 = new wxStaticText(notebook_pane_irc, wxID_ANY, _("Server address:"));
    text_ctrl_1 = new wxTextCtrl(notebook_pane_irc, wxID_ANY, wxEmptyString);
    label_8 = new wxStaticText(notebook_pane_irc, wxID_ANY, _("Port:"));
    spin_ctrl_1 = new wxSpinCtrl(notebook_pane_irc, wxID_ANY, wxT("6667"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 65535);
    label_9 = new wxStaticText(notebook_pane_irc, wxID_ANY, _("Password:"));
    text_ctrl_2 = new wxTextCtrl(notebook_pane_irc, wxID_ANY, wxEmptyString);
    label_10 = new wxStaticText(notebook_pane_irc, wxID_ANY, _("Auto connect:"));
    checkbox_1 = new wxCheckBox(notebook_pane_irc, wxID_ANY, wxEmptyString);
    label_11 = new wxStaticText(notebook_pane_irc, wxID_ANY, _("Real name:"));
    text_ctrl_3 = new wxTextCtrl(notebook_pane_irc, wxID_ANY, wxEmptyString);
    label_12 = new wxStaticText(notebook_pane_irc, wxID_ANY, _("Ident:"));
    text_ctrl_4 = new wxTextCtrl(notebook_pane_irc, wxID_ANY, wxEmptyString);
    const wxString *list_box_1_choices = NULL;
    list_box_1 = new wxListBox(notebook_pane_irc, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, list_box_1_choices, 0);
    button_2 = new wxButton(notebook_pane_irc, wxID_UP, wxEmptyString);
    button_3 = new wxButton(notebook_pane_irc, wxID_DOWN, wxEmptyString);
    const wxString *list_box_2_choices = NULL;
    list_box_2 = new wxListBox(notebook_pane_irc, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, list_box_2_choices, 0);
    button_4 = new wxButton(notebook_pane_irc, wxID_UP, wxEmptyString);
    button_5 = new wxButton(notebook_pane_irc, wxID_DOWN, wxEmptyString);
    button_ok = new wxButton(this, wxID_OK, _("&Ok"));
    button_cancel = new wxButton(this, wxID_CANCEL, _("&Cancel"));

    set_properties();
    do_layout();
    // end wxGlade
}

void CslDlgSettings::set_properties()
{
    // begin wxGlade: CslDlgSettings::set_properties
    SetTitle(_("Cube Server Lister - Settings"));
    button_colour_empty->SetSize(button_colour_empty->GetBestSize());
    button_colour_mm1->SetSize(button_colour_mm1->GetBestSize());
    button_colour_off->SetSize(button_colour_off->GetBestSize());
    button_colour_mm2->SetSize(button_colour_mm2->GetBestSize());
    button_colour_full->SetSize(button_colour_full->GetBestSize());
    button_colour_mm3->SetSize(button_colour_mm3->GetBestSize());
    spin_ctrl_update->SetMinSize(wxSize(64, -1));
    spin_ctrl_wait->SetMinSize(wxSize(64, -1));
    spin_ctrl_min_playtime->SetMinSize(wxSize(64, -1));
    spin_ctrl_server_cleanup->SetMinSize(wxSize(64, -1));
    checkbox_server_cleanup_favourites->SetValue(1);
    checkbox_server_cleanup_stats->SetValue(1);
    dirpicker_game_output->Enable(false);
    tree_ctrl_network->SetMinSize(wxSize(180, -1));
    button_ok->SetDefault();
    // end wxGlade

    checkbox_play_update->SetValue(m_settings.dontUpdatePlaying);
    spin_ctrl_update->SetRange(CSL_UPDATE_INTERVAL_MIN/1000,CSL_UPDATE_INTERVAL_MAX/1000);
    spin_ctrl_update->SetValue(m_settings.updateInterval/1000);
    spin_ctrl_wait->SetRange(CSL_WAIT_SERVER_FULL_MIN,CSL_WAIT_SERVER_FULL_MAX);
    spin_ctrl_wait->SetValue(m_settings.waitServerFull);
    spin_ctrl_min_playtime->SetRange(CSL_MIN_PLAYTIME_MIN,CSL_MIN_PLAYTIME_MAX);
    spin_ctrl_min_playtime->SetValue(m_settings.minPlaytime);
    spin_ctrl_server_cleanup->SetRange(0,CSL_CLEANUP_SERVERS_MAX);
    spin_ctrl_server_cleanup->SetValue(m_settings.cleanupServers/86400);
    spin_ctrl_tooltip_delay->SetRange(CSL_TOOLTIP_DELAY_MIN,CSL_TOOLTIP_DELAY_MAX);
    spin_ctrl_tooltip_delay->SetValue(m_settings.tooltipDelay);
    checkbox_server_cleanup_favourites->SetValue(m_settings.cleanupServersKeepFav);
    checkbox_server_cleanup_stats->SetValue(m_settings.cleanupServersKeepStats);
    checkbox_server_cleanup_favourites->Enable(m_settings.cleanupServers!=0);
    checkbox_server_cleanup_stats->Enable(m_settings.cleanupServers!=0);
    spin_ctrl_ping_good->SetRange(0,9999);
    spin_ctrl_ping_good->SetValue(m_settings.pinggood);
    spin_ctrl_ping_bad->SetRange(0,9999);
    spin_ctrl_ping_bad->SetValue(m_settings.pingbad);

    SetButtonColour(button_colour_empty,button_ok,m_settings.colServerEmpty);
    SetButtonColour(button_colour_off,button_ok,m_settings.colServerOff);
    SetButtonColour(button_colour_full,button_ok,m_settings.colServerFull);
    SetButtonColour(button_colour_mm1,button_ok,m_settings.colServerMM1);
    SetButtonColour(button_colour_mm2,button_ok,m_settings.colServerMM2);
    SetButtonColour(button_colour_mm3,button_ok,m_settings.colServerMM3);

#ifndef __WXMAC__
    if (g_cslSettings->systray&CSL_USE_SYSTRAY)
    {
        checkbox_systray->SetValue(true);
        checkbox_systray_close->Enable();
    }
    else
    {
        checkbox_systray->SetValue(false);
        checkbox_systray_close->Enable(false);
    }
    checkbox_systray_close->SetValue((g_cslSettings->systray&CSL_SYSTRAY_CLOSE)!=0);
#endif

    checkbox_game_output->SetValue(m_settings.autoSaveOutput);
    dirpicker_game_output->SetPath(m_settings.gameOutputPath);
    dirpicker_game_output->Enable(m_settings.autoSaveOutput);
}


void CslDlgSettings::do_layout()
{
    wxImageList *imgList=new wxImageList(24,24,true);
    notebook_games->AssignImageList(imgList);

    vector<CslGame*>& games=::wxGetApp().GetCslEngine()->GetGames();
    loopv(games)
    {
        CslGame *game=games[i];
        notebook_games->AddPage(new CslGamePage(notebook_games,game),game->GetName());

        const char **icon=game->GetIcon(24);
        imgList->Add(icon ? wxBitmap(icon):wxBitmap(24,24));
        notebook_games->SetPageImage(i,i);
    }

    // begin wxGlade: CslDlgSettings::do_layout
    wxFlexGridSizer* grid_sizer_main = new wxFlexGridSizer(2, 1, 0, 0);
    wxFlexGridSizer* grid_sizer_button = new wxFlexGridSizer(1, 3, 0, 0);
    wxFlexGridSizer* grid_sizer_pane_irc = new wxFlexGridSizer(1, 2, 0, 0);
    wxFlexGridSizer* grid_sizer_1 = new wxFlexGridSizer(3, 1, 0, 0);
    wxStaticBoxSizer* sizer_3 = new wxStaticBoxSizer(sizer_3_staticbox, wxHORIZONTAL);
    wxFlexGridSizer* grid_sizer_6 = new wxFlexGridSizer(1, 2, 0, 0);
    wxFlexGridSizer* sizer_5 = new wxFlexGridSizer(2, 1, 0, 0);
    wxStaticBoxSizer* sizer_2 = new wxStaticBoxSizer(sizer_2_staticbox, wxHORIZONTAL);
    wxFlexGridSizer* grid_sizer_3 = new wxFlexGridSizer(2, 1, 0, 0);
    wxFlexGridSizer* grid_sizer_5 = new wxFlexGridSizer(1, 2, 0, 0);
    wxFlexGridSizer* sizer_4 = new wxFlexGridSizer(2, 1, 0, 0);
    wxFlexGridSizer* grid_sizer_4 = new wxFlexGridSizer(1, 4, 0, 0);
    wxStaticBoxSizer* sizer_1 = new wxStaticBoxSizer(sizer_1_staticbox, wxHORIZONTAL);
    wxFlexGridSizer* grid_sizer_2 = new wxFlexGridSizer(2, 4, 0, 0);
    wxStaticBoxSizer* sizer_network = new wxStaticBoxSizer(sizer_network_staticbox, wxHORIZONTAL);
    wxFlexGridSizer* grid_sizer_pane_other = new wxFlexGridSizer(4, 1, 0, 0);
    wxStaticBoxSizer* sizer_output = new wxStaticBoxSizer(sizer_output_staticbox, wxHORIZONTAL);
    wxFlexGridSizer* grid_sizer_output = new wxFlexGridSizer(1, 2, 0, 0);
    wxStaticBoxSizer* sizer_systray = new wxStaticBoxSizer(sizer_systray_staticbox, wxHORIZONTAL);
    wxFlexGridSizer* grid_sizer_systray = new wxFlexGridSizer(1, 3, 0, 0);
    wxStaticBoxSizer* sizer_threshold = new wxStaticBoxSizer(sizer_threshold_staticbox, wxHORIZONTAL);
    wxFlexGridSizer* grid_sizer_threshold = new wxFlexGridSizer(1, 5, 0, 0);
    wxStaticBoxSizer* sizer_times = new wxStaticBoxSizer(sizer_times_staticbox, wxHORIZONTAL);
    wxFlexGridSizer* grid_sizer_times = new wxFlexGridSizer(5, 3, 0, 0);
    wxFlexGridSizer* grid_sizer_server_cleanup = new wxFlexGridSizer(1, 2, 0, 0);
    wxFlexGridSizer* grid_sizer_pane_colours = new wxFlexGridSizer(1, 1, 0, 0);
    wxStaticBoxSizer* sizer_colours = new wxStaticBoxSizer(sizer_colours_staticbox, wxHORIZONTAL);
    wxFlexGridSizer* grid_sizer_colours = new wxFlexGridSizer(3, 5, 0, 0);
    wxBoxSizer* sizer_games = new wxBoxSizer(wxVERTICAL);
    sizer_games->Add(notebook_games, 1, wxALL|wxEXPAND|wxFIXED_MINSIZE, 4);
    notebook_pane_games->SetSizer(sizer_games);
    wxStaticText* label_colour_empty = new wxStaticText(notebook_pane_colour, wxID_ANY, _("Server empty"));
    grid_sizer_colours->Add(label_colour_empty, 0, wxALL|wxALIGN_CENTER_VERTICAL, 6);
    grid_sizer_colours->Add(button_colour_empty, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_colours->Add(1, 1, 0, 0, 0);
    wxStaticText* label_colour_open = new wxStaticText(notebook_pane_colour, wxID_ANY, _("Server veto mode"));
    grid_sizer_colours->Add(label_colour_open, 0, wxALL|wxALIGN_CENTER_VERTICAL, 6);
    grid_sizer_colours->Add(button_colour_mm1, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    wxStaticText* label_colour_offline = new wxStaticText(notebook_pane_colour, wxID_ANY, _("Server offline"));
    grid_sizer_colours->Add(label_colour_offline, 0, wxALL|wxALIGN_CENTER_VERTICAL, 6);
    grid_sizer_colours->Add(button_colour_off, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_colours->Add(1, 1, 0, 0, 0);
    wxStaticText* label_colour_limited = new wxStaticText(notebook_pane_colour, wxID_ANY, _("Server game locked"));
    grid_sizer_colours->Add(label_colour_limited, 0, wxALL|wxALIGN_CENTER_VERTICAL, 6);
    grid_sizer_colours->Add(button_colour_mm2, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    wxStaticText* label_colour_full = new wxStaticText(notebook_pane_colour, wxID_ANY, _("Server full"));
    grid_sizer_colours->Add(label_colour_full, 0, wxALL|wxALIGN_CENTER_VERTICAL, 6);
    grid_sizer_colours->Add(button_colour_full, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_colours->Add(1, 1, 0, 0, 0);
    wxStaticText* label_colour_restricted = new wxStaticText(notebook_pane_colour, wxID_ANY, _("Server restricted (password, banned, private)"));
    grid_sizer_colours->Add(label_colour_restricted, 0, wxALL|wxALIGN_CENTER_VERTICAL, 6);
    grid_sizer_colours->Add(button_colour_mm3, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_colours->AddGrowableCol(2);
    sizer_colours->Add(grid_sizer_colours, 1, wxEXPAND, 0);
    grid_sizer_pane_colours->Add(sizer_colours, 1, wxALL|wxEXPAND, 4);
    notebook_pane_colour->SetSizer(grid_sizer_pane_colours);
    grid_sizer_pane_colours->AddGrowableCol(0);
    grid_sizer_pane_colours->AddGrowableCol(1);
    wxStaticText* label_update_static = new wxStaticText(notebook_pane_other, wxID_ANY, _("Update interval (seconds):"));
    grid_sizer_times->Add(label_update_static, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_times->Add(spin_ctrl_update, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_times->Add(checkbox_play_update, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    wxStaticText* label_wait_static = new wxStaticText(notebook_pane_other, wxID_ANY, _("Wait on full server (seconds):"));
    grid_sizer_times->Add(label_wait_static, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_times->Add(spin_ctrl_wait, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_times->Add(1, 1, 0, 0, 0);
    wxStaticText* label_min_playtime_static = new wxStaticText(notebook_pane_other, wxID_ANY, _("Minimum playtime for statistics (seconds):"));
    grid_sizer_times->Add(label_min_playtime_static, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_times->Add(spin_ctrl_min_playtime, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_times->Add(1, 1, 0, 0, 0);
    wxStaticText* label_server_cleanup = new wxStaticText(notebook_pane_other, wxID_ANY, _("Cleanup dead servers (days)"));
    grid_sizer_times->Add(label_server_cleanup, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_times->Add(spin_ctrl_server_cleanup, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_server_cleanup->Add(checkbox_server_cleanup_favourites, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_server_cleanup->Add(checkbox_server_cleanup_stats, 0, wxALL, 4);
    grid_sizer_times->Add(grid_sizer_server_cleanup, 1, wxALIGN_CENTER_VERTICAL, 0);
    wxStaticText* label_tooltip_delay = new wxStaticText(notebook_pane_other, wxID_ANY, _("Tooltip delay (milliseconds)"));
    grid_sizer_times->Add(label_tooltip_delay, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_times->Add(spin_ctrl_tooltip_delay, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    wxStaticText* label_tooltip_delay_help = new wxStaticText(notebook_pane_other, wxID_ANY, _("To disable tooltips, set this value to 0."));
    grid_sizer_times->Add(label_tooltip_delay_help, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_times->AddGrowableCol(1);
    sizer_times->Add(grid_sizer_times, 1, wxEXPAND, 0);
    grid_sizer_pane_other->Add(sizer_times, 1, wxALL|wxEXPAND, 4);
    wxStaticText* label_ping_good = new wxStaticText(notebook_pane_other, wxID_ANY, _("Good"));
    grid_sizer_threshold->Add(label_ping_good, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_threshold->Add(spin_ctrl_ping_good, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_threshold->Add(1, 1, 0, 0, 0);
    wxStaticText* label_ping_bad = new wxStaticText(notebook_pane_other, wxID_ANY, _("Bad"));
    grid_sizer_threshold->Add(label_ping_bad, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_threshold->Add(spin_ctrl_ping_bad, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_threshold->AddGrowableCol(2);
    sizer_threshold->Add(grid_sizer_threshold, 1, wxEXPAND, 0);
    grid_sizer_pane_other->Add(sizer_threshold, 1, wxALL|wxEXPAND, 4);
    grid_sizer_systray->Add(checkbox_systray, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_systray->Add(checkbox_systray_close, 0, wxALL|wxALIGN_RIGHT, 4);
    grid_sizer_systray->AddGrowableCol(0);
    grid_sizer_systray->AddGrowableCol(1);
    grid_sizer_systray->AddGrowableCol(2);
    sizer_systray->Add(grid_sizer_systray, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL, 0);
    grid_sizer_pane_other->Add(sizer_systray, 1, wxALL|wxEXPAND, 4);
    grid_sizer_output->Add(checkbox_game_output, 0, wxALL, 4);
    grid_sizer_output->Add(dirpicker_game_output, 1, wxEXPAND, 0);
    grid_sizer_output->AddGrowableCol(1);
    sizer_output->Add(grid_sizer_output, 1, wxEXPAND, 0);
    grid_sizer_pane_other->Add(sizer_output, 1, wxALL|wxEXPAND, 4);
    notebook_pane_other->SetSizer(grid_sizer_pane_other);
    grid_sizer_pane_other->AddGrowableCol(0);
    sizer_network->Add(tree_ctrl_network, 1, wxEXPAND, 0);
    grid_sizer_pane_irc->Add(sizer_network, 1, wxALL|wxEXPAND, 4);
    grid_sizer_2->Add(label_7, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_2->Add(text_ctrl_1, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_2->Add(label_8, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_2->Add(spin_ctrl_1, 0, wxALL, 4);
    grid_sizer_2->Add(label_9, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_2->Add(text_ctrl_2, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_2->Add(label_10, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_2->Add(checkbox_1, 0, wxALL, 4);
    grid_sizer_2->AddGrowableCol(1);
    sizer_1->Add(grid_sizer_2, 1, wxEXPAND, 0);
    grid_sizer_1->Add(sizer_1, 1, wxRIGHT|wxTOP|wxBOTTOM|wxEXPAND, 4);
    grid_sizer_4->Add(label_11, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_4->Add(text_ctrl_3, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_4->Add(label_12, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_4->Add(text_ctrl_4, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_4->AddGrowableCol(1);
    grid_sizer_4->AddGrowableCol(3);
    grid_sizer_3->Add(grid_sizer_4, 1, wxEXPAND, 0);
    grid_sizer_5->Add(list_box_1, 0, wxALL|wxEXPAND, 4);
    sizer_4->Add(button_2, 0, wxALL|wxALIGN_BOTTOM, 4);
    sizer_4->Add(button_3, 0, wxALL, 4);
    sizer_4->AddGrowableRow(0);
    sizer_4->AddGrowableRow(1);
    grid_sizer_5->Add(sizer_4, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL, 0);
    grid_sizer_5->AddGrowableRow(0);
    grid_sizer_5->AddGrowableCol(0);
    grid_sizer_3->Add(grid_sizer_5, 1, wxEXPAND, 0);
    grid_sizer_3->AddGrowableRow(1);
    grid_sizer_3->AddGrowableCol(0);
    sizer_2->Add(grid_sizer_3, 1, wxEXPAND, 0);
    grid_sizer_1->Add(sizer_2, 1, wxRIGHT|wxTOP|wxBOTTOM|wxEXPAND, 4);
    grid_sizer_6->Add(list_box_2, 0, wxALL|wxEXPAND, 4);
    sizer_5->Add(button_4, 0, wxALL|wxALIGN_BOTTOM, 4);
    sizer_5->Add(button_5, 0, wxALL, 4);
    sizer_5->AddGrowableRow(0);
    sizer_5->AddGrowableRow(1);
    grid_sizer_6->Add(sizer_5, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL, 0);
    grid_sizer_6->AddGrowableRow(0);
    grid_sizer_6->AddGrowableCol(0);
    sizer_3->Add(grid_sizer_6, 1, wxEXPAND, 0);
    grid_sizer_1->Add(sizer_3, 1, wxRIGHT|wxTOP|wxBOTTOM|wxEXPAND, 4);
    grid_sizer_1->AddGrowableRow(1);
    grid_sizer_1->AddGrowableRow(2);
    grid_sizer_1->AddGrowableCol(0);
    grid_sizer_pane_irc->Add(grid_sizer_1, 1, wxEXPAND, 0);
    notebook_pane_irc->SetSizer(grid_sizer_pane_irc);
    grid_sizer_pane_irc->AddGrowableRow(0);
    grid_sizer_pane_irc->AddGrowableCol(1);
    notebook_settings->AddPage(notebook_pane_games, _("Games"));
    notebook_settings->AddPage(notebook_pane_colour, _("Colours"));
    notebook_settings->AddPage(notebook_pane_other, _("Other"));
    notebook_settings->AddPage(notebook_pane_irc, _("IRC"));
    grid_sizer_main->Add(notebook_settings, 1, wxALL|wxEXPAND, 4);
    grid_sizer_button->Add(1, 1, 0, 0, 0);
    grid_sizer_button->Add(button_ok, 0, wxALL, 4);
    grid_sizer_button->Add(button_cancel, 0, wxALL, 4);
    grid_sizer_button->AddGrowableCol(0);
    grid_sizer_main->Add(grid_sizer_button, 1, wxBOTTOM|wxEXPAND, 4);
    SetSizer(grid_sizer_main);
    grid_sizer_main->Fit(this);
    grid_sizer_main->AddGrowableRow(0);
    grid_sizer_main->AddGrowableCol(0);
    Layout();
    // end wxGlade

    // hmm ...
#ifdef __WXMSW__
    const wxSize& best=notebook_settings->GetBestSize();
    notebook_settings->SetMinSize(wxSize(best.x,best.y+16));
#endif
#ifdef __WXMAC__
    grid_sizer_pane_other->Hide(sizer_systray);
    Layout();
#endif

    grid_sizer_main->SetSizeHints(this);

    CentreOnParent();
}

void CslDlgSettings::OnSpinCtrl(wxSpinEvent& event)
{
    wxInt32 i1,i2;

    switch (event.GetId())
    {
        case SPIN_CLEANUP_SERVERS:
        {
            i1=event.GetPosition();
            checkbox_server_cleanup_favourites->Enable(i1!=0);
            checkbox_server_cleanup_stats->Enable(i1!=0);
            break;
        }
        case SPIN_TOOLTIP_DELAY:
        {
            i1=event.GetPosition();

            if (i1==m_settings.tooltipDelay)
                break;

            if (i1>m_settings.tooltipDelay)
                i1=max(i1-m_settings.tooltipDelay,CSL_TOOLTIP_DELAY_STEP);
            else
                i1=min(i1-m_settings.tooltipDelay,-CSL_TOOLTIP_DELAY_STEP);

            m_settings.tooltipDelay+=(i1/CSL_TOOLTIP_DELAY_STEP*CSL_TOOLTIP_DELAY_STEP);
            spin_ctrl_tooltip_delay->SetValue(m_settings.tooltipDelay);
            break;
        }
        case SPIN_PING_GOOD:
        {
            i1=event.GetPosition();
            i2=spin_ctrl_ping_bad->GetValue();
            if (i1>i2)
                spin_ctrl_ping_bad->SetValue(i1);
            break;
        }
        case SPIN_PING_BAD:
        {
            i1=event.GetPosition();
            i2=spin_ctrl_ping_good->GetValue();
            if (i1<i2)
                spin_ctrl_ping_good->SetValue(i1);
            break;
        }
    }

    event.Skip();
}

void CslDlgSettings::OnPicker(wxFileDirPickerEvent& event)
{
    if (dirpicker_game_output->GetPath().IsEmpty())
        dirpicker_game_output->SetPath(event.GetPath());
}

void CslDlgSettings::OnCommandEvent(wxCommandEvent& event)
{
    wxColour colnew,*colour=NULL;
    wxBitmapButton *colButton=NULL;

    switch (event.GetId())
    {
        case BUTTON_COLOUR_EMPTY:
            colour=&m_settings.colServerEmpty;
            colButton=button_colour_empty;
        case BUTTON_COLOUR_OFF:
            if (!colour)
            {
                colour=&m_settings.colServerOff;
                colButton=button_colour_off;
            }
        case BUTTON_COLOUR_FULL:
            if (!colour)
            {
                colour=&m_settings.colServerFull;
                colButton=button_colour_full;
            }
        case BUTTON_COLOUR_MM1:
            if (!colour)
            {
                colour=&m_settings.colServerMM1;
                colButton=button_colour_mm1;
            }
        case BUTTON_COLOUR_MM2:
            if (!colour)
            {
                colour=&m_settings.colServerMM2;
                colButton=button_colour_mm2;
            }
        case BUTTON_COLOUR_MM3:
            if (!colour)
            {
                colour=&m_settings.colServerMM3;
                colButton=button_colour_mm3;
            }
            colnew=wxGetColourFromUser(this,*colour);
            if (!colnew.Ok())
                return;
            *colour=colnew;
            SetButtonColour(colButton,button_ok,*colour);
            break;

        case CHECK_SYSTRAY:
            checkbox_systray_close->Enable(event.IsChecked());
            break;

        case CHECK_GAME_OUTPUT:
            dirpicker_game_output->Enable(event.IsChecked());
            break;

        case wxID_CANCEL:
            break;
        case wxID_OK:
        {
            wxString msg;

            m_settings.updateInterval=spin_ctrl_update->GetValue()*1000;
            m_settings.waitServerFull=spin_ctrl_wait->GetValue();
            m_settings.dontUpdatePlaying=checkbox_play_update->IsChecked();
            m_settings.minPlaytime=spin_ctrl_min_playtime->GetValue();
            m_settings.cleanupServers=spin_ctrl_server_cleanup->GetValue()*86400;
            m_settings.cleanupServersKeepFav=checkbox_server_cleanup_favourites->GetValue();
            m_settings.cleanupServersKeepStats=checkbox_server_cleanup_stats->GetValue();
            m_settings.tooltipDelay=spin_ctrl_tooltip_delay->GetValue();
            m_settings.pinggood=spin_ctrl_ping_good->GetValue();
            m_settings.pingbad=spin_ctrl_ping_bad->GetValue();
            m_settings.systray=checkbox_systray->GetValue() ? CSL_USE_SYSTRAY : 0;
            m_settings.systray|=checkbox_systray_close->GetValue() ? CSL_SYSTRAY_CLOSE : 0;
            m_settings.gameOutputPath=dirpicker_game_output->GetPath();
            m_settings.autoSaveOutput=checkbox_game_output->IsChecked();

            if (!ValidateSettings())
                return;
            for (wxUint32 i=0;i<notebook_games->GetPageCount();i++)
            {
                if (!((CslGamePage*)notebook_games->GetPage(i))->SaveSettings(&msg))
                {
                    wxMessageBox(msg,_("Error"),wxICON_ERROR,this);
                    return;
                }
            }

            *g_cslSettings=m_settings;

            EndModal(wxID_OK);
            break;
        }
    }

    event.Skip();
}

void CslDlgSettings::SetButtonColour(wxBitmapButton *button,wxButton *refButton,wxColour& colour)
{
    wxInt32 w,h;
    wxColour col=colour;
    refButton->GetClientSize(&w,&h);

    wxMemoryDC memDC;
    wxBitmap bmp(w-16,h-8);

    memDC.SelectObject(bmp);
    memDC.SetBrush(wxBrush(col));
    memDC.DrawRectangle(0,0,w-16,h-8);
    memDC.SelectObject(wxNullBitmap);

    button->SetBitmapLabel(bmp);
}

bool CslDlgSettings::ValidateSettings()
{
    if (m_settings.pinggood>m_settings.pingbad)
    {
        wxMessageBox(_("Threshold for good ping can't be higher than\n" \
                       _L_"threshold for bad ping."),_("Error"),wxICON_ERROR,this);
        return false;
    }

    if (checkbox_game_output->IsChecked() && m_settings.gameOutputPath.IsEmpty())
    {
        wxMessageBox(_("Select a folder for game output logs or disable autosave."),
                     _("Error"),wxICON_ERROR,this);
        return false;
    }

    return true;
}
