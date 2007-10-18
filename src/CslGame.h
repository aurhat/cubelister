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

#ifndef CSLGAME_H
#define CSLGAME_H

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
#include <wx/sckaddr.h>
#include "cube_tools.h"


#define CSL_ERROR_NONE               0
#define CSL_ERROR_GAME_UNKNOWN      -1
#define CSL_ERROR_FILE_OPERATION   -20
#define CSL_ERROR_FILE_DONT_EXIST  -21


#define CSL_DEFAULT_INJECT_DIR_SB     wxT("packages/base/")
#define CSL_DEFAULT_INJECT_FIL_SB     wxT("csl_start_sb")
#define CSL_DEFAULT_INJECT_DIR_AC     wxT("config/")
#define CSL_DEFAULT_INJECT_FIL_AC     wxT("autoexec.cfg")
#define CSL_DEFAULT_INJECT_DIR_BF     CSL_DEFAULT_INJECT_DIR_SB
#define CSL_DEFAULT_INJECT_FIL_BF     CSL_DEFAULT_INJECT_FIL_SB
#define CSL_DEFAULT_INJECT_DIR_CB     wxT("packages/base/")
#define CSL_DEFAULT_INJECT_FIL_CB     wxT("metl3.cfg")

#define CSL_DEFAULT_NAME_SB           wxT("Sauerbraten")
#define CSL_DEFAULT_NAME_AC           wxT("AssaultCube")
#define CSL_DEFAULT_NAME_BF           wxT("Blood Frontier")
#define CSL_DEFAULT_NAME_CB           wxT("Cube")

#define CSL_DEFAULT_MASTER_SB         wxT("sauerbraten.org")
#define CSL_DEFAULT_MASTER_PATH_SB    wxT("/masterserver/retrieve.do?item=list")

#define CSL_DEFAULT_MASTER_AC         wxT("masterserver.cubers.net")
#define CSL_DEFAULT_MASTER_PATH_AC    wxT("/cgi-bin/actioncube.pl/retrieve.do?item=list")

#define CSL_DEFAULT_MASTER_BF         wxT("acord.woop.us")
#define CSL_DEFAULT_MASTER_PATH_BF    wxT("/retrieve.do?item=list")

#define CSL_DEFAULT_MASTER_CB         wxT("wouter.fov120.com")
#define CSL_DEFAULT_MASTER_PATH_CB    wxT("/cube/masterserver/retrieve.do?item=list")

#define CSL_DEFAULT_INFO_PORT_SB      28786
#define CSL_DEFAULT_INFO_PORT_AC      28764
#define CSL_DEFAULT_INFO_PORT_BF      28796
#define CSL_DEFAULT_INFO_PORT_CB      28766

#define CSL_LAST_PROTOCOL_SB   254
#define CSL_LAST_PROTOCOL_AC  1125
#define CSL_LAST_PROTOCOL_BF   254
#define CSL_LAST_PROTOCOL_CB   122

#define CSL_DEFAULT_SERVER_ADDR_SB1  wxT("81.169.170.173")   // TC1
#define CSL_DEFAULT_SERVER_ADDR_SB2  wxT("85.214.41.161")    // TC2
#define CSL_DEFAULT_SERVER_DESC_SB1  wxT("The-Conquerors")
#define CSL_DEFAULT_SERVER_DESC_SB2  wxT("The-Conquerors 2")

#define CSL_UPTIME_REFRESH_MULT 4

#define CSL_VIEW_DEFAULT      1
#define CSL_VIEW_FAVOURITE    2

enum { MM_OPEN = 0, MM_VETO, MM_LOCKED, MM_PRIVATE };

typedef enum
{
    CSL_GAME_START = 0,
    CSL_GAME_SB, CSL_GAME_AC, CSL_GAME_BF, CSL_GAME_CB,
    CSL_GAME_END
} CSL_GAMETYPE;


extern const wxChar* GetVersionStrSB(int n);
extern const wxChar* GetVersionStrAC(int n);
extern const wxChar* GetVersionStrBF(int n);
extern const wxChar* GetVersionStrCB(int n);
extern const wxChar* GetModeStrSB(int n);
extern const wxChar* GetModeStrAC(int n);
extern const wxChar* GetWeaponStrSB(int n);
extern const wxChar* GetGameStr(int n);


