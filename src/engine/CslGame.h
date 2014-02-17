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

#ifndef CSLGAME_H
#define CSLGAME_H

#include "CslExtendedInfo.h"

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

#define CSL_MM_IS_VALID(x)          ((wxInt32)x!=-1)
#define CSL_SERVER_IS_OPEN(x)       ((x&0xff)==0)
#define CSL_SERVER_IS_VETO(x)       ((x&0xff)==1)
#define CSL_SERVER_IS_LOCKED(x)     ((x&0xff)==2)
#define CSL_SERVER_IS_PRIVATE(x)    ((x&0xff)==3)
#define CSL_SERVER_IS_PASSWORD(x)   (x>-1&&x&CSL_SERVER_PASSWORD)
#define CSL_SERVER_IS_BAN(x)        (x>-1&&x&CSL_SERVER_BAN)
#define CSL_SERVER_IS_BLACKLIST(x)  (x>-1&&x&CSL_SERVER_BLACKLIST)

enum
{
    CSL_GAME_GET_PATH_BINARY = 0,
    CSL_GAME_GET_PATH_GAME,
    CSL_GAME_GET_PATH_CONFIG
};

class CslGame;
class CslMaster;
class CslServerInfo;

WX_DEFINE_USER_EXPORTED_ARRAY(CslGame*, CslArrayCslGame, class CSL_DLL_ENGINE);
WX_DEFINE_USER_EXPORTED_ARRAY(CslMaster*, CslArrayCslMaster, class CSL_DLL_ENGINE);
WX_DEFINE_USER_EXPORTED_ARRAY(CslServerInfo*, CslArrayCslServerInfo, class CSL_DLL_ENGINE);

class CSL_DLL_ENGINE CslServerEvents
{
    public:
        enum
        {
            EVENT_NONE      = 0,
            EVENT_ONLINE    = 1<<0,
            EVENT_OFFLINE   = 1<<1,
            EVENT_FULL      = 1<<2,
            EVENT_NOT_FULL  = 1<<3,
            EVENT_EMPTY     = 1<<4,
            EVENT_NOT_EMPTY = 1<<5,
            EVENT_LOCKED    = 1<<6,
            EVENT_PRIVATE   = 1<<7
        };

        CslServerEvents() : m_flags(EVENT_NONE),m_events(EVENT_NONE) {}

        void RegisterEvents(wxUint32 flags) { CSL_FLAG_SET(m_flags, flags); }
        void UnRegisterEvents(wxUint32 flags) { CSL_FLAG_UNSET(m_flags, flags); }
        bool HasRegisteredEvent(wxUint32 flag) const { return CSL_FLAG_CHECK(m_flags, flag); }
        wxUint32 GetRegisteredEvents() const { return m_flags; }

        void ClearEvents() { m_events=EVENT_NONE; }
        void SetEvents(wxUint32 flags) { CSL_FLAG_SET(m_events, flags); }
        bool HasEvents() const { return m_events!=EVENT_NONE; }
        bool HasEvent(wxUint32 flag) const { return CSL_FLAG_CHECK(m_events, flag); }

    private:
        wxUint32 m_flags, m_events;
};


class CSL_DLL_ENGINE CslMaster
{
    friend class CslGame;
    friend class CslEngine;

    public:
        CslMaster(const wxURI& uri);
        ~CslMaster() { UnrefServers(); }

        bool operator==(const CslMaster& master) const;
        bool operator!=(const CslMaster& master) const
            { return !(*this == master); }

        static CslMaster* Create(const wxURI& uri)
            { return new CslMaster(uri); }
        static void Destroy(CslMaster *master)
            { delete master; }

        static wxURI CreateURI(const wxString scheme,
                               const wxString& address,
                               wxUint16 port,
                               const wxString& path);

        bool IsOk() const { return m_ok; }
        wxInt32 GetId() const { return m_id; }
        wxURI& GetURI() { return m_uri; }
        bool IsUpdating() const { return m_updating; }
        CslGame* GetGame() const { return m_game; }
        CslArrayCslServerInfo& GetServers() { return m_servers; }

        bool UnrefServer(CslServerInfo *info, wxInt32 pos = -1);
        void UnrefServers();

    protected:
        void Init(CslGame *game, wxUint32 id);
        void AddServer(CslServerInfo *info);
        void SetIsUpdating(bool updating) { m_updating = updating; }

        bool m_ok;
        wxInt32 m_id;
        wxURI m_uri;
        bool m_updating;
        CslGame *m_game;
        CslArrayCslServerInfo m_servers;
};


class CSL_DLL_ENGINE CslGameClientSettings
{
    public:
        CslGameClientSettings() { }
        CslGameClientSettings(const wxString& binary,
                              const wxString& path,
                              const wxString& configpath,
                              const wxString& options,
                              const wxString& preScript,
                              const wxString& postScript) :
                Binary(binary), GamePath(path), ConfigPath(configpath),
                Options(options), PreScript(preScript), PostScript(postScript)
        { }

