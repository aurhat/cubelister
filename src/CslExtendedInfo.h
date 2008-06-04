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
#include "CslGeoIP.h"
#include "cube_tools.h"

#define CSL_EX_VERSION_MIN      102
#define CSL_EX_VERSION_MAX      103
// commands
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
    CSL_PLAYER_STATE_UNKNOWN = -1,
    CSL_PLAYER_STATE_ALIVE,
    CSL_PLAYER_STATE_DEAD,
    CSL_PLAYER_STATE_SPAWNING,
    CSL_PLAYER_STATE_LAGGED,
    CSL_PLAYER_STATE_EDITING,
    CSL_PLAYER_STATE_SPECTATOR
};

enum
{
    CSL_PLAYER_PRIV_UNKNOWN = -1,
    CSL_PLAYER_PRIV_NONE,
    CSL_PLAYER_PRIV_MASTER,
    CSL_PLAYER_PRIV_ADMIN
};


class CslPlayerStatsData
{
    public:
        CslPlayerStatsData() :
                m_frags(0),m_deaths(0),m_teamkills(0),m_accuracy(0),
                m_health(-1),m_armour(-1),m_weapon(-1),m_id(-1),
                m_priv(CSL_PLAYER_STATE_UNKNOWN),m_state(CSL_PLAYER_PRIV_UNKNOWN),
                m_ip(0),m_ok(false) {}

        wxString m_player,m_team;
        wxInt32 m_frags,m_deaths,m_teamkills,m_accuracy;
        wxInt32 m_health,m_armour,m_weapon;
        wxInt32 m_id;
        wxInt32 m_priv,m_state;
        wxUint32 m_ip;
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
        CslTeamStatsData() : m_score(-1),m_score2(-1),m_ok(false) {}

        void Reset()
        {
            m_ok=false;
            m_bases.setsize(0);
        }

        wxString m_team;
        wxInt32 m_score,m_score2;
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


#define CSL_UPTIME_REFRESH_MULT 4

class CslExtendedInfo
{
    public:
        CslExtendedInfo() : m_extInfoStatus(CSL_EXT_STATUS_FALSE),m_extInfoVersion(0),
                m_uptime(0),m_uptimeRefresh(CSL_UPTIME_REFRESH_MULT),
                m_playerStats(NULL),m_teamStats(NULL) {};

        bool CanPingUptime() { return m_uptimeRefresh++%CSL_UPTIME_REFRESH_MULT==0; }
        void CreatePlayerStats() { if (!m_playerStats) m_playerStats=new CslPlayerStats(); }
        void DeletePlayerStats() { if (m_playerStats) { delete m_playerStats; m_playerStats=NULL; } }
        void CreateTeamStats() { if (!m_teamStats) m_teamStats=new CslTeamStats(); }
        void DeleteTeamStats() { if (m_teamStats) { delete m_teamStats; m_teamStats=NULL; } }

        wxInt32 m_extInfoStatus,m_extInfoVersion;
        wxUint32 m_uptime,m_uptimeRefresh;
        CslPlayerStats *m_playerStats;
        CslTeamStats *m_teamStats;
};

#endif // CSLEXTENDEDINFO_H
