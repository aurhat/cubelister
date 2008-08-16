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

#ifndef CSLSETTINGS_H
#define CSLSETTINGS_H

/**
 @author Glen Masgai <mimosius@gmx.de>
*/

#include "wx/wxprec.h"
#ifdef __BORLANDC__
#pragma hdrstop
#endif
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif
#include "engine/CslEngine.h"

#define  CSL_CONFIG_VERSION        1
#define  CSL_SERVERCONFIG_VERSION  1

#define CSL_FRAME_MIN_WIDTH  800
#define CSL_FRAME_MIN_HEIGHT 600

#define CSL_AUI_DEFAULT_LAYOUT wxT("")

#define CSL_FILTER_OFFLINE     1<<0
#define CSL_FILTER_FULL        1<<1
#define CSL_FILTER_EMPTY       1<<2
#define CSL_FILTER_NONEMPTY    1<<3
#define CSL_FILTER_MM2         1<<4
#define CSL_FILTER_MM3         1<<5

#define CSL_WAIT_SERVER_FULL_MIN   10
#define CSL_WAIT_SERVER_FULL_MAX   600
#define CSL_WAIT_SERVER_FULL_STD   60

#define CSL_CLEANUP_SERVERS      14
#define CSL_CLEANUP_SERVERS_MAX  365

#define CSL_MIN_PLAYTIME_MIN   0
#define CSL_MIN_PLAYTIME_MAX   60
#define CSL_MIN_PLAYTIME_STD   30

#define CSL_PING_GOOD_STD    200
#define CSL_PING_BAD_STD     400


class CslSettings
{
    public:
        CslSettings() :
                /* GUI */
                frameSize(wxSize(CSL_FRAME_MIN_WIDTH,CSL_FRAME_MIN_HEIGHT)),
                layout(CSL_AUI_DEFAULT_LAYOUT),
                updateInterval(CSL_UPDATE_INTERVAL_MIN),
                dontUpdatePlaying(true),
                showSearch(true),
                filterMaster(0),
                filterFavourites(0),
                waitServerFull(CSL_WAIT_SERVER_FULL_STD),
                pinggood(CSL_PING_GOOD_STD),
                pingbad(CSL_PING_BAD_STD),
                cleanupServers(CSL_CLEANUP_SERVERS*86400),
                cleanupServersKeepFav(true),
                cleanupServersKeepStats(true),
                autoSaveOutput(false),

                /* ListCtrl */
                autoSortColumns(true),
                colServerS1(0.18f),
                colServerS2(0.18f),
                colServerS3(0.09f),
                colServerS4(0.09f),
                colServerS5(0.10f),
                colServerS6(0.14f),
                colServerS7(0.07f),
                colServerS8(0.07f),
                colServerS9(0.07f),
                colServerEmpty(wxColour(60,15,15)),
                colServerOff(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT)),
                colServerFull(*wxRED),
                colServerMM1(*wxBLACK),
                colServerMM2(*wxBLUE),
                colServerMM3(*wxRED),
                colServerHigh(wxColour(135,255,110)),
                colServerPlay(wxColour(240,160,160)),
                colInfoStripe(wxColour(235,255,235)),

                /* Client */
                minPlaytime(CSL_MIN_PLAYTIME_STD)
        {}

        /* GUI */
        wxSize frameSize;
        wxString layout;
        wxStringList layouts;
        wxInt32 updateInterval;
        bool dontUpdatePlaying;
        bool showSearch;
        wxInt32 filterMaster,filterFavourites;
        wxInt32 waitServerFull;
        wxInt32 pinggood,pingbad;
        wxString gameOutputPath;
        wxUint32 cleanupServers;
        bool cleanupServersKeepFav,cleanupServersKeepStats;
        bool autoSaveOutput;
        wxString lastGame;

        /* ListCtrl*/
        bool autoSortColumns;
        float colServerS1,colServerS2,colServerS3,colServerS4;
        float colServerS5,colServerS6,colServerS7,colServerS8,colServerS9;
        wxColour colServerEmpty,colServerOff,colServerFull;
        wxColour colServerMM1,colServerMM2,colServerMM3;
        wxColour colServerHigh;
        wxColour colServerPlay;
        wxColour colInfoStripe;

        /* Client */
        wxUint32 minPlaytime;
};

extern CslSettings *g_cslSettings;

#endif // CSLSETTINGS_H
