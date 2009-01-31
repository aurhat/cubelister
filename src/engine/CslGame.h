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

#include <wx/sckaddr.h>
#include "CslExtendedInfo.h"

#define CSL_DEFAULT_MASTER_WEB_PORT   80

#define CSL_CAPABILITY_EXTINFO             1<<0
#define CSL_CAPABILITY_CONNECT_PASS        1<<1
#define CSL_CAPABILITY_CONNECT_ADMIN_PASS  1<<2
#define CSL_CAPABILITY_CUSTOM_CONFIG       1<<3

#define CSL_CAP_EXTINFO(x)             ((x&CSL_CAPABILITY_EXTINFO)!=0)
#define CSL_CAP_CONNECT_PASS(x)        ((x&CSL_CAPABILITY_CONNECT_PASS)!=0)
#define CSL_CAP_CONNECT_ADMIN_PASS(x)  ((x&CSL_CAPABILITY_CONNECT_ADMIN_PASS)!=0)
#define CSL_CAP_CUSTOM_CONFIG(x)       ((x&CSL_CAPABILITY_CUSTOM_CONFIG)!=0)

enum
{
    CSL_SERVER_OPEN,
    CSL_SERVER_VETO,
    CSL_SERVER_LOCKED,
    CSL_SERVER_PRIVATE,
    CSL_SERVER_PASSWORD  = 1<<8,
    CSL_SERVER_BAN       = 1<<9,
    CSL_SERVER_BLACKLIST = 1<<10,
};
#define CSL_SERVER_IS_OPEN(x)       ((x&0xff)==0)
#define CSL_SERVER_IS_VETO(x)       ((x&0xff)==1)
#define CSL_SERVER_IS_LOCKED(x)     ((x&0xff)==2)
#define CSL_SERVER_IS_PRIVATE(x)    ((x&0xff)==3)
#define CSL_SERVER_IS_PASSWORD(x)   (x>-1&&x&CSL_SERVER_PASSWORD)
#define CSL_SERVER_IS_BAN(x)        (x>-1&&x&CSL_SERVER_BAN)
#define CSL_SERVER_IS_BLACKLIST(x)  (x>-1&&x&CSL_SERVER_BLACKLIST)


class CslMaster;
class CslServerInfo;


class CslMasterConnection
{
    public:

        enum { CONNECTION_HTTP, CONNECTION_OTHER };

        CslMasterConnection() : m_type(-1), m_port(0) {}

        CslMasterConnection(const wxString& address,const wxString& path,
                            wxUint16 port=CSL_DEFAULT_MASTER_WEB_PORT) :
                m_type(CONNECTION_HTTP),m_address(address),
                m_path(path),m_port(port) {};

        CslMasterConnection(const wxString& address,wxUint16 port) :
                m_type(CONNECTION_OTHER),m_address(address),
                m_path(wxEmptyString),m_port(port) {};

