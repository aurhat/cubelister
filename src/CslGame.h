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
#include "CslExtendedInfo.h"
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

#define CSL_DEFAULT_MASTER_WEB_PORT   80

#define CSL_DEFAULT_MASTER_SB         wxT("sauerbraten.org")
#define CSL_DEFAULT_MASTER_PATH_SB    wxT("/masterserver/retrieve.do?item=list")

#define CSL_DEFAULT_MASTER_AC         wxT("masterserver.cubers.net")
#define CSL_DEFAULT_MASTER_PATH_AC    wxT("/cgi-bin/actioncube.pl/retrieve.do?item=list")

#define CSL_DEFAULT_MASTER_BF         wxT("bloodfrontier.com")
#define CSL_DEFAULT_MASTER_PORT_BF    28800

#define CSL_DEFAULT_MASTER_CB         wxT("wouter.fov120.com")
#define CSL_DEFAULT_MASTER_PATH_CB    wxT("/cube/masterserver/retrieve.do?item=list")

#define CSL_DEFAULT_PORT_SB  28785
#define CSL_DEFAULT_PORT_AC  28763
#define CSL_DEFAULT_PORT_BF  28795
#define CSL_DEFAULT_PORT_CB  28765

#define CSL_LAST_PROTOCOL_SB   256
#define CSL_LAST_PROTOCOL_AC  1126
#define CSL_LAST_PROTOCOL_BF     0
#define CSL_LAST_PROTOCOL_CB   122

#define CSL_DEFAULT_SERVER_ADDR_SB1  wxT("85.214.113.69")   // TC1
#define CSL_DEFAULT_SERVER_ADDR_SB2  wxT("81.169.135.134")  // TC2
#define CSL_DEFAULT_SERVER_DESC_SB1  wxT("The-Conquerors")
#define CSL_DEFAULT_SERVER_DESC_SB2  wxT("The-Conquerors 2")

#define CSL_VIEW_DEFAULT      1
#define CSL_VIEW_FAVOURITE    2

enum { MM_OPEN = 0, MM_VETO, MM_LOCKED, MM_PRIVATE };

typedef enum
{
    CSL_GAME_START = 0,
    CSL_GAME_SB, CSL_GAME_AC, CSL_GAME_BF, CSL_GAME_CB,
    CSL_GAME_END
} CSL_GAMETYPE;


extern wxUint32 GetDefaultPort(const CSL_GAMETYPE type);
extern const wxChar* GetVersionStr(const CSL_GAMETYPE type,const wxInt32 protocol);
extern const wxChar* GetVersionStrSB(int n);
extern const wxChar* GetVersionStrAC(int n);
extern const wxChar* GetVersionStrBF(int n);
extern const wxChar* GetVersionStrCB(int n);
extern const wxChar* GetModeStrSB(int n);
extern const wxChar* GetModeStrAC(int n);
extern const wxChar* GetWeaponStrSB(int n);
extern const wxChar* GetWeaponStrAC(int n);
extern const wxChar* GetWeaponStrCB(int n);
extern const wxChar* GetGameStr(int n);


class CslGame;
class CslMaster;
class CslMasterConnection;


class CslServerInfo : public CslExtendedInfo
{
        friend class CslEngine;
        friend class CslGame;
        friend class CslMaster;

    public:
        CslServerInfo(const CSL_GAMETYPE type=CSL_GAME_START,
                      const wxString& host=wxT("localhost"),wxUint16 port=0,
                      const wxUint32 view=CSL_VIEW_DEFAULT,const wxUint32 lastSeen=0,
                      const wxUint32 playLast=0,const wxUint32 playTimeLastGame=0,
                      const wxUint32 playTimeTotal=0,const wxUint32 connectedTimes=0,
                      const wxString& oldDescription=wxEmptyString,
                      const wxString& password=wxEmptyString)
        {
            m_host=host;
            m_port=port ? port:GetDefaultPort(type);
            m_type=type;
            m_protocol=-1;
            m_ping=-1;
            m_timeRemain=-2;
            m_players=-1;
            m_playersMax=-1;
            m_mm=-1;
            m_isCapture=false;
            m_view=view;
            m_addr.Hostname(host);
            m_addr.Service(port ? port+1:m_port+1);
            m_pingSend=0;
            m_pingResp=0;
            m_lastSeen=lastSeen;
            m_playLast=playLast;
            m_playTimeLastGame=playTimeLastGame;
            m_playTimeTotal=playTimeTotal;
            m_connectedTimes=connectedTimes;
            m_password=password;
            m_descOld=oldDescription;
            m_lock=0;
            m_waiting=false;
        }

        bool operator==(const CslServerInfo& info) const
        {
            return (m_type==info.m_type && m_host==info.m_host && m_port==info.m_port);
        }

        void Destroy() { DeletePlayerStats(); DeleteTeamStats(); delete this; }

