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

#ifndef CSLSETTINGS_H
#define CSLSETTINGS_H

/**
 @author Glen Masgai <mimosius@users.sourceforge.net>
*/

#include "CslEngine.h"

#define CSL_SETTINGS_FILE  wxString(CSL_USER_DIR+wxT("settings.ini"))
#define CSL_SERVERS_FILE   wxString(CSL_USER_DIR+wxT("servers.ini"))
#define CSL_LOCATORS_FILE  wxString(CSL_USER_DIR+wxT("locators.ini"))

/** Config file version changes
*
*   1: initial version
*   2: ???
*   3: force new "Selected server" AUI layout
*
**/

#define CSL_CONFIG_VERSION         3
#define CSL_SERVERCONFIG_VERSION   1
#define CSL_LOCATORCONFIG_VERSION  1

#define CSL_FRAME_MIN_WIDTH  800
#define CSL_FRAME_MIN_HEIGHT 600

#define CSL_AUI_DEFAULT_LAYOUT wxT("")

#define CSL_FILTER_OFFLINE     (1<<0)
#define CSL_FILTER_FULL        (1<<1)
#define CSL_FILTER_EMPTY       (1<<2)
#define CSL_FILTER_NONEMPTY    (1<<3)
#define CSL_FILTER_MM2         (1<<4)
#define CSL_FILTER_MM3         (1<<5)

#define CSL_WAIT_SERVER_FULL_MIN   10
#define CSL_WAIT_SERVER_FULL_STD   60
#define CSL_WAIT_SERVER_FULL_MAX   600

#define CSL_MIN_PLAYTIME_MIN   0
#define CSL_MIN_PLAYTIME_STD   30
#define CSL_MIN_PLAYTIME_MAX   60

#define CSL_CLEANUP_SERVERS      14
#define CSL_CLEANUP_SERVERS_MAX  365

#define CSL_TOOLTIP_DELAY_MIN   0
#define CSL_TOOLTIP_DELAY_STD   1500
#define CSL_TOOLTIP_DELAY_MAX   10000
#define CSL_TOOLTIP_DELAY_STEP  500

#define CSL_TTS_VOLUME_DEFAULT  75

#define CSL_PING_GOOD_STD  200
#define CSL_PING_BAD_STD   400

#define CSL_USE_SYSTRAY    (1<<0)
#define CSL_SYSTRAY_CLOSE  (1<<1)

#define CSL_COLOUR_MASTER     wxColour(64,  255, 128)
#define CSL_COLOUR_AUTH       wxColour(192,  64, 192)
#define CSL_COLOUR_ADMIN      wxColour(255, 128,   0)
#define CSL_COLOUR_SPECTATOR  wxColour(192, 192, 192)

class CslListCtrlSettings
{
    public:
        CslListCtrlSettings(const wxString& name=wxEmptyString,
                            wxUint32 columnMask=0) :
                Name(name),
                ColumnMask(columnMask)
        { }

        wxString Name;
        wxUint32 ColumnMask;
};

WX_DEFINE_ARRAY(CslListCtrlSettings*, CslArrayListCtrlSettings);

class CslSettings
{
    public:
        CslSettings() :
                Version(CSL_CONFIG_VERSION),
                /* GUI */
                FrameSize(wxSize(CSL_FRAME_MIN_WIDTH, CSL_FRAME_MIN_HEIGHT)),
                FrameSizeMax(wxDefaultSize),
                Layout(CSL_AUI_DEFAULT_LAYOUT),
                Systray(0),
                TooltipDelay(CSL_TOOLTIP_DELAY_STD),
                TTS(true),
                TTSVolume(CSL_TTS_VOLUME_DEFAULT),
                UpdateInterval(CSL_UPDATE_INTERVAL_MIN),
                DontUpdatePlaying(true),
                ShowSearch(true),
                SearchLAN(true),
                FilterMaster(0),
                FilterFavourites(0),
                WaitServerFull(CSL_WAIT_SERVER_FULL_STD),
                PingGood(CSL_PING_GOOD_STD),
                PingBad(CSL_PING_BAD_STD),
                CleanupServers(CSL_CLEANUP_SERVERS*86400),
                CleanupServersKeepFav(true),
                CleanupServersKeepStats(true),
                AutoSaveOutput(false),
                MinPlaytime(CSL_MIN_PLAYTIME_STD),

                /* ListCtrl */
                AutoSortColumns(true),
                 /* Server lists */
                ColServerEmpty(wxColour(60, 15, 15)),
                ColServerOff(CSL_SYSCOLOUR(wxSYS_COLOUR_GRAYTEXT)),
                ColServerFull(*wxRED),
                ColServerMM1(*wxBLACK),
                ColServerMM2(*wxBLUE),
                ColServerMM3(*wxRED),
                ColServerHigh(wxColour(135, 255, 110)),
                ColServerPlay(wxColour(240, 160, 160)),
                ColInfoStripe(wxColour(235, 255, 235)),
                   /* Player lists */
                ColPlayerMaster(CSL_COLOUR_MASTER),
                ColPlayerAuth(CSL_COLOUR_AUTH),
                ColPlayerAdmin(CSL_COLOUR_ADMIN),
                ColPlayerSpectator(CSL_COLOUR_SPECTATOR)
        {}

        ~CslSettings()
        {
            CslSettings& self = GetInstance();

            WX_CLEAR_ARRAY(self.CslListSettings);
        }

        static CslSettings& GetInstance()
        {
            static CslSettings self;
            return self;
        }

        static CslListCtrlSettings& GetListSettings(const wxString& name);

        static void LoadSettings();
        static void SaveSettings();

        static bool LoadServers(wxUint32 *numm=NULL, wxUint32 *nums=NULL);
        static void SaveServers();

        wxUint32 Version;
        /* GUI */
        wxSize FrameSize, FrameSizeMax;
        wxString Layout;
        wxInt32 Systray;
        wxStringList Layouts;
        wxInt32 TooltipDelay;
        bool TTS;
        wxInt32 TTSVolume;
        wxInt32 UpdateInterval;
        bool DontUpdatePlaying;
        bool ShowSearch, SearchLAN;
        wxUint32 ColumnsMaster, ColumnsFavourites;
        wxInt32 FilterMaster, FilterFavourites;
        wxInt32 WaitServerFull;
        wxInt32 PingGood, PingBad;
        wxString GameOutputPath, ScreenOutputPath;
        wxUint32 CleanupServers;
        bool CleanupServersKeepFav, CleanupServersKeepStats;
        bool AutoSaveOutput;
        wxString LastGame;
        wxUint32 MinPlaytime;

        /* ListCtrl*/
        bool AutoSortColumns;
        /* Server lists */
        wxColour ColServerEmpty, ColServerOff, ColServerFull;
        wxColour ColServerMM1, ColServerMM2, ColServerMM3;
        wxColour ColServerHigh, ColServerPlay, ColInfoStripe;
           /* Player lists */
        wxColour ColPlayerMaster, ColPlayerAuth, ColPlayerAdmin, ColPlayerSpectator;

        CslArrayListCtrlSettings CslListSettings;
};

static inline CslSettings& CslGetSettings()
{
    return CslSettings::GetInstance();
}

#endif // CSLSETTINGS_H
