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
#include "CslEngine.h"

#define  CSL_CONFIG_VERSION        1
#define  CSL_SERVERCONFIG_VERSION  1

#define CSL_FRAME_MIN_WIDTH  800
#define CSL_FRAME_MIN_HEIGHT 600

#define CSL_FILTER_OFFLINE     1<<0
#define CSL_FILTER_FULL        1<<1
#define CSL_FILTER_EMPTY       1<<2
#define CSL_FILTER_NONEMPTY    1<<3
#define CSL_FILTER_MM2         1<<4
#define CSL_FILTER_MM3         1<<5

#define CSL_WAIT_SERVER_FULL_MIN   10
#define CSL_WAIT_SERVER_FULL_MAX   600
#define CSL_WAIT_SERVER_FULL_STD   60

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
                m_frameSize(wxSize(CSL_FRAME_MIN_WIDTH,CSL_FRAME_MIN_HEIGHT)),
                m_updateInterval(CSL_UPDATE_INTERVAL_MIN),
                m_dontUpdatePlaying(true),
                m_showFilter(true),
                m_filterMaster(0),
                m_filterFavourites(0),
                m_waitServerFull(CSL_WAIT_SERVER_FULL_STD),
                m_ping_good(CSL_PING_GOOD_STD),
                m_ping_bad(CSL_PING_BAD_STD),

                /* ListCtrl */
                m_autoFitColumns(true),
                m_autoSortColumns(true),
                m_colServerS1(0.18f),
                m_colServerS2(0.18f),
                m_colServerS3(0.09f),
                m_colServerS4(0.09f),
                m_colServerS5(0.10f),
                m_colServerS6(0.14f),
                m_colServerS7(0.07f),
                m_colServerS8(0.07f),
                m_colServerS9(0.07f),
                m_colServerEmpty(wxColour(60,15,15)),
                m_colServerOff(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT)),
                m_colServerFull(*wxRED),
                m_colServerMM1(*wxBLACK),
                m_colServerMM2(*wxBLUE),
                m_colServerMM3(*wxRED),
                m_colServerHigh(wxColour(135,255,110)),
                m_colServerPlay(wxColour(240,160,160)),
                m_colInfoStripe(wxColour(235,255,235)),

                /* Client */
                m_clientBinSB(wxEmptyString),
                m_clientOptsSB(wxEmptyString),
                m_gamePathSB(wxEmptyString),
#ifdef __WXMAC__
                m_privConfSB(true),
#else
                m_privConfSB(false),
#endif
                m_clientBinAC(wxEmptyString),
                m_clientOptsAC(wxEmptyString),
                m_gamePathAC(wxEmptyString),
                m_clientBinBF(wxEmptyString),
                m_clientOptsBF(wxEmptyString),
                m_gamePathBF(wxEmptyString),
#ifdef __WXMAC__
                m_privConfBF(true),
#else
                m_privConfBF(false),
#endif
                m_clientBinCB(wxEmptyString),
                m_clientOptsCB(wxEmptyString),
                m_gamePathCB(wxEmptyString),

                m_minPlaytime(CSL_MIN_PLAYTIME_STD)
        {}

        /* GUI */
        wxSize m_frameSize;
        wxInt32 m_updateInterval;
        bool m_dontUpdatePlaying;
        bool m_showFilter;
        wxUint32 m_filterMaster,m_filterFavourites;
        wxInt32 m_waitServerFull;
        wxUint32 m_ping_good,m_ping_bad;
        wxString m_outputPath;

        /* ListCtrl*/
        bool m_autoFitColumns,m_autoSortColumns;
        float m_colServerS1,m_colServerS2,m_colServerS3,m_colServerS4;
        float m_colServerS5,m_colServerS6,m_colServerS7,m_colServerS8,m_colServerS9;
        wxColour m_colServerEmpty,m_colServerOff,m_colServerFull;
        wxColour m_colServerMM1,m_colServerMM2,m_colServerMM3;
        wxColour m_colServerHigh;
        wxColour m_colServerPlay;
        wxColour m_colInfoStripe;

        /* Client */
        wxString m_clientBinSB;
        wxString m_clientOptsSB;
        wxString m_gamePathSB;
        bool m_privConfSB;
        wxString m_clientBinAC;
        wxString m_clientOptsAC;
        wxString m_gamePathAC;
        wxString m_clientBinBF;
        wxString m_clientOptsBF;
        wxString m_gamePathBF;
        bool m_privConfBF;
        wxString m_clientBinCB;
        wxString m_clientOptsCB;
        wxString m_gamePathCB;
        wxUint32 m_minPlaytime;
};

extern CslSettings *g_cslSettings;

#endif // CSLSETTINGS_H