class CslGame;
class CslMaster;

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

        CslPlayerStats() : m_status(CSL_STATS_NEED_IDS),
                m_lastRefresh(0),m_lastResponse(0) { m_critical=new wxCriticalSection(); }

        ~CslPlayerStats() { DeleteStats(); delete m_critical; }

        CslPlayerStatsData* GetNewStatsPtr()
        {
            wxCriticalSectionLocker enter(*m_critical);

            loopv(m_stats)
            {
                if (m_stats[i]->m_ok) continue;
                return m_stats[i];
            }
            return new CslPlayerStatsData;
        }

        bool AddStats(CslPlayerStatsData *data)
        {
            wxCriticalSectionLocker enter(*m_critical);

            if (m_status!=CSL_STATS_NEED_STATS)
                return false;

            loopv(m_ids)
            {
                if (m_ids[i]!=data->m_id)
                    continue;
                m_ids.remove(i);
                data->m_ok=true;
                if (m_ids.length()==0)
                    m_status=CSL_STATS_NEED_IDS;
                loopv(m_stats) if (m_stats[i]==data) return true;
                m_stats.add(data);
                return true;
            }
            return false;
        }

        void RemoveStats(CslPlayerStatsData *data)
        {
            wxCriticalSectionLocker enter(*m_critical);

            loopv(m_stats)
            {
                if (m_stats[i]==data)
                {
                    m_stats[i]->m_ok=false;
                    return;
                }
            }
            delete data;
        }

        void DeleteStats()
        {
            wxCriticalSectionLocker enter(*m_critical);

            m_status=CSL_STATS_NEED_IDS;
            loopvrev(m_stats) delete m_stats[i];
            m_stats.setsize(0);
        }

        void Reset()
        {
            wxCriticalSectionLocker enter(*m_critical);

            m_status=CSL_STATS_NEED_IDS;
            loopv(m_stats) m_stats[i]->m_ok=false;
            loopv(m_ids) m_ids.remove(0);
        }

        bool AddId(wxInt32 id)
        {
            wxCriticalSectionLocker enter(*m_critical);

            if (m_status!=CSL_STATS_NEED_IDS)
                return false;
            loopv(m_ids) if (m_ids[i]==id)
                return false;
            m_ids.add(id);
            return true;
        }

        bool RemoveId(wxInt32 id)
        {
            wxCriticalSectionLocker enter(*m_critical);

            loopv(m_ids)
            {
                if (m_ids[i]!=id)
                    continue;
                m_ids.remove(i);
                if (m_ids.length()==0)
                    m_status=CSL_STATS_NEED_IDS;
                return true;
            }
            return false;
        }

        void SetWaitForStats()
        {
            if (m_ids.length())
                m_status=CSL_STATS_NEED_STATS;
            else
                m_status=CSL_STATS_NEED_IDS;
        }

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
        CslTeamStats() :
                m_teamplay(false),m_remain(-1),
                m_lastRefresh(0),m_lastResponse(0)
        {
            m_critical=new wxCriticalSection();
        }
        ~CslTeamStats() { DeleteStats(); delete m_critical; }

        CslTeamStatsData* GetNewStatsPtr()
        {
            wxCriticalSectionLocker enter(*m_critical);

            loopv(m_stats)
            {
                if (m_stats[i]->m_ok) continue;
                return m_stats[i];
            }
            return new CslTeamStatsData;
        }

        void AddStats(CslTeamStatsData *data)
        {
            wxCriticalSectionLocker enter(*m_critical);

            data->m_ok=true;
            loopv(m_stats) if (m_stats[i]==data) return;
            m_stats.add(data);
        }

        void DeleteStats()
        {
            wxCriticalSectionLocker enter(*m_critical);

            loopvrev(m_stats) delete m_stats[i];
            m_stats.setsize(0);
        }

        void Reset()
        {
            wxCriticalSectionLocker enter(*m_critical);

            loopv(m_stats) m_stats[i]->Reset();
        }

        bool m_teamplay;
        wxInt32 m_remain;
        wxUint32 m_lastRefresh,m_lastResponse;
        vector<CslTeamStatsData*> m_stats;
        wxCriticalSection *m_critical;
};

enum { CSL_EXINFO_OK = 0, CSL_EXINFO_FALSE, CSL_EXINFO_ERROR };

class CslServerInfo
{
        friend class CslEngine;
        friend class CslGame;
        friend class CslMaster;