        wxString Binary, GamePath, ConfigPath, Options, PreScript, PostScript;
        wxArrayString PackageDirs;
};


class CSL_DLL_ENGINE CslGameIcon
{
    public:
        CslGameIcon(wxInt32 type,
                    wxInt32 size,
                    const void *data,
                    wxUint32 datasize) :
            Type(type), Size(size), Data(data), DataSize(datasize)
        { }

        wxInt32 Type;
        wxInt32 Size;
        const void *Data;
        wxUint32 DataSize;
};

WX_DEFINE_USER_EXPORTED_ARRAY(CslGameIcon*, CslArrayCslGameIcon, class CSL_DLL_ENGINE);

class CSL_DLL_ENGINE CslGame
{
    friend class CslEngine;

    public:
        CslGame();
        virtual ~CslGame();
        bool operator==(const CslGame& game) const { return m_fourcc==game.m_fourcc; }

        const wxString& GetName() const { return m_name; }
        wxUint32 GetFourCC() const { return m_fourcc; }
        wxUint32 GetCapabilities() const { return m_capabilities; }
        const CslGameIcon* GetIcon(wxInt32 size) const;
        const CslGameClientSettings& GetClientSettings() const { return  m_clientSettings; }

        wxInt32 AddMaster(CslMaster *master);
        bool DeleteMaster(wxInt32 masterID,wxInt32 pos=-1);
        void DeleteMasters();
        CslArrayCslMaster& GetMasters() { return m_masters; }
        bool AddServer(CslServerInfo *info,wxInt32 masterID=-1);
        bool DeleteServer(CslServerInfo *info);
        CslArrayCslServerInfo& GetServers() { return m_servers; }
        void GetFavourites(CslArrayCslServerInfo& servers);
        void GetExtServers(CslArrayCslServerInfo& servers, bool all=false);
        CslServerInfo* FindServerByHost(const wxString& host, wxUint16 port);
        CslServerInfo* FindServerByAddr(const CslIPV4Addr& addr);
        const wxURI& GetDefaultMasterURI() const { return m_defaultMasterURI; }
        static wxString& ProcessScript(const CslServerInfo& info,wxUint32 connectMode,wxString& script);

    protected:
        void AddIcon(wxInt32 type, wxInt32 size, const void *data, wxUint32 datasize);
        void FreeIcon(wxUint32 pos);

        wxString m_name;
        wxUint32 m_fourcc;
        wxUint32 m_capabilities;
        CslArrayCslGameIcon m_icons;

        wxURI m_defaultMasterURI;
        CslGameClientSettings m_clientSettings;

    public:
        enum
        {
            PLAYER_STATS_NAME,
            PLAYER_STATS_TEAM,
            PLAYER_STATS_FRAGS,
            PLAYER_STATS_DEATHS,
            PLAYER_STATS_TEAMKILLS,
            PLAYER_STATS_PING,
            PLAYER_STATS_KPD,
            PLAYER_STATS_ACCURACY,
            PLAYER_STATS_HEALTH,
            PLAYER_STATS_ARMOUR,
            PLAYER_STATS_WEAPON
        };

        virtual wxInt32 GetPlayerstatsDescriptions(const wxString **desc) const;
        virtual const wxString& GetWeaponName(wxInt32 n, wxInt32 prot) const;
        virtual wxInt32 GetPrivileges(wxInt32 n, wxInt32 prot) const { return n;}
        virtual bool ModeHasFlags(wxInt32 mode,wxInt32 prot) const { return false; }
        virtual bool ModeHasBases(wxInt32 mode,wxInt32 prot) const { return false; }
        virtual wxInt32 ModeScoreLimit(wxInt32 mode,wxInt32 prot) const { return -1; }
        virtual wxInt32 GetBestTeam(CslTeamStats& stats,wxInt32 prot) const { return -1; }
        virtual wxUint16 GetDefaultGamePort() const = 0;
        virtual wxUint16 GetInfoPort(wxUint16 gamePort=0) const = 0;
        virtual wxUint16 GetBroadcastPort() { return 0; }
        // parse master respone
        virtual wxInt32 ParseMasterResponse(CslMaster *master, char *response, size_t len);
        // server query and response buffer handling
        virtual bool PingDefault(ucharbuf& buf, CslServerInfo& info) const { return false; }
        virtual bool PingEx(ucharbuf& buf, CslServerInfo& info) const { return false; }
        virtual bool ParseDefaultPong(ucharbuf& buf,CslServerInfo& info) const = 0;
        virtual bool ParsePlayerPong(wxInt32 protocol, ucharbuf& buf, CslPlayerStatsData& info) const { return false; }
        virtual bool ParseTeamPong(wxInt32 protocol, ucharbuf& buf, CslTeamStatsData& info) const { return false; }
        // client settings
        virtual CslGameClientSettings GuessClientSettings(const wxString& path) const  = 0;
        virtual wxString ValidateClientSettings(CslGameClientSettings& settings) const = 0;
        virtual void SetClientSettings(const CslGameClientSettings& settings) { m_clientSettings=settings; }
        // launch it
        virtual wxString GameStart(CslServerInfo *info, wxInt32 mode, wxString& error) = 0;
        virtual wxInt32 GameEnd(wxString& error) = 0;
        virtual bool GetMapImagePaths(wxArrayString& paths) const { return false; }
        // hooks for workarounds or special handling
        virtual void ProcessOutput(char *data) const { }
        virtual bool ReturnOk(wxInt32 code) const { return code==0; }

