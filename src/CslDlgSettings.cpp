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

#include "Csl.h"
#include "CslEngine.h"
#include "CslApp.h"
#include "CslArt.h"
#include "CslTTS.h"
#include "CslPanelGameSettings.h"
#include "CslDlgSettings.h"


BEGIN_EVENT_TABLE(CslDlgSettings,wxDialog)
    EVT_BUTTON(wxID_ANY,CslDlgSettings::OnCommandEvent)
    EVT_RADIOBUTTON(wxID_ANY,CslDlgSettings::OnCommandEvent)
    EVT_SPINCTRL(wxID_ANY,CslDlgSettings::OnSpinCtrl)
    EVT_CHECKBOX(wxID_ANY,CslDlgSettings::OnCommandEvent)
    EVT_DIRPICKER_CHANGED(wxID_ANY,CslDlgSettings::OnPicker)
END_EVENT_TABLE()


enum
{
    /* server lists */
    BUTTON_COLOUR_SERVER_EMPTY = wxID_HIGHEST + 1,
    BUTTON_COLOUR_SERVER_OFFLINE,
    BUTTON_COLOUR_SERVER_FULL,
    BUTTON_COLOUR_SERVER_MM1,
    BUTTON_COLOUR_SERVER_MM2,
    BUTTON_COLOUR_SERVER_MM3,
    /* player lists */
    BUTTON_COLOUR_PLAYER_MASTER,
    BUTTON_COLOUR_PLAYER_AUTH,
    BUTTON_COLOUR_PLAYER_ADMIN,
    BUTTON_COLOUR_PLAYER_SPECTATOR,

    SPIN_CLEANUP_SERVERS,

    SPIN_TOOLTIP_DELAY,

    SPIN_PING_GOOD,
    SPIN_PING_BAD,

    CHECK_SYSTRAY,

    CHECK_TTS,
    BUTTON_TEST_TTS,

    CHECK_GAME_OUTPUT
};


