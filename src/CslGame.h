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

#define CSL_DEFAULT_NAME_SB           wxT("Sauerbraten")
#define CSL_DEFAULT_NAME_AC           wxT("AssaultCube")
#define CSL_DEFAULT_NAME_CB           wxT("Cube")

#define CSL_DEFAULT_MASTER_SB         wxT("sauerbraten.org")
#define CSL_DEFAULT_MASTER_PATH_SB    wxT("/masterserver/retrieve.do?item=list")

#define CSL_DEFAULT_MASTER_AC         wxT("masterserver.cubers.net")
#define CSL_DEFAULT_MASTER_PATH_AC    wxT("/cgi-bin/actioncube.pl/retrieve.do?item=list")

#define CSL_DEFAULT_MASTER_CB         wxT("wouter.fov120.com")
#define CSL_DEFAULT_MASTER_PATH_CB    wxT("/cube/masterserver/retrieve.do?item=list")

#define CSL_DEFAULT_INFO_PORT_SB      28786
#define CSL_DEFAULT_INFO_PORT_AC      28764
#define CSL_DEFAULT_INFO_PORT_CB      28766

#define CSL_LAST_PROTOCOL_SB  254
#define CSL_LAST_PROTOCOL_AC  1125
#define CSL_LAST_PROTOCOL_CB  122

#define CSL_DEFAULT_SERVER_ADDR_SB1  wxT("81.169.170.173")   // TC1
#define CSL_DEFAULT_SERVER_ADDR_SB2  wxT("85.214.41.161")    // TC2
#define CSL_DEFAULT_SERVER_DESC_SB1  wxT("The-Conquerors")
#define CSL_DEFAULT_SERVER_DESC_SB2  wxT("The-Conquerors 2")

#define CSL_VIEW_DEFAULT      1
#define CSL_VIEW_FAVOURITE    2

enum { MM_OPEN = 0, MM_VETO, MM_LOCKED, MM_PRIVATE };

typedef enum
{
    CSL_GAME_START = 0,
    CSL_GAME_SB, CSL_GAME_AC, CSL_GAME_CB,
    CSL_GAME_END
} CSL_GAMETYPE;


extern const wxChar* GetVersionStrSB(int n);
extern const wxChar* GetVersionStrAC(int n);
extern const wxChar* GetVersionStrCB(int n);
extern const wxChar* GetModeStrSB(int n);
extern const wxChar* GetModeStrAC(int n);
extern const wxChar* GetGameStr(int n);


class CslGame;
class CslMaster;

class CslServerInfo
{
        friend class CslEngine;
        friend class CslGame;
        friend class CslMaster;

    public:
        CslServerInfo(wxString host=wxT("localhost"),CSL_GAMETYPE type=CSL_GAME_START,
                      wxUint32 view=CSL_VIEW_DEFAULT, wxUint32 lastSeen=0,
                      wxUint32 playLast=0,wxUint32 playTimeLastGame=0,
                      wxUint32 playTimeTotal=0,wxUint32 connectedTimes=0)
        {
            m_host=host;
            m_type=type;
            m_protocol=-1;
            m_ping=-1;
            m_timeRemain=-2;
            m_players=-1;
            m_playersMax=-1;
            m_mm=-1;
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
            m_locked=false;
            m_waiting=false;
        }

        bool operator==(const CslServerInfo& i2)
        {
            return (m_type==i2.m_type && m_host==i2.m_host);
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

        wxString GetVersionStr()
        {
            switch (m_type)
            {
                case CSL_GAME_SB:
                    return GetVersionStrSB(m_protocol);
                case CSL_GAME_AC:
                    return GetVersionStrAC(m_protocol);
                case CSL_GAME_CB:
                    return GetVersionStrCB(m_protocol);
                default:
                    break;
            }
            return wxEmptyString;
        }

        void Lock(bool lock=true) { m_locked=lock; }
        void SetWaiting(bool wait=true) { m_waiting=wait; }

        bool IsLocked() { return m_locked; }
        bool IsUnused() { return m_view==0; }
        bool IsDefault() { return (m_view&CSL_VIEW_DEFAULT)>0; }
        bool IsFavourite() { return (m_view&CSL_VIEW_FAVOURITE)>0; }

        wxString m_host,m_domain;
        vector<wxInt32> m_masterIDs;
        wxString m_desc,m_gameMode,m_map;
        CSL_GAMETYPE m_type;
        wxInt32 m_protocol;
        wxInt32 m_ping,m_timeRemain;
        wxInt32 m_players,m_playersMax;
        wxInt32 m_mm;
        wxUint32 m_view;
        wxIPV4address m_addr;
        wxUint32 m_lastSeen,m_pingSend,m_pingResp;
        wxUint32 m_playLast,m_playTimeLastGame,m_playTimeTotal;
        wxUint32 m_connectedTimes;
        bool m_locked,m_waiting;

    protected:
        void AddMaster(wxInt32 id)
        {
            m_masterIDs.add(id);
            SetDefault();
        }
        void RemoveMaster(wxInt32 id)
        {
            wxInt32 i=m_masterIDs.find(id);
            if (id>=0) m_masterIDs.remove(i);
            if (m_masterIDs.length()==0) RemoveDefault();
        }

        void SetDefault() { m_view|=CSL_VIEW_DEFAULT; }
        void SetFavourite() { m_view|=CSL_VIEW_FAVOURITE; }
        void RemoveDefault() { m_view&=~CSL_VIEW_DEFAULT; }
        void RemoveFavourite() { m_view&=~CSL_VIEW_FAVOURITE; }

        wxUint32 GetDefaultPort(CSL_GAMETYPE type)
        {
            switch (type)
            {
                case CSL_GAME_SB:
                    return CSL_DEFAULT_INFO_PORT_SB;
                    break;
                case CSL_GAME_AC:
                    return CSL_DEFAULT_INFO_PORT_AC;
                    break;
                case CSL_GAME_CB:
                    return CSL_DEFAULT_INFO_PORT_CB;
                    break;
                default:
                    break;
            }
            return 0;
        }
};


class CslMaster
{
        friend class CslGame;

    public:
        CslMaster(CSL_GAMETYPE type=CSL_GAME_START,wxString address=wxEmptyString,
                  wxString path=wxEmptyString,bool def=false)
                : m_game(NULL),m_address(address),m_path(path),m_type(type),m_id(0) {}

        ~CslMaster() { RemoveServers(); }

        bool operator==(const CslMaster& m2)
        {
            return m_address==m2.m_address && m_path==m2.m_path;
        }

        void Create(CSL_GAMETYPE type,wxString address=wxEmptyString,wxString path=wxEmptyString)
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
        CslServerInfo* FindServerByAddr(wxIPV4address addr);

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