        void CreateFavourite(const CSL_GAMETYPE type=CSL_GAME_START,
                             const wxString& host=wxT("localhost"),const wxUint16 port=0)
        {
            m_host=host;
            m_port=port ? port:GetDefaultPort(type);
            m_type=type;
            m_view=CSL_VIEW_FAVOURITE;
            m_addr.Hostname(host);
            m_addr.Service(port ? port+1:m_port+1);
        }

        void SetLastPlayTime(const wxUint32 time)
        {
            m_playTimeLastGame=time;
            m_playTimeTotal+=time;
        }

        void SetWaiting(const bool wait=true) { m_waiting=wait; }

        void SetDescription(const wxString& description)
        {
            m_desc=description;
            m_descOld=description;
        }

        wxString GetBestDescription() const
        {
            if (m_desc.IsEmpty())
            {
                if (m_descOld.IsEmpty()) return m_host;
                else return m_descOld;
            }
            return m_desc;
        }

        const wxChar* GetVersionStr() const { return ::GetVersionStr(m_type,m_protocol); }

        void Lock(bool lock=true)
        {
            if (!lock && --m_lock<=0) m_lock=0;
            else m_lock++;
        }

        bool IsLocked() const { return m_lock>0; }
        bool IsUnused() const { return m_view==0; }
        bool IsDefault() const { return (m_view&CSL_VIEW_DEFAULT)>0; }
        bool IsFavourite() const { return (m_view&CSL_VIEW_FAVOURITE)>0; }

        wxString m_host,m_domain,m_password;
        wxUint16 m_port;
        vector<wxInt32> m_masterIDs;
        wxString m_desc,m_descOld;
        wxString m_gameMode,m_map;
        CSL_GAMETYPE m_type;
        wxInt32 m_protocol;
        wxInt32 m_ping,m_timeRemain;
        wxInt32 m_players,m_playersMax;
        wxInt32 m_mm;
        bool m_isCapture;
        wxUint32 m_view;
        wxIPV4address m_addr;
        wxUint32 m_lastSeen,m_pingSend,m_pingResp;
        wxUint32 m_playLast,m_playTimeLastGame,m_playTimeTotal;
        wxUint32 m_connectedTimes;
        wxInt32 m_lock;
        bool m_waiting;

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


enum { CSL_MASTER_CONN_HTTP, CSL_MASTER_CONN_OTHER };

class CslMasterConnection
{
        friend class CslMaster;
    public:
        CslMasterConnection() {}

        CslMasterConnection(const wxString& address,const wxString& path,
                            const wxUint16 port=CSL_DEFAULT_MASTER_WEB_PORT) :
                m_type(CSL_MASTER_CONN_HTTP),m_address(address),
                m_path(path),m_port(port) {};

        CslMasterConnection(const wxString& address,const wxUint16 port) :
                m_type(CSL_MASTER_CONN_OTHER),m_address(address),
                m_path(wxEmptyString),m_port(port) {};

        bool operator==(const CslMasterConnection& m2) const
        {
            return m_address==m2.m_address && m_path==m2.m_path;
        }

        wxInt32 GetType() const { return m_type; }
        wxString GetAddress() const { return m_address; }
        wxString GetPath() const { return m_path; }
        wxUint16 GetPort() const { return m_port; }

    private:
        wxInt32 m_type;
        wxString m_address;
        wxString m_path;
        wxUint16 m_port;
};

class CslMaster
{
        friend class CslGame;

    public:
        CslMaster(const CSL_GAMETYPE type) :
                m_game(NULL),m_type(type),m_id(0) {}

        CslMaster(const CSL_GAMETYPE type,const CslMasterConnection& connection):
                m_game(NULL),m_type(type),m_id(0),m_connection(connection) {}

        ~CslMaster() { RemoveServers(); }

        bool operator==(const CslMaster& m2) const
        {
            return m_connection==m2.m_connection;
        }

        void Create(const CSL_GAMETYPE type,const CslMasterConnection& connection)
        {
            m_type=type;
            m_connection=connection;
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

        CslGame* GetGame() const { return m_game; }
        CSL_GAMETYPE GetType() const { return m_type; }
        wxInt32 GetID() const { return m_id; }
        CslMasterConnection& GetConnection() { return m_connection; }
        vector<CslServerInfo*>* GetServers() { return &m_servers; }

    protected:
        CslGame *m_game;
        CSL_GAMETYPE m_type;
        wxInt32 m_id;
        CslMasterConnection m_connection;
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

        CSL_GAMETYPE GetType() const { return m_type; }
        wxString GetName() const { return m_name; }
        vector<CslMaster*>* GetMasters() { return &m_masters; }
        vector<CslServerInfo*>* GetServers() { return &m_servers; }
        CslServerInfo* FindServerByAddr(const wxIPV4address& addr);

        static wxString GetWeaponName(const CSL_GAMETYPE type,const wxInt32 weapon);

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