CslDlgSettings::CslDlgSettings(wxWindow* parent,int id,const wxString& title,
                               const wxPoint& pos,const wxSize& size,long style):
        wxDialog(parent, id, title, pos, size, style)
{
    m_settings=CslGetSettings();

    // begin wxGlade: CslDlgSettings::CslDlgSettings
    notebook_settings = new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
    notebook_pane_irc = new wxPanel(notebook_settings, wxID_ANY);
    notebook_pane_colour = new wxPanel(notebook_settings, wxID_ANY);
    sizer_colours_server_staticbox = new wxStaticBox(notebook_pane_colour, -1, _("Server"));
    sizer_colours_player_staticbox = new wxStaticBox(notebook_pane_colour, -1, _("Player"));
    sizer_times_staticbox = new wxStaticBox(notebook_pane_irc, -1, _("Times && Intervals"));
    sizer_systray_staticbox = new wxStaticBox(notebook_pane_irc, -1, _("System tray"));
    sizer_tts_staticbox = new wxStaticBox(notebook_pane_irc, -1, _("Text to speech"));
    sizer_ping_staticbox = new wxStaticBox(notebook_pane_irc, -1, _("Ping thresholds"));
    sizer_output_staticbox = new wxStaticBox(notebook_pane_irc, -1, _("Game output"));
    notebook_pane_games = new wxPanel(notebook_settings, wxID_ANY);
    notebook_games = new wxListbook(notebook_pane_games, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
    button_colour_server_empty = new wxBitmapButton(notebook_pane_colour, BUTTON_COLOUR_SERVER_EMPTY, wxNullBitmap);
    button_colour_server_mm1 = new wxBitmapButton(notebook_pane_colour, BUTTON_COLOUR_SERVER_MM1, wxNullBitmap);
    button_colour_server_offline = new wxBitmapButton(notebook_pane_colour, BUTTON_COLOUR_SERVER_OFFLINE, wxNullBitmap);
    button_colour_server_mm2 = new wxBitmapButton(notebook_pane_colour, BUTTON_COLOUR_SERVER_MM2, wxNullBitmap);
    button_colour_server_full = new wxBitmapButton(notebook_pane_colour, BUTTON_COLOUR_SERVER_FULL, wxNullBitmap);
    button_colour_server_mm3 = new wxBitmapButton(notebook_pane_colour, BUTTON_COLOUR_SERVER_MM3, wxNullBitmap);
    button_colour_player_master = new wxBitmapButton(notebook_pane_colour, BUTTON_COLOUR_PLAYER_MASTER, wxNullBitmap);
    button_colour_player_auth = new wxBitmapButton(notebook_pane_colour, BUTTON_COLOUR_PLAYER_AUTH, wxNullBitmap);
    button_colour_player_admin = new wxBitmapButton(notebook_pane_colour, BUTTON_COLOUR_PLAYER_ADMIN, wxNullBitmap);
    button_colour_player_spectator = new wxBitmapButton(notebook_pane_colour, BUTTON_COLOUR_PLAYER_SPECTATOR, wxNullBitmap);
    spin_ctrl_update = new wxSpinCtrl(notebook_pane_irc, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS|wxTE_AUTO_URL, 0, 100);
    checkbox_play_update = new wxCheckBox(notebook_pane_irc, wxID_ANY, _("Don't update when playing"));
    spin_ctrl_wait = new wxSpinCtrl(notebook_pane_irc, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS|wxTE_AUTO_URL, 0, 100);
    spin_ctrl_min_playtime = new wxSpinCtrl(notebook_pane_irc, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS|wxTE_AUTO_URL, 0, 100);
    spin_ctrl_server_cleanup = new wxSpinCtrl(notebook_pane_irc, SPIN_CLEANUP_SERVERS, wxT(""), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS|wxTE_AUTO_URL, 0, 100);
    checkbox_server_cleanup_favourites = new wxCheckBox(notebook_pane_irc, wxID_ANY, _("Keep favourites"));
    checkbox_server_cleanup_stats = new wxCheckBox(notebook_pane_irc, wxID_ANY, _("Keep servers with statistics"));
    spin_ctrl_tooltip_delay = new wxSpinCtrl(notebook_pane_irc, SPIN_TOOLTIP_DELAY, wxT(""), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS|wxTE_AUTO_URL, 0, 100);
    checkbox_systray = new wxCheckBox(notebook_pane_irc, CHECK_SYSTRAY, _("Enable to system tray icon"));
    checkbox_systray_close = new wxCheckBox(notebook_pane_irc, wxID_ANY, _("Minimise on close"));
    checkbox_tts = new wxCheckBox(notebook_pane_irc, CHECK_TTS, _("Enable text to speech"));
    spin_ctrl_tts_volume = new wxSpinCtrl(notebook_pane_irc, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS|wxTE_AUTO_URL, 1, 100);
    button_test_tts = new wxButton(notebook_pane_irc, BUTTON_TEST_TTS, _("Test"));
    spin_ctrl_ping_good = new wxSpinCtrl(notebook_pane_irc, SPIN_PING_GOOD, wxT(""), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS|wxTE_AUTO_URL, 0, 100);
    spin_ctrl_ping_bad = new wxSpinCtrl(notebook_pane_irc, SPIN_PING_BAD, wxT(""), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS|wxTE_AUTO_URL, 0, 100);
    checkbox_game_output = new wxCheckBox(notebook_pane_irc, CHECK_GAME_OUTPUT, _("Auto &save game output to:"));
    dirpicker_game_output = new wxDirPickerCtrl(notebook_pane_irc, wxID_ANY, m_settings.GameOutputPath, _("Select game output path"), wxDefaultPosition, wxDefaultSize, wxDIRP_DEFAULT_STYLE|wxDIRP_USE_TEXTCTRL|wxDIRP_DIR_MUST_EXIST);
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
    button_colour_server_empty->SetSize(button_colour_server_empty->GetBestSize());
    button_colour_server_mm1->SetSize(button_colour_server_mm1->GetBestSize());
    button_colour_server_offline->SetSize(button_colour_server_offline->GetBestSize());
    button_colour_server_mm2->SetSize(button_colour_server_mm2->GetBestSize());
    button_colour_server_full->SetSize(button_colour_server_full->GetBestSize());
    button_colour_server_mm3->SetSize(button_colour_server_mm3->GetBestSize());
    button_colour_player_master->SetSize(button_colour_player_master->GetBestSize());
    button_colour_player_auth->SetSize(button_colour_player_auth->GetBestSize());
    button_colour_player_admin->SetSize(button_colour_player_admin->GetBestSize());
    button_colour_player_spectator->SetSize(button_colour_player_spectator->GetBestSize());
    spin_ctrl_update->SetMinSize(wxSize(64, -1));
    spin_ctrl_wait->SetMinSize(wxSize(64, -1));
    spin_ctrl_min_playtime->SetMinSize(wxSize(64, -1));
    spin_ctrl_server_cleanup->SetMinSize(wxSize(64, -1));
    checkbox_server_cleanup_favourites->SetValue(1);
    checkbox_server_cleanup_stats->SetValue(1);
    spin_ctrl_tooltip_delay->SetMinSize(wxSize(64, -1));
    spin_ctrl_tts_volume->SetMinSize(wxSize(61, -1));
    spin_ctrl_ping_good->SetMinSize(wxSize(64, -1));
    spin_ctrl_ping_bad->SetMinSize(wxSize(64, -1));
    dirpicker_game_output->Enable(false);
    // end wxGlade

    checkbox_play_update->SetValue(m_settings.DontUpdatePlaying);
    spin_ctrl_update->SetRange(CSL_UPDATE_INTERVAL_MIN/1000,CSL_UPDATE_INTERVAL_MAX/1000);
    spin_ctrl_update->SetValue(m_settings.UpdateInterval/1000);
    spin_ctrl_wait->SetRange(CSL_WAIT_SERVER_FULL_MIN,CSL_WAIT_SERVER_FULL_MAX);
    spin_ctrl_wait->SetValue(m_settings.WaitServerFull);
    spin_ctrl_min_playtime->SetRange(CSL_MIN_PLAYTIME_MIN,CSL_MIN_PLAYTIME_MAX);
    spin_ctrl_min_playtime->SetValue(m_settings.MinPlaytime);
    spin_ctrl_server_cleanup->SetRange(0,CSL_CLEANUP_SERVERS_MAX);
    spin_ctrl_server_cleanup->SetValue(m_settings.CleanupServers/86400);
    spin_ctrl_tooltip_delay->SetRange(CSL_TOOLTIP_DELAY_MIN,CSL_TOOLTIP_DELAY_MAX);
    spin_ctrl_tooltip_delay->SetValue(m_settings.TooltipDelay);
    checkbox_server_cleanup_favourites->SetValue(m_settings.CleanupServersKeepFav);
    checkbox_server_cleanup_stats->SetValue(m_settings.CleanupServersKeepStats);
    checkbox_server_cleanup_favourites->Enable(m_settings.CleanupServers!=0);
    checkbox_server_cleanup_stats->Enable(m_settings.CleanupServers!=0);
    spin_ctrl_ping_good->SetRange(0,9999);
    spin_ctrl_ping_good->SetValue(m_settings.PingGood);
    spin_ctrl_ping_bad->SetRange(0,9999);
    spin_ctrl_ping_bad->SetValue(m_settings.PingBad);

    /* server lists */
    SetButtonColour(button_colour_server_empty, button_ok, m_settings.ColServerEmpty);
    SetButtonColour(button_colour_server_offline, button_ok, m_settings.ColServerOff);
    SetButtonColour(button_colour_server_full, button_ok, m_settings.ColServerFull);
    SetButtonColour(button_colour_server_mm1, button_ok, m_settings.ColServerMM1);
    SetButtonColour(button_colour_server_mm2, button_ok, m_settings.ColServerMM2);
    SetButtonColour(button_colour_server_mm3, button_ok, m_settings.ColServerMM3);
    /* player lists */
    SetButtonColour(button_colour_player_master, button_ok, m_settings.ColPlayerMaster);
    SetButtonColour(button_colour_player_auth, button_ok, m_settings.ColPlayerAuth);
    SetButtonColour(button_colour_player_admin, button_ok, m_settings.ColPlayerAdmin);
    SetButtonColour(button_colour_player_spectator, button_ok, m_settings.ColPlayerSpectator);

#ifndef __WXMAC__
    if (CSL_FLAG_CHECK(CslGetSettings().Systray, CSL_USE_SYSTRAY))
    {
        checkbox_systray->SetValue(true);
        checkbox_systray_close->Enable();
    }
    else
    {
        checkbox_systray->SetValue(false);
        checkbox_systray_close->Enable(false);
    }
    checkbox_systray_close->SetValue(CSL_FLAG_CHECK(CslGetSettings().Systray, CSL_SYSTRAY_CLOSE));
#endif

    checkbox_tts->SetValue(CslGetSettings().TTS);
    spin_ctrl_tts_volume->SetValue(CslGetSettings().TTSVolume);
    checkbox_tts->Enable(CslTTS::IsOk());
    spin_ctrl_tts_volume->Enable(CslTTS::IsOk() && CslGetSettings().TTS);
    button_test_tts->Enable(CslTTS::IsOk() && CslGetSettings().TTS);

    checkbox_game_output->SetValue(m_settings.AutoSaveOutput);
    dirpicker_game_output->SetPath(m_settings.GameOutputPath);
    dirpicker_game_output->Enable(m_settings.AutoSaveOutput);

    CSL_SET_WINDOW_ICON();
}

void CslDlgSettings::do_layout()
{
    wxImageList *imgList=new wxImageList(24,24,true);
    notebook_games->AssignImageList(imgList);

    CslArrayCslGame& games=::wxGetApp().GetCslEngine()->GetGames();
    loopv(games)
    {
        CslGame *game=games[i];

        notebook_games->AddPage(new CslPanelGameSettings(notebook_games, game), game->GetName());

        const CslGameIcon *icon=game->GetIcon(24);

        wxBitmap bmp = icon ?
            BitmapFromData(Csl2wxBitmapType(icon->Type), icon->Data, icon->DataSize) :
            wxNullBitmap;

        if (bmp.IsOk())
        {
            imgList->Add(bmp);
            notebook_games->SetPageImage(i,i);
        }
        else
            imgList->Add(wxBitmap(24,24));
    }

    // begin wxGlade: CslDlgSettings::do_layout
    wxFlexGridSizer* grid_sizer_main = new wxFlexGridSizer(2, 1, 0, 0);
    wxFlexGridSizer* grid_sizer_button = new wxFlexGridSizer(1, 3, 0, 0);
    wxFlexGridSizer* grid_sizer_pane_other = new wxFlexGridSizer(5, 1, 0, 0);
    wxStaticBoxSizer* sizer_output = new wxStaticBoxSizer(sizer_output_staticbox, wxHORIZONTAL);
    wxFlexGridSizer* grid_sizer_output = new wxFlexGridSizer(1, 2, 0, 0);
    wxStaticBoxSizer* sizer_ping = new wxStaticBoxSizer(sizer_ping_staticbox, wxHORIZONTAL);
    wxFlexGridSizer* grid_sizer_ping = new wxFlexGridSizer(1, 4, 0, 0);
    wxStaticBoxSizer* sizer_tts = new wxStaticBoxSizer(sizer_tts_staticbox, wxHORIZONTAL);
    wxFlexGridSizer* grid_sizer_tts = new wxFlexGridSizer(1, 4, 0, 0);
    wxStaticBoxSizer* sizer_systray = new wxStaticBoxSizer(sizer_systray_staticbox, wxHORIZONTAL);
    wxFlexGridSizer* grid_sizer_systray = new wxFlexGridSizer(1, 2, 0, 0);
    wxStaticBoxSizer* sizer_times = new wxStaticBoxSizer(sizer_times_staticbox, wxHORIZONTAL);
    wxFlexGridSizer* grid_sizer_times = new wxFlexGridSizer(5, 3, 0, 0);
    wxFlexGridSizer* grid_sizer_server_cleanup = new wxFlexGridSizer(1, 2, 0, 0);
    wxFlexGridSizer* grid_sizer_pane_colours = new wxFlexGridSizer(2, 1, 0, 0);
    wxStaticBoxSizer* sizer_colours_player = new wxStaticBoxSizer(sizer_colours_player_staticbox, wxVERTICAL);
    wxFlexGridSizer* grid_sizer_colours_player = new wxFlexGridSizer(2, 5, 0, 0);
    wxStaticBoxSizer* sizer_colours_server = new wxStaticBoxSizer(sizer_colours_server_staticbox, wxVERTICAL);
    wxFlexGridSizer* grid_sizer_colours_server = new wxFlexGridSizer(3, 5, 0, 0);
    wxBoxSizer* sizer_games = new wxBoxSizer(wxVERTICAL);
    sizer_games->Add(notebook_games, 1, wxALL|wxEXPAND|wxFIXED_MINSIZE, 4);
    notebook_pane_games->SetSizer(sizer_games);
    wxStaticText* label_colour_server_empty = new wxStaticText(notebook_pane_colour, wxID_ANY, _("Empty"));
    grid_sizer_colours_server->Add(label_colour_server_empty, 0, wxALL|wxALIGN_CENTER_VERTICAL, 6);
    grid_sizer_colours_server->Add(button_colour_server_empty, 0, wxALL|wxEXPAND|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_colours_server->Add(1, 1, 0, 0, 0);
    wxStaticText* label_colour_server_mm1 = new wxStaticText(notebook_pane_colour, wxID_ANY, _("Master control"));
    grid_sizer_colours_server->Add(label_colour_server_mm1, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 6);
    grid_sizer_colours_server->Add(button_colour_server_mm1, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    wxStaticText* label_colour_server_offline = new wxStaticText(notebook_pane_colour, wxID_ANY, _("Offline"));
    grid_sizer_colours_server->Add(label_colour_server_offline, 0, wxALL|wxALIGN_CENTER_VERTICAL, 6);
    grid_sizer_colours_server->Add(button_colour_server_offline, 0, wxALL|wxEXPAND|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_colours_server->Add(1, 1, 0, 0, 0);
    wxStaticText* label_colour_server_mm2 = new wxStaticText(notebook_pane_colour, wxID_ANY, _("Locked"));
    grid_sizer_colours_server->Add(label_colour_server_mm2, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 6);
    grid_sizer_colours_server->Add(button_colour_server_mm2, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    wxStaticText* label_colour_server_full = new wxStaticText(notebook_pane_colour, wxID_ANY, _("Full"));
    grid_sizer_colours_server->Add(label_colour_server_full, 0, wxALL|wxALIGN_CENTER_VERTICAL, 6);
    grid_sizer_colours_server->Add(button_colour_server_full, 0, wxALL|wxEXPAND|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_colours_server->Add(1, 1, 0, 0, 0);
    wxStaticText* label_colour_server_mm3 = new wxStaticText(notebook_pane_colour, wxID_ANY, _("Restricted (password, banned, private)"));
    grid_sizer_colours_server->Add(label_colour_server_mm3, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 6);
    grid_sizer_colours_server->Add(button_colour_server_mm3, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_colours_server->AddGrowableCol(2);
    sizer_colours_server->Add(grid_sizer_colours_server, 1, wxEXPAND, 0);
    grid_sizer_pane_colours->Add(sizer_colours_server, 1, wxALL|wxEXPAND, 4);
    wxStaticText* label_colour_player_master = new wxStaticText(notebook_pane_colour, wxID_ANY, _("Master"));
    grid_sizer_colours_player->Add(label_colour_player_master, 0, wxALL|wxALIGN_CENTER_VERTICAL, 6);
    grid_sizer_colours_player->Add(button_colour_player_master, 0, wxALL|wxEXPAND|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_colours_player->Add(1, 1, 0, 0, 0);
    wxStaticText* label_colour_player_auth = new wxStaticText(notebook_pane_colour, wxID_ANY, _("Auth master"));
    grid_sizer_colours_player->Add(label_colour_player_auth, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 6);
    grid_sizer_colours_player->Add(button_colour_player_auth, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    wxStaticText* label_colour_player_admin = new wxStaticText(notebook_pane_colour, wxID_ANY, _("Admin"));
    grid_sizer_colours_player->Add(label_colour_player_admin, 0, wxALL|wxALIGN_CENTER_VERTICAL, 6);
    grid_sizer_colours_player->Add(button_colour_player_admin, 0, wxALL|wxEXPAND|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_colours_player->Add(1, 1, 0, 0, 0);
    wxStaticText* label_colour_player_spectator = new wxStaticText(notebook_pane_colour, wxID_ANY, _("Spectator (background)"));
    grid_sizer_colours_player->Add(label_colour_player_spectator, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 6);
    grid_sizer_colours_player->Add(button_colour_player_spectator, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_colours_player->AddGrowableCol(2);
    sizer_colours_player->Add(grid_sizer_colours_player, 1, wxEXPAND, 0);
    grid_sizer_pane_colours->Add(sizer_colours_player, 1, wxALL|wxEXPAND, 4);
    notebook_pane_colour->SetSizer(grid_sizer_pane_colours);
    grid_sizer_pane_colours->AddGrowableCol(0);
    grid_sizer_pane_colours->AddGrowableCol(1);
    wxStaticText* label_update_static = new wxStaticText(notebook_pane_irc, wxID_ANY, _("Update interval (seconds):"));
    grid_sizer_times->Add(label_update_static, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_times->Add(spin_ctrl_update, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_times->Add(checkbox_play_update, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    wxStaticText* label_wait_static = new wxStaticText(notebook_pane_irc, wxID_ANY, _("Wait on full server (seconds):"));
    grid_sizer_times->Add(label_wait_static, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_times->Add(spin_ctrl_wait, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_times->Add(1, 1, 0, 0, 0);
    wxStaticText* label_min_playtime_static = new wxStaticText(notebook_pane_irc, wxID_ANY, _("Minimum playtime for statistics (seconds):"));
    grid_sizer_times->Add(label_min_playtime_static, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_times->Add(spin_ctrl_min_playtime, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_times->Add(1, 1, 0, 0, 0);
    wxStaticText* label_server_cleanup = new wxStaticText(notebook_pane_irc, wxID_ANY, _("Cleanup dead servers (days)"));
    grid_sizer_times->Add(label_server_cleanup, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_times->Add(spin_ctrl_server_cleanup, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_server_cleanup->Add(checkbox_server_cleanup_favourites, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_server_cleanup->Add(checkbox_server_cleanup_stats, 0, wxALL, 4);
    grid_sizer_times->Add(grid_sizer_server_cleanup, 1, wxALIGN_CENTER_VERTICAL, 0);
    wxStaticText* label_tooltip_delay = new wxStaticText(notebook_pane_irc, wxID_ANY, _("Tooltip delay (milliseconds)"));
    grid_sizer_times->Add(label_tooltip_delay, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_times->Add(spin_ctrl_tooltip_delay, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    wxStaticText* label_tooltip_delay_help = new wxStaticText(notebook_pane_irc, wxID_ANY, _("To disable tooltips, set this value to 0."));
    grid_sizer_times->Add(label_tooltip_delay_help, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_times->AddGrowableCol(1);
    sizer_times->Add(grid_sizer_times, 1, wxEXPAND, 0);
    grid_sizer_pane_other->Add(sizer_times, 1, wxALL|wxEXPAND, 4);
    grid_sizer_systray->Add(checkbox_systray, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_systray->Add(checkbox_systray_close, 0, wxALL|wxALIGN_RIGHT, 4);
    sizer_systray->Add(grid_sizer_systray, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL, 0);
    grid_sizer_pane_other->Add(sizer_systray, 1, wxALL|wxEXPAND, 4);
    grid_sizer_tts->Add(checkbox_tts, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    wxStaticText* label_tts_volume = new wxStaticText(notebook_pane_irc, wxID_ANY, _("Volume"));
    grid_sizer_tts->Add(label_tts_volume, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_tts->Add(spin_ctrl_tts_volume, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_tts->Add(button_test_tts, 0, wxALL, 4);
    sizer_tts->Add(grid_sizer_tts, 1, wxEXPAND, 0);
    grid_sizer_pane_other->Add(sizer_tts, 1, wxALL|wxEXPAND, 4);
    wxStaticText* label_ping_good = new wxStaticText(notebook_pane_irc, wxID_ANY, _("Good"));
    grid_sizer_ping->Add(label_ping_good, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_ping->Add(spin_ctrl_ping_good, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    wxStaticText* label_ping_bad = new wxStaticText(notebook_pane_irc, wxID_ANY, _("Bad"));
    grid_sizer_ping->Add(label_ping_bad, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_ping->Add(spin_ctrl_ping_bad, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    sizer_ping->Add(grid_sizer_ping, 1, wxEXPAND, 0);
    grid_sizer_pane_other->Add(sizer_ping, 1, wxALL|wxEXPAND, 4);
    grid_sizer_output->Add(checkbox_game_output, 0, wxALL, 4);
    grid_sizer_output->Add(dirpicker_game_output, 1, wxEXPAND, 0);
    grid_sizer_output->AddGrowableCol(1);
    sizer_output->Add(grid_sizer_output, 1, wxEXPAND, 0);
    grid_sizer_pane_other->Add(sizer_output, 1, wxALL|wxEXPAND, 4);
    notebook_pane_irc->SetSizer(grid_sizer_pane_other);
    grid_sizer_pane_other->AddGrowableCol(0);
    notebook_settings->AddPage(notebook_pane_games, _("Games"));
    notebook_settings->AddPage(notebook_pane_colour, _("Gui"));
    notebook_settings->AddPage(notebook_pane_irc, _("Other"));
    grid_sizer_main->Add(notebook_settings, 1, wxALL|wxEXPAND, 4);
    grid_sizer_button->Add(20, 1, 0, 0, 0);
    grid_sizer_button->Add(button_ok, 0, wxALL, 4);
    grid_sizer_button->Add(button_cancel, 0, wxALL, 4);
    grid_sizer_button->AddGrowableCol(0);
    grid_sizer_main->Add(grid_sizer_button, 1, wxEXPAND, 0);
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
    grid_sizer_pane_other->Layout();
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

            if (i1==m_settings.TooltipDelay)
                break;

            if (i1>m_settings.TooltipDelay)
                i1=max(i1-m_settings.TooltipDelay, CSL_TOOLTIP_DELAY_STEP);
            else
                i1=min(i1-m_settings.TooltipDelay, -CSL_TOOLTIP_DELAY_STEP);

            m_settings.TooltipDelay+=(i1/CSL_TOOLTIP_DELAY_STEP*CSL_TOOLTIP_DELAY_STEP);
            spin_ctrl_tooltip_delay->SetValue(m_settings.TooltipDelay);
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
    /*if (dirpicker_game_output->GetPath().IsEmpty())
        dirpicker_game_output->SetPath(event.GetPath());*/
}

void CslDlgSettings::OnCommandEvent(wxCommandEvent& event)
{
    wxColour *colour = NULL;
    wxBitmapButton *button_color = NULL;

    switch (event.GetId())
    {
        case BUTTON_COLOUR_SERVER_EMPTY:
            colour = &m_settings.ColServerEmpty;
            button_color = button_colour_server_empty;
        case BUTTON_COLOUR_SERVER_OFFLINE:
            if (!colour)
            {
                colour = &m_settings.ColServerOff;
                button_color = button_colour_server_offline;
            }
        case BUTTON_COLOUR_SERVER_FULL:
            if (!colour)
            {
                colour = &m_settings.ColServerFull;
                button_color = button_colour_server_full;
            }
        case BUTTON_COLOUR_SERVER_MM1:
            if (!colour)
            {
                colour = &m_settings.ColServerMM1;
                button_color = button_colour_server_mm1;
            }
        case BUTTON_COLOUR_SERVER_MM2:
            if (!colour)
            {
                colour = &m_settings.ColServerMM2;
                button_color = button_colour_server_mm2;
            }
        case BUTTON_COLOUR_SERVER_MM3:
            if (!colour)
            {
                colour = &m_settings.ColServerMM3;
                button_color = button_colour_server_mm3;
            }
        case BUTTON_COLOUR_PLAYER_MASTER:
            if (!colour)
            {
                colour = &m_settings.ColPlayerMaster;
                button_color = button_colour_player_master;
            }
        case BUTTON_COLOUR_PLAYER_AUTH:
            if (!colour)
            {
                colour = &m_settings.ColPlayerAuth;
                button_color = button_colour_player_auth;
            }
        case BUTTON_COLOUR_PLAYER_ADMIN:
            if (!colour)
            {
                colour = &m_settings.ColPlayerAdmin;
                button_color = button_colour_player_admin;
            }
        case BUTTON_COLOUR_PLAYER_SPECTATOR:
        {
            if (!colour)
            {
                colour = &m_settings.ColPlayerSpectator;
                button_color = button_colour_player_spectator;
            }

            wxColor color_new = wxGetColourFromUser(this, *colour);

            if (!color_new.Ok())
                break;

            *colour = color_new;
            SetButtonColour(button_color, button_ok, color_new);
            break;
        }

#ifndef __WXMAC__
        case CHECK_SYSTRAY:
            checkbox_systray_close->Enable(event.IsChecked());
            break;
#endif
        case CHECK_TTS:
            spin_ctrl_tts_volume->Enable(event.IsChecked());
            button_test_tts->Enable(event.IsChecked());
            break;

        case BUTTON_TEST_TTS:
        {
            CslTTS::Say(wxT("CSL TTS test"), spin_ctrl_tts_volume->GetValue());
            break;
        }

        case CHECK_GAME_OUTPUT:
            dirpicker_game_output->Enable(event.IsChecked());
            break;

        case wxID_CANCEL:
            break;
        case wxID_OK:
        {
            wxString msg;

            m_settings.UpdateInterval=spin_ctrl_update->GetValue()*1000;
            m_settings.WaitServerFull=spin_ctrl_wait->GetValue();
            m_settings.DontUpdatePlaying=checkbox_play_update->IsChecked();
            m_settings.MinPlaytime=spin_ctrl_min_playtime->GetValue();
            m_settings.CleanupServers=spin_ctrl_server_cleanup->GetValue()*86400;
            m_settings.CleanupServersKeepFav=checkbox_server_cleanup_favourites->GetValue();
            m_settings.CleanupServersKeepStats=checkbox_server_cleanup_stats->GetValue();
            m_settings.TooltipDelay=spin_ctrl_tooltip_delay->GetValue();
            m_settings.PingGood=spin_ctrl_ping_good->GetValue();
            m_settings.PingBad=spin_ctrl_ping_bad->GetValue();
#ifndef __WXMAC__
            m_settings.Systray=checkbox_systray->IsChecked() ? CSL_USE_SYSTRAY : 0;
            CSL_FLAG_SET(m_settings.Systray, checkbox_systray_close->GetValue() ? CSL_SYSTRAY_CLOSE : 0);
#endif
            m_settings.TTS=checkbox_tts->IsChecked();
            m_settings.TTSVolume=spin_ctrl_tts_volume->GetValue();
            m_settings.GameOutputPath=dirpicker_game_output->GetPath();
            m_settings.AutoSaveOutput=checkbox_game_output->IsChecked();

            if (!ValidateSettings())
                return;
            for (wxUint32 i=0;i<notebook_games->GetPageCount();i++)
            {
                if (!((CslPanelGameSettings*)notebook_games->GetPage(i))->SaveSettings(msg))
                {
                    wxMessageBox(msg,_("Error"),wxICON_ERROR,this);
                    return;
                }
            }

            CslTTS::Set(m_settings.TTS, m_settings.TTSVolume);

            CslGetSettings() = m_settings;

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
    if (m_settings.PingGood>m_settings.PingBad)
    {
        wxMessageBox(    _("Threshold for good ping can't be higher than\n"
                     wxT_2("threshold for bad ping.")), _("Error"), wxICON_ERROR, this);
        return false;
    }

    if (checkbox_game_output->IsChecked() && m_settings.GameOutputPath.IsEmpty())
    {
        wxMessageBox(_("Select a folder for game output logs or disable autosave."),
                     _("Error"),wxICON_ERROR,this);
        return false;
    }

    return true;
}