    public:
        CslServerInfo(const wxString& host=wxT("localhost"),
                      const CSL_GAMETYPE type=CSL_GAME_START,
                      const wxUint32 view=CSL_VIEW_DEFAULT,const wxUint32 lastSeen=0,
                      const wxUint32 playLast=0,const wxUint32 playTimeLastGame=0,
                      const wxUint32 playTimeTotal=0,const wxUint32 connectedTimes=0,
                      const wxString& oldDescription=wxEmptyString,
                      const wxString& password=wxEmptyString)
        {
            m_host=host;
            m_type=type;
            m_protocol=-1;
            m_ping=-1;
            m_timeRemain=-2;
            m_players=-1;
            m_playersMax=-1;
            m_mm=-1;
            m_hasBases=false;
            m_view=view;
            m_addr.Hostname(host);
            m_addr.Service(GetDefaultPort(type));
            m_pingSend=0;
            m_pingResp=0;
            m_lastSeen=lastSeen;
            m_playLast=playLast;
            m_playTimeLastGame=playTimeLastGame;
            m_playTimeTotal=playTimeTotal;
            m_connectedTimes=connectedTimes;
            m_password=password;
            m_descOld=oldDescription;
            m_uptime=0;
            m_uptimeRefresh=CSL_UPTIME_REFRESH_MULT;
            m_lock=0;
            m_waiting=false;
            m_exInfo=CSL_EXINFO_FALSE;
            m_playerStats=NULL;
            m_teamStats=NULL;
        }

        //virtual ~CslServerInfo() { DeletePlayerStats(); }

        bool operator==(const CslServerInfo& info)
        {
            return (m_type==info.m_type && m_host==info.m_host);
        }

        void CreateFavourite(const wxString& host=wxT("localhost"),
                             CSL_GAMETYPE type=CSL_GAME_START)
        {
            m_host=host;
            m_type=type;
            m_view=CSL_VIEW_FAVOURITE;
            m_addr.Hostname(host);
            m_addr.Service(GetDefaultPort(type));
        }

        void SetLastPlayTime(wxUint32 time)
        {
            m_playTimeLastGame=time;
            m_playTimeTotal+=time;
        }

        void SetWaiting(bool wait=true) { m_waiting=wait; }

        wxString GetVersionStr()
        {
            switch (m_type)
            {
                case CSL_GAME_SB:
                    return GetVersionStrSB(m_protocol);
                case CSL_GAME_AC:
                    return GetVersionStrAC(m_protocol);
                case CSL_GAME_BF:
                    return GetVersionStrBF(m_protocol);
                case CSL_GAME_CB:
                    return GetVersionStrCB(m_protocol);
                default:
                    break;
            }
            return wxEmptyString;
        }

        wxUint32 GetDefaultPort(const CSL_GAMETYPE type)
        {
            switch (type)
            {
                case CSL_GAME_SB:
                    return CSL_DEFAULT_INFO_PORT_SB;
                    break;
                case CSL_GAME_AC:
                    return CSL_DEFAULT_INFO_PORT_AC;
                    break;
                case CSL_GAME_BF:
                    return CSL_DEFAULT_INFO_PORT_BF;
                    break;
                case CSL_GAME_CB:
                    return CSL_DEFAULT_INFO_PORT_CB;
                    break;
                default:
                    break;
            }
            return 0;
        }

        void SetDescription(const wxString& description)
        {
            m_desc=description;
            m_descOld=description;
        }

        wxString GetBestDescription()
        {
            if (m_desc.IsEmpty())
            {
                if (m_descOld.IsEmpty())
                    return m_host;
                else return m_descOld;
            }
            return m_desc;
        }

        void Lock(bool lock=true)
        {
            if (!lock && --m_lock<=0) m_lock=0;
            else m_lock++;
        }

        bool CanPingUptime() { return m_uptimeRefresh++%CSL_UPTIME_REFRESH_MULT==0; }

        bool IsLocked() { return m_lock>0; }
        bool IsUnused() { return m_view==0; }
        bool IsDefault() { return (m_view&CSL_VIEW_DEFAULT)>0; }
        bool IsFavourite() { return (m_view&CSL_VIEW_FAVOURITE)>0; }

        void CreatePlayerStats() { if (!m_playerStats) m_playerStats=new CslPlayerStats(); }
        void DeletePlayerStats() { if (m_playerStats) { delete m_playerStats; m_playerStats=NULL; } }
        void CreateTeamStats() { if (!m_teamStats) m_teamStats=new CslTeamStats(); }
        void DeleteTeamStats() { if (m_teamStats) { delete m_teamStats; m_teamStats=NULL; } }

        wxString m_host,m_domain,m_password;
        vector<wxInt32> m_masterIDs;
        wxString m_desc,m_descOld;
        wxString m_gameMode,m_map;
        CSL_GAMETYPE m_type;
        wxInt32 m_protocol;
        wxInt32 m_ping,m_timeRemain;
        wxInt32 m_players,m_playersMax;
        wxInt32 m_mm;
        bool m_hasBases;
        wxUint32 m_view;
        wxIPV4address m_addr;
        wxUint32 m_lastSeen,m_pingSend,m_pingResp;
        wxUint32 m_playLast,m_playTimeLastGame,m_playTimeTotal;
        wxUint32 m_connectedTimes;
        wxUint32 m_uptime;
        wxUint32 m_uptimeRefresh;
        wxInt32 m_lock;
        bool m_waiting;
        wxInt32 m_exInfo;
        CslPlayerStats *m_playerStats;
        CslTeamStats *m_teamStats;

