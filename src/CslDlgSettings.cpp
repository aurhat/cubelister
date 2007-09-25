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

#include <wx/colordlg.h>
#include "CslDlgSettings.h"
#include <wx/imaglist.h>
#ifndef _MSC_VER
#include "img/sb_24.xpm"
#include "img/ac_24.xpm"
#include "img/bf_24.xpm"
#include "img/cb_24.xpm"
#endif


BEGIN_EVENT_TABLE(CslDlgSettings,wxDialog)
    EVT_FILEPICKER_CHANGED(wxID_ANY,CslDlgSettings::OnPicker)
    EVT_DIRPICKER_CHANGED(wxID_ANY,CslDlgSettings::OnPicker)
    EVT_BUTTON(wxID_ANY,CslDlgSettings::OnCommandEvent)
    EVT_RADIOBUTTON(wxID_ANY,CslDlgSettings::OnCommandEvent)
    EVT_SPINCTRL(wxID_ANY,CslDlgSettings::OnSpinCtrl)
END_EVENT_TABLE()

enum
{
    FILE_PICKER_SB = wxID_HIGHEST + 1,
    DIR_PICKER_SB,
    FILE_PICKER_AC,
    DIR_PICKER_AC,
    FILE_PICKER_BF,
    DIR_PICKER_BF,
    FILE_PICKER_CB,
    DIR_PICKER_CB,

    BUTTON_COLOUR_EMPTY,
    BUTTON_COLOUR_OFF,
    BUTTON_COLOUR_FULL,
    BUTTON_COLOUR_MM1,
    BUTTON_COLOUR_MM2,
    BUTTON_COLOUR_MM3,

    SPIN_PING_GOOD,
    SPIN_PING_BAD
};

enum
{
    IMG_LIST_GAMES_SB = 0,
    IMG_LIST_GAMES_AC,
    IMG_LIST_GAMES_BF,
    IMG_LIST_GAMES_CB
};


CslSettings *g_cslSettings;

