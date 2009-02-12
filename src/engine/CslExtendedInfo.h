/***************************************************************************
 *   Copyright (C) 2007-2009 by Glen Masgai                                *
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

#ifndef CSLEXTENDEDINFO_H
#define CSLEXTENDEDINFO_H

/**
 @author Glen Masgai <mimosius@users.sourceforge.net>
*/

#include "wx/wxprec.h"
#ifdef __BORLANDC__
#pragma hdrstop
#endif
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif
#include "cube_tools.h"

#define CSL_EX_VERSION_MIN      102
#define CSL_EX_VERSION_MAX      104
// commands
#define CSL_EX_PING_UPTIME      0
#define CSL_EX_PING_PLAYERSTATS 1
#define CSL_EX_PING_TEAMSTATS   2

#define CSL_SCORE_INVALID     (1<<31)
#define CSL_SCORE_IS_VALID(x) (x!=CSL_SCORE_INVALID)


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
                Frags(0),Flagscore(0),Deaths(0),Teamkills(0),Accuracy(0),
                Health(-1),Armour(-1),Weapon(-1),ID(-1),
                Privileges(CSL_PLAYER_STATE_UNKNOWN),State(CSL_PLAYER_PRIV_UNKNOWN),
                Ping(-1),IP(0),Ok(false) {}

        wxString Name,Team;
        wxInt32 Frags,Flagscore,Deaths,Teamkills,Accuracy;
        wxInt32 Health,Armour,Weapon;
        wxInt32 ID,Privileges,State,Ping;
        wxUint32 IP;
        bool Ok;
};

class CslPlayerStats
{
    public:
        enum { CSL_STATS_NEED_IDS=0, CSL_STATS_NEED_STATS };

        CslPlayerStats();
        ~CslPlayerStats();

        CslPlayerStatsData* GetNewStats();
        bool AddStats(CslPlayerStatsData *data);
        void RemoveStats(CslPlayerStatsData *data);
        void DeleteStats();
        bool AddId(wxInt32 id);
        bool RemoveId(wxInt32 id);
        void SetWaitForStats();
        void Reset();

        wxUint32 m_status;
        wxUint32 m_lastPing,m_lastPong;
        vector<wxInt32> m_ids;
        vector<CslPlayerStatsData*> m_stats;
};

class CslTeamStatsData
{
    public:
        CslTeamStatsData() : Score(CSL_SCORE_INVALID),Score2(CSL_SCORE_INVALID),Ok(false) {}

        void Reset()
        {
            Ok=false;
            Bases.setsize(0);
        }

        wxString Name;
        wxInt32 Score,Score2;
        vector<wxInt32> Bases;
        bool Ok;
};

class CslTeamStats
{
    public:
        CslTeamStats();
        ~CslTeamStats();

        CslTeamStatsData* GetNewStats();
        void AddStats(CslTeamStatsData *data);
        void RemoveStats(CslTeamStatsData *data);
        void DeleteStats();
        void Reset();

        bool TeamMode;
        wxInt32 TimeRemain,GameMode;
        wxUint32 LastPing,LastPong;
        vector<CslTeamStatsData*> m_stats;
};


class CslExtendedInfo
{
    public:
        CslExtendedInfo() :
                ExtInfoStatus(CSL_EXT_STATUS_FALSE),
                ExtInfoVersion(0),Uptime(0),m_pingExt(0) {};

        void PingExt(bool ping) { ping ? m_pingExt++:m_pingExt-=m_pingExt>0; }
        bool PingExt() { return m_pingExt>0; }

        wxInt32 ExtInfoStatus,ExtInfoVersion;
        wxUint32 Uptime;
        CslPlayerStats PlayerStats;
        CslTeamStats TeamStats;

    private:
        wxUint32 m_pingExt;

};
#endif // CSLEXTENDEDINFO_H