    protected:
        void AddMaster(const wxInt32 id)
        {
            m_masterIDs.add(id);
            SetDefault();
        }
        void RemoveMaster(const wxInt32 id)
        {
            wxInt32 i=m_masterIDs.find(id);
            if (id>=0) m_masterIDs.remove(i);
            if (m_masterIDs.length()==0) RemoveDefault();
        }

        void SetDefault() { m_view|=CSL_VIEW_DEFAULT; }
        void SetFavourite() { m_view|=CSL_VIEW_FAVOURITE; }
        void RemoveDefault() { m_view&=~CSL_VIEW_DEFAULT; }
        void RemoveFavourite() { m_view&=~CSL_VIEW_FAVOURITE; }
};


class CslMaster
{
        friend class CslGame;

    public:
        CslMaster(const CSL_GAMETYPE type=CSL_GAME_START,
                  const wxString& address=wxEmptyString,
                  const wxString& path=wxEmptyString,
                  const bool def=false) :
                m_game(NULL),m_address(address),m_path(path),
                m_type(type),m_id(0) {}

        ~CslMaster() { RemoveServers(); }

        bool operator==(const CslMaster& m2)
        {
            return m_address==m2.m_address && m_path==m2.m_path;
        }

        void Create(const CSL_GAMETYPE type,
                    const wxString& address=wxEmptyString,
                    const wxString& path=wxEmptyString)
        {
            m_type=type;
            m_address=address;
            m_path=path;
        }

        void AddServer(CslServerInfo *info)
        {
            wxInt32 i=m_servers.find(info);
            if (i<0)
            {
                m_servers.add(info);
                info->AddMaster(m_id);
            }
        }

        bool RemoveServer(CslServerInfo *info)
        {
            wxInt32 i=m_servers.find(info);
            if (i>=0)
            {
                m_servers.remove(i);
                info->RemoveMaster(m_id);
                return true;
            }
            return false;
        }

        void RemoveServers()
        {
            loopvrev(m_servers)
            RemoveServer(m_servers[i]);
        }

        CslGame* GetGame() { return m_game; }
        wxString GetAddress() { return m_address; }
        wxString GetPath() { return m_path; }
        CSL_GAMETYPE GetType() { return m_type; }
        wxInt32 GetID() { return m_id; }
        vector<CslServerInfo*>* GetServers() { return &m_servers; }

    protected:
        CslGame *m_game;
        wxString m_address;
        wxString m_path;
        CSL_GAMETYPE m_type;
        wxInt32 m_id;
        vector<CslServerInfo*> m_servers;

        void Init(CslGame *game,wxUint32 id)
        {
            m_game=game;
            m_id=id;
        }
};


class CslGame
{
        friend class CslEngine;

    public:
        CslGame(CSL_GAMETYPE type);
        ~CslGame();

        CSL_GAMETYPE GetType() { return m_type; }
        wxString GetName() { return m_name; }
        vector<CslMaster*>* GetMasters() { return &m_masters; }
        vector<CslServerInfo*>* GetServers() { return &m_servers; }
        CslServerInfo* FindServerByAddr(const wxIPV4address& addr);

        static wxString GetGameName(const CSL_GAMETYPE type);
        static wxInt32 ConnectCleanupConfig(const CSL_GAMETYPE type,const wxString& cfg);
        static wxInt32 ConnectWriteConfig(const CSL_GAMETYPE& type,const wxString& cfg,const wxString& str);
        static wxInt32 ConnectPrepareConfig(wxString& out,const CslServerInfo *info,const wxString& path,
                                            const wxString& password=wxEmptyString,const bool admin=false);

    protected:
        CSL_GAMETYPE m_type;
        wxString m_name;

        wxInt32 m_masterID;
        vector<wxInt32> m_masterIDs;

        vector<CslMaster*> m_masters;
        vector<CslServerInfo*> m_servers;

        wxInt32 AddMaster(CslMaster *master);
        void DeleteMaster(CslMaster *master=NULL);

        CslServerInfo* AddServer(CslServerInfo *info,wxInt32 id);
        bool RemoveServer(CslServerInfo *info,CslMaster *master);
        bool DeleteServer(CslServerInfo *info);
};

#endif // CSLGAME_H