        bool operator==(const CslMasterConnection& conn) const
        {
            return m_type==conn.m_type && m_address==conn.m_address &&
                   m_path==conn.m_path && m_port==conn.m_port;
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


class CslGameClientSettings
{
    public:
        CslGameClientSettings() : Expert(false) {}
        CslGameClientSettings(const wxString& binary,const wxString& path,const wxString& configpath,
                              const wxString& options,bool priv) :
                Binary(binary),GamePath(path),ConfigPath(configpath),
                Options(options),Expert(priv) {}

        wxString Binary,GamePath,ConfigPath,Options;
        bool Expert;
};

class CslGame
{
        friend class CslEngine;

    public:
        enum { CSL_CONFIG_DIR, CSL_CONFIG_EXE };

        CslGame();
        virtual ~CslGame();

        bool operator==(const CslGame& game) const
        {
            return m_name.CmpNoCase(game.m_name)==0;
        }

        wxInt32 AddMaster(CslMaster *master);
        bool DeleteMaster(wxInt32 masterID,wxInt32 pos=-1);
        void DeleteMasters();
        bool AddServer(CslServerInfo *info,wxInt32 masterID=-1);
        bool DeleteServer(CslServerInfo *info);

        vector<CslMaster*>& GetMasters() { return m_masters; }
        vector<CslServerInfo*>& GetServers() { return m_servers; }
        void GetFavourites(vector <CslServerInfo*>& servers);
        void GetExtServers(vector <CslServerInfo*>& servers,bool all=false);
        CslServerInfo* FindServerByAddr(const wxString host,wxUint16 port);
        CslServerInfo* FindServerByAddr(const wxIPV4address& addr);

        wxInt32 GetId() { return m_gameId; }
        wxString GetName() { return m_name; }
        wxUint32 GetCapabilities() { return m_capabilities; }
        wxUint32 GetConfigType() { return m_configType; }
        CslMasterConnection& GetDefaultMasterConnection() { return m_defaultMasterConnection; }

        CslGameClientSettings& GetClientSettings() { return  m_clientSettings; }

        virtual void GetPlayerstatsDescriptions(vector<wxString>& desc) const;
        virtual const wxChar* GetWeaponName(wxInt32 n) const { return wxEmptyString; }
        virtual bool ModeHasFlags(wxInt32 mode,wxInt32 prot) const { return false; }
        virtual bool ModeHasBases(wxInt32 mode,wxInt32 prot) const { return false; }
        virtual wxInt32 ModeScoreLimit(wxInt32 mode,wxInt32 prot) const { return -1; }
        virtual wxInt32 GetBestTeam(CslTeamStats& stats,wxInt32 prot) const { return -1; }
        virtual wxUint16 GetDefaultGamePort() const = 0;
        virtual wxUint16 GetInfoPort(wxUint16 gamePort=0) const = 0;
        virtual bool ParseDefaultPong(ucharbuf& buf,CslServerInfo& info) const = 0;
        virtual bool ParsePlayerPong(wxUint32 protocol,ucharbuf& buf,CslPlayerStatsData& info) const { return false; }
        virtual bool ParseTeamPong(wxUint32 protocol,ucharbuf& buf,CslTeamStatsData& info) const { return false; }
        virtual bool ValidateClientSettings(CslGameClientSettings *settings=NULL,wxString *msg=NULL) { return true; }
        virtual void SetClientSettings(const CslGameClientSettings& settings) { m_clientSettings=settings; }
        virtual wxString GameStart(CslServerInfo *info,wxUint32 mode,wxString *error) = 0;
        virtual wxInt32 GameEnd(wxString *error=NULL) = 0;
        virtual const char** GetIcon(wxInt32 size) const { return NULL; }
        //hooks for workarounds or special handling
        virtual void ProcessOutput(char *data,wxInt32 *len) const {}
        virtual bool ReturnOk(wxInt32 code) const { return code==0; }

    private:
        vector<CslMaster*> m_masters;
        vector<CslServerInfo*> m_servers;
        void SetGameID(wxInt32 gameId) { m_gameId=gameId; }

    protected:
        wxString m_name;
        wxInt32 m_gameId;
        wxUint32 m_capabilities;
        wxUint32 m_configType;

        CslMasterConnection m_defaultMasterConnection;
        CslGameClientSettings m_clientSettings;
};


class CslMaster
{
        friend class CslGame;

    public:
        CslMaster();
        CslMaster(const CslMasterConnection& connection);
        ~CslMaster();

        bool operator==(const CslMaster& master) const
        {
            return m_connection==master.m_connection;
        }

        void Create(const CslMasterConnection& connection);
        bool UnrefServer(CslServerInfo *info,wxInt32 pos=-1);
        void UnrefServers();

        CslGame* GetGame() const { return m_game; }
        wxInt32 GetId() const { return m_id; }
        CslMasterConnection& GetConnection() { return m_connection; }
        vector<CslServerInfo*>& GetServers() { return m_servers; }

    protected:
        CslGame *m_game;
        wxInt32 m_id;
        CslMasterConnection m_connection;
        vector<CslServerInfo*> m_servers;

        void Init(CslGame *game,wxUint32 id);
        void AddServer(CslServerInfo *info);
};


class CslServerInfo : public CslExtendedInfo
{
        friend class CslGame;
        friend class CslMaster;

    public:
        enum { CSL_VIEW_DEFAULT = 1<<0, CSL_VIEW_FAVOURITE = 1<<1 };
        enum { CSL_CONNECT_DEFAULT, CSL_CONNECT_PASS, CSL_CONNECT_ADMIN_PASS };

        CslServerInfo(CslGame *game=NULL,
                      const wxString& host=wxT("localhost"),
                      wxUint16 gamePort=0,wxUint16 infoPort=0,
                      wxUint32 view=CSL_VIEW_DEFAULT,wxUint32 lastSeen=0,
                      wxUint32 playedLast=0,wxUint32 playTimeLast=0,
                      wxUint32 playTimeTotal=0,wxUint32 connectedTimes=0,
                      const wxString& oldDescription=wxEmptyString,
                      const wxString& pass=wxEmptyString,
                      const wxString& passAdm=wxEmptyString);

        bool operator==(const CslServerInfo& info) const
        {
            return (Host==info.Host && GamePort==info.GamePort && InfoPort==info.InfoPort);
        }

        bool Create(CslGame *game,const wxString& host=wxT("localhost"),
                    wxUint16 gamePort=0,wxUint16 infoPort=0,
                    const wxString& pass=wxEmptyString,
                    const wxString& passAdm=wxEmptyString);
        void Destroy();
        void SetLastPlayTime(wxUint32 time);

        void SetDescription(const wxString& description);
        wxString GetBestDescription() const;

        CslGame& GetGame() const { return *m_game; }
        const vector<wxInt32>& GetMasterIDs() const { return m_masterIDs; }

        void Lock(bool lock=true);
        bool IsLocked() const { return m_lock>0; }

        void SetDefault() { View|=CSL_VIEW_DEFAULT; }
        void SetFavourite() { View|=CSL_VIEW_FAVOURITE; }
        void RemoveDefault() { View&=~CSL_VIEW_DEFAULT; }
        void RemoveFavourite() { View&=~CSL_VIEW_FAVOURITE; }

        bool IsUnused() const { return View==0; }
        bool IsDefault() const { return (View&CSL_VIEW_DEFAULT)>0; }
        bool IsFavourite() const { return (View&CSL_VIEW_FAVOURITE)>0; }

        void SetWaiting(bool wait=true) { m_waiting=wait; }
        bool IsWaiting() { return m_waiting; }

        bool HasStats() const;

        wxString Host,Domain;
        wxUint16 GamePort,InfoPort;
        wxIPV4address Addr;
        bool Pingable;
        wxString Password,PasswordAdmin;
        wxString Description,DescriptionOld;
        wxString GameMode,Map;
        wxString Version;
        wxString MMDescription;
        wxInt32 MM;
        wxInt32 Protocol;
        wxInt32 Ping,TimeRemain;
        wxInt32 Players,PlayersMax;
        wxUint32 View;
        wxUint32 LastSeen,PingSend,PingResp;
        wxUint32 PlayedLast,PlayTimeLast,PlayTimeTotal;
        wxUint32 ConnectedTimes;
        bool Search;

    private:
        CslGame *m_game;
        vector<wxInt32> m_masterIDs;

        wxInt32 m_lock;
        bool m_waiting;

        void AddMaster(wxInt32 id);
        void RemoveMaster(wxInt32 id);
};

#endif // CSLGAME_H