CslDlgSettings::CslDlgSettings(wxWindow* parent,int id,const wxString& title,
                               const wxPoint& pos,const wxSize& size,long style):
        wxDialog(parent, id, title, pos, size, style)
{
    m_settings=*g_cslSettings;

    // begin wxGlade: CslDlgSettings::CslDlgSettings
    notebook_settings = new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
    notebook_pane_other = new wxPanel(notebook_settings, wxID_ANY);
    notebook_pane_colour = new wxPanel(notebook_settings, wxID_ANY);
    notebook_pane_games = new wxPanel(notebook_settings, wxID_ANY);
    notebook_games = new wxListbook(notebook_pane_games, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
    notebook_pane_cube = new wxPanel(notebook_games, wxID_ANY);
    notebook_pane_frontier = new wxPanel(notebook_games, wxID_ANY);
    notebook_pane_assault = new wxPanel(notebook_games, wxID_ANY);
    sizer_colours_staticbox = new wxStaticBox(notebook_pane_colour, -1, _("Server lists"));
    sizer_times_staticbox = new wxStaticBox(notebook_pane_other, -1, _("Times && Intervals"));
    sizer_ping_threshold_staticbox = new wxStaticBox(notebook_pane_other, -1, _("Ping thresholds"));
    notebook_pane_sauer = new wxPanel(notebook_games, wxID_ANY);
    text_ctrl_sauer_options = new wxTextCtrl(notebook_pane_sauer, wxID_ANY, wxEmptyString);
    text_ctrl_assault_options = new wxTextCtrl(notebook_pane_assault, wxID_ANY, wxEmptyString);
    text_ctrl_frontier_options = new wxTextCtrl(notebook_pane_frontier, wxID_ANY, wxEmptyString);
    text_ctrl_cube_options = new wxTextCtrl(notebook_pane_cube, wxID_ANY, wxEmptyString);
    button_colour_empty = new wxBitmapButton(notebook_pane_colour, BUTTON_COLOUR_EMPTY, wxNullBitmap);
    button_colour_mm1 = new wxBitmapButton(notebook_pane_colour, BUTTON_COLOUR_MM1, wxNullBitmap);
    button_colour_off = new wxBitmapButton(notebook_pane_colour, BUTTON_COLOUR_OFF, wxNullBitmap);
    button_colour_mm2 = new wxBitmapButton(notebook_pane_colour, BUTTON_COLOUR_MM2, wxNullBitmap);
    button_colour_full = new wxBitmapButton(notebook_pane_colour, BUTTON_COLOUR_FULL, wxNullBitmap);
    button_colour_mm3 = new wxBitmapButton(notebook_pane_colour, BUTTON_COLOUR_MM3, wxNullBitmap);
    spin_ctrl_update = new wxSpinCtrl(notebook_pane_other, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100);
    checkbox_play_update = new wxCheckBox(notebook_pane_other, wxID_ANY, _("Don't update when playing"));
    spin_ctrl_wait = new wxSpinCtrl(notebook_pane_other, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100);
    spin_ctrl_min_playtime = new wxSpinCtrl(notebook_pane_other, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100);
    spin_ctrl_ping_good = new wxSpinCtrl(notebook_pane_other, SPIN_PING_GOOD, wxT(""), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100);
    spin_ctrl_ping_bad = new wxSpinCtrl(notebook_pane_other, SPIN_PING_BAD, wxT(""), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100);
    button_ok = new wxButton(this, wxID_OK, _("&Ok"));
    button_cancel = new wxButton(this, wxID_CANCEL, _("&Cancel"));

    set_properties();
    do_layout();
    // end wxGlade

    wxImageList *imgList=new wxImageList(24,24,true);
#ifndef _MSC_VER
    imgList->Add(wxBitmap(sb_24_xpm));
    imgList->Add(wxBitmap(ac_24_xpm));
    imgList->Add(wxBitmap(bf_24_xpm));
    imgList->Add(wxBitmap(cb_24_xpm));
#else
    imgList->Add(wxIcon(wxT("ICON_TREE_SB_24"),wxBITMAP_TYPE_ICO_RESOURCE));
    imgList->Add(wxIcon(wxT("ICON_TREE_AC_24"),wxBITMAP_TYPE_ICO_RESOURCE));
    imgList->Add(wxIcon(wxT("ICON_TREE_BF_24"),wxBITMAP_TYPE_ICO_RESOURCE));
    imgList->Add(wxIcon(wxT("ICON_TREE_CB_24"),wxBITMAP_TYPE_ICO_RESOURCE));
#endif

    notebook_games->AssignImageList(imgList);
    notebook_games->SetPageImage(0,IMG_LIST_GAMES_SB);
    notebook_games->SetPageImage(1,IMG_LIST_GAMES_AC);
    notebook_games->SetPageImage(2,IMG_LIST_GAMES_BF);
    notebook_games->SetPageImage(3,IMG_LIST_GAMES_CB);
}

CslDlgSettings::~CslDlgSettings()
{
#ifndef __WXMAC__
    delete fpickSauer;
    delete fpickAssault;
    delete fpickFrontier;
    delete fpickCube;
#endif
    delete dpickSauer;
    delete dpickAssault;
    delete dpickFrontier;
    delete dpickCube;
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
    spin_ctrl_update->SetMinSize(wxDLG_UNIT(spin_ctrl_update, wxSize(48, -1)));
    spin_ctrl_wait->SetMinSize(wxDLG_UNIT(spin_ctrl_wait, wxSize(48, -1)));
    spin_ctrl_min_playtime->SetMinSize(wxDLG_UNIT(spin_ctrl_min_playtime, wxSize(48, -1)));
    button_ok->SetDefault();
    // end wxGlade

    text_ctrl_sauer_options->SetValue(m_settings.m_clientOptsSB);
    text_ctrl_assault_options->SetValue(m_settings.m_clientOptsAC);
    text_ctrl_frontier_options->SetValue(m_settings.m_clientOptsBF);
    text_ctrl_cube_options->SetValue(m_settings.m_clientOptsCB);

    checkbox_play_update->SetValue(m_settings.m_dontUpdatePlaying);
    spin_ctrl_update->SetRange(CSL_UPDATE_INTERVAL_MIN/1000,CSL_UPDATE_INTERVAL_MAX/1000);
    spin_ctrl_update->SetValue(m_settings.m_updateInterval/1000);
    spin_ctrl_wait->SetRange(CSL_WAIT_SERVER_FULL_MIN,CSL_WAIT_SERVER_FULL_MAX);
    spin_ctrl_wait->SetValue(m_settings.m_waitServerFull);
    spin_ctrl_min_playtime->SetRange(CSL_MIN_PLAYTIME_MIN,CSL_MIN_PLAYTIME_MAX);
    spin_ctrl_min_playtime->SetValue(m_settings.m_minPlaytime);
    spin_ctrl_ping_good->SetRange(0,9999);
    spin_ctrl_ping_good->SetValue(m_settings.m_ping_good);
    spin_ctrl_ping_bad->SetRange(0,9999);
    spin_ctrl_ping_bad->SetValue(m_settings.m_ping_bad);

    SetButtonColour(button_colour_empty,button_ok,m_settings.m_colServerEmpty);
    SetButtonColour(button_colour_off,button_ok,m_settings.m_colServerOff);
    SetButtonColour(button_colour_full,button_ok,m_settings.m_colServerFull);
    SetButtonColour(button_colour_mm1,button_ok,m_settings.m_colServerMM1);
    SetButtonColour(button_colour_mm2,button_ok,m_settings.m_colServerMM2);
    SetButtonColour(button_colour_mm3,button_ok,m_settings.m_colServerMM3);
}


void CslDlgSettings::do_layout()
{
#ifdef __WXMSW__
    wxString ext=_("Executables (*.exe; *.bat; *.cmd)|*.exe;*.bat;*.cmd");
#else
    wxString ext=wxT("*");
#endif

    wxSize sizePicker(-1,-1);

#ifndef __WXMAC__
    fpickSauer=new wxFilePickerCtrl(notebook_pane_sauer,FILE_PICKER_SB,wxEmptyString,
                                    _("Select Sauerbraten executable"),ext,
                                    wxDefaultPosition,sizePicker,
                                    wxFLP_DEFAULT_STYLE|wxFLP_USE_TEXTCTRL|wxFLP_FILE_MUST_EXIST);
    fpickAssault=new wxFilePickerCtrl(notebook_pane_assault,FILE_PICKER_AC,wxEmptyString,
                                      _("Select AssaultCube executable"),ext,
                                      wxDefaultPosition,sizePicker,
                                      wxFLP_DEFAULT_STYLE|wxFLP_USE_TEXTCTRL|wxFLP_FILE_MUST_EXIST);
    fpickFrontier=new wxFilePickerCtrl(notebook_pane_frontier,FILE_PICKER_BF,wxEmptyString,
                                       _("Select Blood Frontier executable"),ext,
                                       wxDefaultPosition,sizePicker,
                                       wxFLP_DEFAULT_STYLE|wxFLP_USE_TEXTCTRL|wxFLP_FILE_MUST_EXIST);
    fpickCube=new wxFilePickerCtrl(notebook_pane_cube,FILE_PICKER_CB,wxEmptyString,
                                   _("Select Cube executable"),ext,
                                   wxDefaultPosition,sizePicker,
                                   wxFLP_DEFAULT_STYLE|wxFLP_USE_TEXTCTRL|wxFLP_FILE_MUST_EXIST);
#endif
    dpickSauer=new wxDirPickerCtrl(notebook_pane_sauer,DIR_PICKER_SB,m_settings.m_configPathSB,
                                   _("Select Sauerbraten game path"),
                                   wxDefaultPosition,sizePicker,
                                   wxDIRP_DEFAULT_STYLE|wxDIRP_USE_TEXTCTRL|wxDIRP_DIR_MUST_EXIST);
    dpickAssault=new wxDirPickerCtrl(notebook_pane_assault,DIR_PICKER_AC,m_settings.m_configPathAC,
                                     _("Select AssaultCube game path"),
                                     wxDefaultPosition,sizePicker,
                                     wxDIRP_DEFAULT_STYLE|wxDIRP_USE_TEXTCTRL|wxDIRP_DIR_MUST_EXIST);
    dpickFrontier=new wxDirPickerCtrl(notebook_pane_frontier,DIR_PICKER_BF,m_settings.m_configPathBF,
                                      _("Select Blood Frontier game path"),
                                      wxDefaultPosition,sizePicker,
                                      wxDIRP_DEFAULT_STYLE|wxDIRP_USE_TEXTCTRL|wxDIRP_DIR_MUST_EXIST);
    dpickCube=new wxDirPickerCtrl(notebook_pane_cube,DIR_PICKER_CB,m_settings.m_configPathCB,
                                  _("Select Cube game path"),
                                  wxDefaultPosition,sizePicker,
                                  wxDIRP_DEFAULT_STYLE|wxDIRP_USE_TEXTCTRL|wxDIRP_DIR_MUST_EXIST);

#ifndef __WXMAC__
    if (!m_settings.m_clientBinSB.IsEmpty())
        fpickSauer->SetPath(m_settings.m_clientBinSB);
    if (!m_settings.m_clientBinAC.IsEmpty())
        fpickAssault->SetPath(m_settings.m_clientBinAC);
    if (!m_settings.m_clientBinBF.IsEmpty())
        fpickFrontier->SetPath(m_settings.m_clientBinBF);
    if (!m_settings.m_clientBinCB.IsEmpty())
        fpickCube->SetPath(m_settings.m_clientBinCB);
#endif
    if (!m_settings.m_configPathSB.IsEmpty())
        dpickSauer->SetPath(m_settings.m_configPathSB);
    if (!m_settings.m_configPathAC.IsEmpty())
        dpickAssault->SetPath(m_settings.m_configPathAC);
    if (!m_settings.m_configPathBF.IsEmpty())
        dpickFrontier->SetPath(m_settings.m_configPathBF);
    if (!m_settings.m_configPathCB.IsEmpty())
        dpickCube->SetPath(m_settings.m_configPathCB);

    // begin wxGlade: CslDlgSettings::do_layout
    wxFlexGridSizer* grid_sizer_main = new wxFlexGridSizer(2, 1, 0, 0);
    wxFlexGridSizer* grid_sizer_button = new wxFlexGridSizer(1, 3, 0, 0);
    wxFlexGridSizer* grid_sizer_pane_other = new wxFlexGridSizer(2, 1, 0, 0);
    wxStaticBoxSizer* sizer_ping_threshold = new wxStaticBoxSizer(sizer_ping_threshold_staticbox, wxHORIZONTAL);
    wxFlexGridSizer* grid_sizer_threshold = new wxFlexGridSizer(1, 5, 0, 0);
    wxStaticBoxSizer* sizer_times = new wxStaticBoxSizer(sizer_times_staticbox, wxHORIZONTAL);
    wxFlexGridSizer* grid_sizer_times = new wxFlexGridSizer(3, 3, 0, 0);
    wxFlexGridSizer* grid_sizer_pane_colours = new wxFlexGridSizer(1, 1, 0, 0);
    wxStaticBoxSizer* sizer_colours = new wxStaticBoxSizer(sizer_colours_staticbox, wxHORIZONTAL);
    wxFlexGridSizer* grid_sizer_colours = new wxFlexGridSizer(3, 5, 0, 0);
    wxBoxSizer* sizer_games = new wxBoxSizer(wxHORIZONTAL);
    wxFlexGridSizer* grid_sizer_cube_path = new wxFlexGridSizer(3, 2, 0, 0);
    wxFlexGridSizer* grid_sizer_frontier_path = new wxFlexGridSizer(3, 2, 0, 0);
    wxFlexGridSizer* grid_sizer_assault_path = new wxFlexGridSizer(3, 2, 0, 0);
    wxFlexGridSizer* grid_sizer_sauer_path = new wxFlexGridSizer(3, 2, 0, 0);
    wxStaticText* label_sauer_exe = new wxStaticText(notebook_pane_sauer, wxID_ANY, _("Game executable:"));
    grid_sizer_sauer_path->Add(label_sauer_exe, 0, wxLEFT|wxALIGN_CENTER_VERTICAL, 8);
    wxStaticText* label_sauer_path = new wxStaticText(notebook_pane_sauer, wxID_ANY, _("Game directory:"));
    grid_sizer_sauer_path->Add(label_sauer_path, 0, wxLEFT|wxALIGN_CENTER_VERTICAL, 8);
    wxStaticText* label_sauer_options = new wxStaticText(notebook_pane_sauer, wxID_ANY, _("Game paramters:"));
    grid_sizer_sauer_path->Add(label_sauer_options, 0, wxLEFT|wxALIGN_CENTER_VERTICAL, 8);
    grid_sizer_sauer_path->Add(text_ctrl_sauer_options, 0, wxALL|wxEXPAND, 4);
    notebook_pane_sauer->SetSizer(grid_sizer_sauer_path);
    grid_sizer_sauer_path->AddGrowableCol(1);
    wxStaticText* label_assault_exe = new wxStaticText(notebook_pane_assault, wxID_ANY, _("Game executable:"));
    grid_sizer_assault_path->Add(label_assault_exe, 0, wxLEFT|wxALIGN_CENTER_VERTICAL, 8);
    wxStaticText* label_assault_path = new wxStaticText(notebook_pane_assault, wxID_ANY, _("Game directory:"));
    grid_sizer_assault_path->Add(label_assault_path, 0, wxLEFT|wxALIGN_CENTER_VERTICAL, 8);
    wxStaticText* label_assault_options = new wxStaticText(notebook_pane_assault, wxID_ANY, _("Game paramters:"));
    grid_sizer_assault_path->Add(label_assault_options, 0, wxLEFT|wxALIGN_CENTER_VERTICAL, 8);
    grid_sizer_assault_path->Add(text_ctrl_assault_options, 0, wxALL|wxEXPAND, 4);
    notebook_pane_assault->SetSizer(grid_sizer_assault_path);
    grid_sizer_assault_path->AddGrowableCol(1);
    wxStaticText* label_frontier_exe = new wxStaticText(notebook_pane_frontier, wxID_ANY, _("Game executable:"));
    grid_sizer_frontier_path->Add(label_frontier_exe, 0, wxLEFT|wxALIGN_CENTER_VERTICAL, 8);
    wxStaticText* label_frontier_path = new wxStaticText(notebook_pane_frontier, wxID_ANY, _("Game directory:"));
    grid_sizer_frontier_path->Add(label_frontier_path, 0, wxLEFT|wxALIGN_CENTER_VERTICAL, 8);
    wxStaticText* label_frontier_options = new wxStaticText(notebook_pane_frontier, wxID_ANY, _("Game paramters:"));
    grid_sizer_frontier_path->Add(label_frontier_options, 0, wxLEFT|wxALIGN_CENTER_VERTICAL, 8);
    grid_sizer_frontier_path->Add(text_ctrl_frontier_options, 0, wxALL|wxEXPAND, 4);
    notebook_pane_frontier->SetSizer(grid_sizer_frontier_path);
    grid_sizer_frontier_path->AddGrowableCol(1);
    wxStaticText* label_cube_exe = new wxStaticText(notebook_pane_cube, wxID_ANY, _("Game executable:"));
    grid_sizer_cube_path->Add(label_cube_exe, 0, wxLEFT|wxALIGN_CENTER_VERTICAL, 8);
    wxStaticText* label_cube_path = new wxStaticText(notebook_pane_cube, wxID_ANY, _("Game directory:"));
    grid_sizer_cube_path->Add(label_cube_path, 0, wxLEFT|wxALIGN_CENTER_VERTICAL, 8);
    wxStaticText* label_cube_options = new wxStaticText(notebook_pane_cube, wxID_ANY, _("Game paramters:"));
    grid_sizer_cube_path->Add(label_cube_options, 0, wxLEFT|wxALIGN_CENTER_VERTICAL, 8);
    grid_sizer_cube_path->Add(text_ctrl_cube_options, 0, wxALL|wxEXPAND, 4);
    notebook_pane_cube->SetSizer(grid_sizer_cube_path);
    grid_sizer_cube_path->AddGrowableCol(1);
    notebook_games->AddPage(notebook_pane_sauer, _("Sauerbraten"));
    notebook_games->AddPage(notebook_pane_assault, _("AssaultCube"));
    notebook_games->AddPage(notebook_pane_frontier, _("Blood Frontier"));
    notebook_games->AddPage(notebook_pane_cube, _("Cube"));
    sizer_games->Add(notebook_games, 1, wxALL|wxEXPAND, 4);
    notebook_pane_games->SetSizer(sizer_games);
    wxStaticText* label_6 = new wxStaticText(notebook_pane_colour, wxID_ANY, _("Server empty"));
    grid_sizer_colours->Add(label_6, 0, wxALL|wxALIGN_CENTER_VERTICAL, 6);
    grid_sizer_colours->Add(button_colour_empty, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_colours->Add(1, 1, 0, wxADJUST_MINSIZE, 0);
    wxStaticText* label_3 = new wxStaticText(notebook_pane_colour, wxID_ANY, _("Mastermode 1"));
    grid_sizer_colours->Add(label_3, 0, wxALL|wxALIGN_CENTER_VERTICAL, 6);
    grid_sizer_colours->Add(button_colour_mm1, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    wxStaticText* label_1 = new wxStaticText(notebook_pane_colour, wxID_ANY, _("Server offline"));
    grid_sizer_colours->Add(label_1, 0, wxALL|wxALIGN_CENTER_VERTICAL, 6);
    grid_sizer_colours->Add(button_colour_off, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_colours->Add(1, 1, 0, wxADJUST_MINSIZE, 0);
    wxStaticText* label_4 = new wxStaticText(notebook_pane_colour, wxID_ANY, _("Mastermode 2"));
    grid_sizer_colours->Add(label_4, 0, wxALL|wxALIGN_CENTER_VERTICAL, 6);
    grid_sizer_colours->Add(button_colour_mm2, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    wxStaticText* label_2 = new wxStaticText(notebook_pane_colour, wxID_ANY, _("Server full"));
    grid_sizer_colours->Add(label_2, 0, wxALL|wxALIGN_CENTER_VERTICAL, 6);
    grid_sizer_colours->Add(button_colour_full, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_colours->Add(1, 1, 0, wxADJUST_MINSIZE, 0);
    wxStaticText* label_5 = new wxStaticText(notebook_pane_colour, wxID_ANY, _("Mastermode 3"));
    grid_sizer_colours->Add(label_5, 0, wxALL|wxALIGN_CENTER_VERTICAL, 6);
    grid_sizer_colours->Add(button_colour_mm3, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_colours->AddGrowableCol(2);
    sizer_colours->Add(grid_sizer_colours, 1, wxEXPAND, 0);
    grid_sizer_pane_colours->Add(sizer_colours, 1, wxALL|wxEXPAND, 4);
    notebook_pane_colour->SetSizer(grid_sizer_pane_colours);
    grid_sizer_pane_colours->AddGrowableCol(0);
    grid_sizer_pane_colours->AddGrowableCol(1);
    wxStaticText* label_update_static = new wxStaticText(notebook_pane_other, wxID_ANY, _("Update interval (sec):"));
    grid_sizer_times->Add(label_update_static, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_times->Add(spin_ctrl_update, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_times->Add(checkbox_play_update, 0, wxLEFT|wxALIGN_CENTER_VERTICAL, 8);
    wxStaticText* label_wait_static = new wxStaticText(notebook_pane_other, wxID_ANY, _("Wait on full server (sec):"));
    grid_sizer_times->Add(label_wait_static, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_times->Add(spin_ctrl_wait, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_times->Add(1, 1, 0, 0, 0);
    wxStaticText* label_min_playtime_static = new wxStaticText(notebook_pane_other, wxID_ANY, _("Minimum playtime for statistics (sec):"));
    grid_sizer_times->Add(label_min_playtime_static, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_times->Add(spin_ctrl_min_playtime, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_times->Add(1, 1, 0, 0, 0);
    grid_sizer_times->AddGrowableCol(1);
    sizer_times->Add(grid_sizer_times, 1, wxEXPAND, 0);
    grid_sizer_pane_other->Add(sizer_times, 1, wxALL|wxEXPAND, 4);
    wxStaticText* label_ping_good = new wxStaticText(notebook_pane_other, wxID_ANY, _("Good"));
    grid_sizer_threshold->Add(label_ping_good, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_threshold->Add(spin_ctrl_ping_good, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_threshold->Add(1, 1, 0, wxADJUST_MINSIZE, 0);
    wxStaticText* label_ping_bad = new wxStaticText(notebook_pane_other, wxID_ANY, _("Bad"));
    grid_sizer_threshold->Add(label_ping_bad, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_threshold->Add(spin_ctrl_ping_bad, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_threshold->AddGrowableCol(2);
    sizer_ping_threshold->Add(grid_sizer_threshold, 1, wxEXPAND, 0);
    grid_sizer_pane_other->Add(sizer_ping_threshold, 1, wxALL|wxEXPAND, 4);
    notebook_pane_other->SetSizer(grid_sizer_pane_other);
    grid_sizer_pane_other->AddGrowableCol(0);
    notebook_settings->AddPage(notebook_pane_games, _("Games"));
    notebook_settings->AddPage(notebook_pane_colour, _("Colours"));
    notebook_settings->AddPage(notebook_pane_other, _("Other"));
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

    CentreOnParent();

    wxInt32 dppos=3;
#ifdef __WXMAC__
    dppos=1;
    label_sauer_exe->Hide();
    label_assault_exe->Hide();
    label_frontier_exe->Hide();
    label_cube_exe->Hide();
    grid_sizer_sauer_path->Detach(label_sauer_exe);
    grid_sizer_assault_path->Detach(label_assault_exe);
    grid_sizer_frontier_path->Detach(label_frontier_exe);
    grid_sizer_cube_path->Detach(label_cube_exe);
#else
    grid_sizer_sauer_path->Insert(1,fpickSauer,0,wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL,4);
    grid_sizer_assault_path->Insert(1,fpickAssault,0,wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL,4);
    grid_sizer_frontier_path->Insert(1,fpickFrontier,0,wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL,4);
    grid_sizer_cube_path->Insert(1,fpickCube,0,wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL,4);
#endif
    grid_sizer_sauer_path->Insert(dppos,dpickSauer,0,wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL,4);
    grid_sizer_assault_path->Insert(dppos,dpickAssault,0,wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL,4);
    grid_sizer_frontier_path->Insert(dppos,dpickFrontier,0,wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL,4);
    grid_sizer_cube_path->Insert(dppos,dpickCube,0,wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL,4);

    Layout();
    grid_sizer_main->Fit(this);

    // what a crap
    wxSize a=notebook_settings->GetBestSize();
    wxSize d(a.x,a.y+48);
    notebook_settings->SetMinSize(d);

    grid_sizer_main->SetSizeHints(this);
}

void CslDlgSettings::OnPicker(wxFileDirPickerEvent& event)
{
    wxString path=event.GetPath();
    wxString gpath;

    switch (event.GetId())
    {
#ifndef __WXMAC__
        case FILE_PICKER_SB:
            m_settings.m_clientBinSB=path;
            if (dpickSauer->GetPath().IsEmpty())
            {
                gpath=path.BeforeLast(PATHDIVA);
                dpickSauer->SetPath(gpath);
                m_settings.m_configPathSB=gpath;
            }

            break;
        case FILE_PICKER_AC:
            m_settings.m_clientBinAC=path;
            if (dpickAssault->GetPath().IsEmpty())
            {
                gpath=path.BeforeLast(PATHDIVA);
                dpickAssault->SetPath(gpath);
                m_settings.m_configPathAC=gpath;
            }
            break;
        case FILE_PICKER_BF:
            m_settings.m_clientBinBF=path;
            if (dpickFrontier->GetPath().IsEmpty())
            {
                gpath=path.BeforeLast(PATHDIVA);
                dpickFrontier->SetPath(gpath);
                m_settings.m_configPathBF=gpath;
            }
            break;
        case FILE_PICKER_CB:
            m_settings.m_clientBinCB=path;
            if (dpickCube->GetPath().IsEmpty())
            {
                gpath=path.BeforeLast(PATHDIVA);
                dpickCube->SetPath(gpath);
                m_settings.m_configPathCB=gpath;
            }
            break;
#endif
        case DIR_PICKER_SB:
            m_settings.m_configPathSB=path;
#ifdef __WXMAC__
            m_settings.m_configPathSB+=wxT("/sauerbraten");
            m_settings.m_clientBinSB=path+wxT("/sauerbraten/sauerbraten.app/Contents/MacOS/sauerbraten");
#endif
            break;
        case DIR_PICKER_AC:
            m_settings.m_configPathAC=path;
#ifdef __WXMAC__
            m_settings.m_configPathAC+=wxT("/assaultcube");
            m_settings.m_clientBinAC=path+wxT("/assaultcube/actioncube.app/Contents/MacOS/actioncube");
#endif
            break;
        case DIR_PICKER_BF:
            m_settings.m_configPathBF=path;
#ifdef __WXMAC__
            m_settings.m_configPathBF+=wxT("");
            m_settings.m_clientBinBF=path+wxT("/sauerbraten.app/Contents/MacOS/sauerbraten");
#endif
            break;
        case DIR_PICKER_CB:
            m_settings.m_configPathCB=path;
#ifdef __WXMAC__
            m_settings.m_configPathCB+=wxT("/cube");
            m_settings.m_clientBinCB=path+wxT("/cube/cube.app/Contents/MacOS/Cube");
#endif
            break;
    }
}

void CslDlgSettings::OnSpinCtrl(wxSpinEvent& event)
{
    switch (event.GetId())
    {
        case SPIN_PING_GOOD:
        {
            wxUint32 val=event.GetPosition();
            wxUint32 bad=spin_ctrl_ping_bad->GetValue();
            if (val>bad)
                spin_ctrl_ping_bad->SetValue(val);
            break;
        }
        case SPIN_PING_BAD:
        {
            wxUint32 val=event.GetPosition();
            wxUint32 good=spin_ctrl_ping_good->GetValue();
            if (val<good)
                spin_ctrl_ping_good->SetValue(val);
            break;
        }
    }

    event.Skip();
}

void CslDlgSettings::OnCommandEvent(wxCommandEvent& event)
{
    wxColour colnew,*colour=NULL;
    wxBitmapButton *colButton=NULL;

    switch (event.GetId())
    {
        case BUTTON_COLOUR_EMPTY:
            colour=&m_settings.m_colServerEmpty;
            colButton=button_colour_empty;
        case BUTTON_COLOUR_OFF:
            if (!colour)
            {
                colour=&m_settings.m_colServerOff;
                colButton=button_colour_off;
            }
        case BUTTON_COLOUR_FULL:
            if (!colour)
            {
                colour=&m_settings.m_colServerFull;
                colButton=button_colour_full;
            }
        case BUTTON_COLOUR_MM1:
            if (!colour)
            {
                colour=&m_settings.m_colServerMM1;
                colButton=button_colour_mm1;
            }
        case BUTTON_COLOUR_MM2:
            if (!colour)
            {
                colour=&m_settings.m_colServerMM2;
                colButton=button_colour_mm2;
            }
        case BUTTON_COLOUR_MM3:
            if (!colour)
            {
                colour=&m_settings.m_colServerMM3;
                colButton=button_colour_mm3;
            }
            colnew=wxGetColourFromUser(this,*colour);
            if (!colnew.Ok())
                return;
            *colour=colnew;
            SetButtonColour(colButton,button_ok,*colour);
            break;

        case wxID_CANCEL:
            break;
        case wxID_OK:
            m_settings.m_clientOptsSB=text_ctrl_sauer_options->GetValue();
            m_settings.m_clientOptsAC=text_ctrl_assault_options->GetValue();
            m_settings.m_clientOptsBF=text_ctrl_frontier_options->GetValue();
            m_settings.m_clientOptsCB=text_ctrl_cube_options->GetValue();
            m_settings.m_updateInterval=spin_ctrl_update->GetValue()*1000;
            m_settings.m_waitServerFull=spin_ctrl_wait->GetValue();
            m_settings.m_dontUpdatePlaying=checkbox_play_update->IsChecked();
            m_settings.m_minPlaytime=spin_ctrl_min_playtime->GetValue();
            m_settings.m_ping_good=spin_ctrl_ping_good->GetValue();
            m_settings.m_ping_bad=spin_ctrl_ping_bad->GetValue();

            if (!Validate())
                return;
            *g_cslSettings=m_settings;
            EndModal(wxID_OK);
            break;
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


#define CSL_ERROR_SETTINGS_CONNECT_FMT_STR \
    _("Game installation path missing!\n" \
      "For game %s the folder containing\n"\
      "%s is necessary to connect to a server.")

bool CslDlgSettings::Validate()
{
    wxString msg;

    msg=wxString::Format(CSL_ERROR_SETTINGS_CONNECT_FMT_STR,
                         GetGameStr(CSL_GAME_SB),
                         CSL_DEFAULT_INJECT_DIR_SB);
    if (!m_settings.m_clientBinSB.IsEmpty())
    {
        if (m_settings.m_configPathSB.IsEmpty())
        {
            wxMessageBox(msg,_("Error"),wxICON_ERROR,this);
            return false;
        }

        if (!m_settings.m_configPathSB.EndsWith(wxString(PATHDIV)))
            m_settings.m_configPathSB+=PATHDIV;

        if (!::wxDirExists(m_settings.m_configPathSB+CSL_DEFAULT_INJECT_DIR_SB))
        {
            wxMessageBox(msg,_("Error"),wxICON_ERROR,this);
            return false;
        }
    }

    msg=wxString::Format(CSL_ERROR_SETTINGS_CONNECT_FMT_STR,
                         GetGameStr(CSL_GAME_AC),
                         CSL_DEFAULT_INJECT_DIR_AC);
    if (!m_settings.m_clientBinAC.IsEmpty())
    {
        if (m_settings.m_configPathAC.IsEmpty())
        {
            wxMessageBox(msg,_("Error"),wxICON_ERROR,this);
            return false;
        }
        if (!m_settings.m_configPathAC.EndsWith(wxString(PATHDIV)))
            m_settings.m_configPathAC+=PATHDIV;
        if (!::wxDirExists(m_settings.m_configPathAC+CSL_DEFAULT_INJECT_DIR_AC))
        {
            wxMessageBox(msg,_("Error"),wxICON_ERROR,this);
            return false;
        }
    }

    msg=wxString::Format(CSL_ERROR_SETTINGS_CONNECT_FMT_STR,
                         GetGameStr(CSL_GAME_BF),
                         CSL_DEFAULT_INJECT_DIR_BF);
    if (!m_settings.m_clientBinBF.IsEmpty())
    {
        if (m_settings.m_configPathBF.IsEmpty())
        {
            wxMessageBox(msg,_("Error"),wxICON_ERROR,this);
            return false;
        }

        if (!m_settings.m_configPathBF.EndsWith(wxString(PATHDIV)))
            m_settings.m_configPathBF+=PATHDIV;

        if (!::wxDirExists(m_settings.m_configPathBF+CSL_DEFAULT_INJECT_DIR_BF))
        {
            wxMessageBox(msg,_("Error"),wxICON_ERROR,this);
            return false;
        }
    }

    msg=wxString::Format(CSL_ERROR_SETTINGS_CONNECT_FMT_STR,
                         GetGameStr(CSL_GAME_CB),
                         CSL_DEFAULT_INJECT_DIR_CB);
    if (!m_settings.m_clientBinCB.IsEmpty())
    {
        if (m_settings.m_configPathCB.IsEmpty())
        {
            wxMessageBox(msg,_("Error"),wxICON_ERROR,this);
            return false;
        }
        if (!m_settings.m_configPathCB.EndsWith(wxString(PATHDIV)))
            m_settings.m_configPathCB+=PATHDIV;
        if (!::wxDirExists(m_settings.m_configPathCB+CSL_DEFAULT_INJECT_DIR_CB))
        {
            wxMessageBox(msg,_("Error"),wxICON_ERROR,this);
            return false;
        }
    }

    if (m_settings.m_ping_good>m_settings.m_ping_bad)
    {
        wxMessageBox(_("Threshold for good ping can't be higher than\n" \
                       "threshold for bad ping."),_("Error"),wxICON_ERROR,this);
        return false;
    }

    return true;
}