    private:
        CslArrayCslMaster m_masters;
        CslArrayCslServerInfo m_servers;
};

class CSL_DLL_ENGINE CslServerInfo : public CslExtendedInfo, public CslServerEvents
{
    friend class CslGame;
    friend class CslMaster;
    friend class CslEngine;

    #define CSL_SERVERINFO_CTOR_ARGS_H \
                CslGame *game = NULL, \
                const wxString& host = wxT("127.0.0.1"), \
                wxUint16 gamePort = 0, \
                wxUint16 infoPort = 0, \
                wxUint32 view = CSL_VIEW_DEFAULT, \
                wxUint32 lastSeen = 0, \
                wxUint32 playedLast = 0, \
                wxUint32 playTimeLast = 0, \
                wxUint32 playTimeTotal = 0, \
                wxUint32 connectedTimes = 0, \
                const wxString& oldDescription = wxEmptyString, \
                const wxString& pass = wxEmptyString, \
                const wxString& passAdm = wxEmptyString

    public:
        CslServerInfo(CSL_SERVERINFO_CTOR_ARGS_H);
        ~CslServerInfo() { }

        enum { CSL_VIEW_DEFAULT = 1<<0, CSL_VIEW_FAVOURITE = 1<<1 };
        enum { CSL_CONNECT_DEFAULT, CSL_CONNECT_PASS, CSL_CONNECT_ADMIN_PASS };

        bool operator==(const CslServerInfo& info) const
        {
            return (Host==info.Host && GamePort==info.GamePort && m_addr.GetPort()==info.m_addr.GetPort());
        }

        void Init(CSL_SERVERINFO_CTOR_ARGS_H);

        static CslServerInfo* Create(CSL_SERVERINFO_CTOR_ARGS_H);
        static void Destroy(CslServerInfo *info) { delete info; }

        CslGame& GetGame() const { return *m_game; }
        const wxArrayInt& GetMasterIDs() const { return m_masterIDs; }

        void SetLastPlayTime(wxUint32 time);

        void SetDescription(const wxString& description);
        wxString GetBestDescription() const;

        void Lock(bool lock = true);
        bool IsLocked() const { return m_lock>0; }
        bool IsFull() const { return Players>0 && PlayersMax>0 && Players>=PlayersMax; }

        bool Pingable() const { return m_addr.GetMaskBits()==32 && m_addr.GetPort()!=0; }

        const CslIPV4Addr& Address() const { return m_addr; }

        void SetDefault() { CSL_FLAG_SET(View, CSL_VIEW_DEFAULT); }
        void SetFavourite() { CSL_FLAG_SET(View, CSL_VIEW_FAVOURITE); }
        void RemoveDefault() { CSL_FLAG_UNSET(View, CSL_VIEW_DEFAULT); }
        void RemoveFavourite() { CSL_FLAG_UNSET(View, CSL_VIEW_FAVOURITE); }

        bool IsUnused() const { return View==0; }
        bool IsDefault() const { return CSL_FLAG_CHECK(View, CSL_VIEW_DEFAULT); }
        bool IsFavourite() const { return CSL_FLAG_CHECK(View, CSL_VIEW_FAVOURITE); }

        bool HasStats() const;

        wxString Host, Domain;
        wxString InfoText;
        wxUint16 GamePort;
        wxString Password, PasswordAdmin;
        wxString Description, DescriptionOld;
        wxString GameMode, Map;
        wxString Version;
        wxString MMDescription;
        wxInt32 MM;
        wxInt32 Protocol;
        wxInt32 Ping, TimeRemain;
        wxInt32 Players, PlayersMax;
        wxUint32 View;
        wxUint32 LastSeen, PingSend, PingResp;
        wxUint32 PlayedLast, PlayTimeLast, PlayTimeTotal;
        wxUint32 ConnectedTimes;
        wxInt32 ConnectWait;
        bool Search;

    protected:
        CslGame *m_game;
        wxArrayInt m_masterIDs;

        CslIPV4Addr m_addr;

        wxInt32 m_lock;

        void AddMaster(wxInt32 id);
        void RemoveMaster(wxInt32 id);

    #undef CSL_SERVERINFO_INIT_PARAMS
};

#endif // CSLGAME_H
