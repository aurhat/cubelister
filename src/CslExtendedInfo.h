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

#ifndef CSLEXTENDEDINFO_H
#define CSLEXTENDEDINFO_H

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
#include "cube_tools.h"

#define CSL_EX_VERSION_MIN      101
#define CSL_EX_VERSION_MAX      102
// commands
// version 101
#define CSL_EX_PING_UPTIME_STR      "ut"
#define CSL_EX_PING_PLAYERSTATS_STR "ps"
#define CSL_EX_PING_TEAMSTATS_STR   "ts"
// since version 102
#define CSL_EX_PING_UPTIME      0
#define CSL_EX_PING_PLAYERSTATS 1
#define CSL_EX_PING_TEAMSTATS   2


enum
{
    CSL_EXT_STATUS_ERROR    = -1,
    CSL_EXT_STATUS_FALSE,
    CSL_EXT_STATUS_OK,
    CSL_EXT_STATUS_MISMATCH,
};

enum
{
    CSL_PLAYER_STATE_SB_ALIVE = 0,
    CSL_PLAYER_STATE_SB_DEAD,
    CSL_PLAYER_STATE_SB_SPAWNING,
    CSL_PLAYER_STATE_SB_LAGGED,
    CSL_PLAYER_STATE_SB_EDITING,
    CSL_PLAYER_STATE_SB_SPECTATOR
};

enum
{
    CSL_PLAYER_PRIV_SB_NONE = 0,
    CSL_PLAYER_PRIV_SB_MASTER,
    CSL_PLAYER_PRIV_SB_ADMIN
};


class CslPlayerStatsData
{
    public:
        CslPlayerStatsData() :
                m_frags(-1),m_deaths(-1),m_teamkills(-1),
                m_health(-1),m_armour(-1),m_weapon(-1),
                m_id(-1),m_priv(0),m_state(0),
                m_ok(false) {}

        wxString m_player,m_team;
        wxInt32 m_frags,m_deaths,m_teamkills;
        wxInt32 m_health,m_armour,m_weapon;
        wxInt32 m_id;
        wxUint32 m_priv,m_state;
        bool m_ok;
};

class CslPlayerStats
{
    public:
        enum { CSL_STATS_NEED_IDS=0, CSL_STATS_NEED_STATS };

        CslPlayerStats();
        ~CslPlayerStats();

        CslPlayerStatsData* GetNewStatsPtr();
        bool AddStats(CslPlayerStatsData *data);
        void RemoveStats(CslPlayerStatsData *data);
        void DeleteStats();
        bool AddId(wxInt32 id);
        bool RemoveId(wxInt32 id);
        void SetWaitForStats();
        void Reset();

        wxUint32 m_status;
        wxUint32 m_lastRefresh,m_lastResponse;
        vector<wxInt32> m_ids;
        vector<CslPlayerStatsData*> m_stats;
        wxCriticalSection *m_critical;
};


class CslTeamStatsData
{
    public:
        CslTeamStatsData() : m_score(-1),m_ok(false) {}

        void Reset()
        {
            m_ok=false;
            m_bases.setsize(0);
        }

        bool IsCapture() { return m_bases.length()>0; }

        wxString m_team;
        wxInt32 m_score;
        vector<wxInt32> m_bases;
        bool m_ok;
};

class CslTeamStats
{
    public:
        CslTeamStats();
        ~CslTeamStats();

        CslTeamStatsData* GetNewStatsPtr();
        void AddStats(CslTeamStatsData *data);
        void DeleteStats();
        void Reset();

        bool m_teamplay;
        wxInt32 m_remain;
        wxUint32 m_lastRefresh,m_lastResponse;
        vector<CslTeamStatsData*> m_stats;
        wxCriticalSection *m_critical;
};

#endif // CSLEXTENDEDINFO_H
