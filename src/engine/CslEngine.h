/***************************************************************************
 *   Copyright (C) 2007-2014 by Glen Masgai                                *
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

#ifndef CSLENGINE_H
#define CSLENGINE_H

#include <CslUDP.h>
#include <CslGame.h>
#include <CslDNSResolver.h>
#include <CslProtocolInput.h>

class CslEngine;

#include <CslPlugin.h>

#define CSL_UPDATE_INTERVAL_MIN    5000
#define CSL_UPDATE_INTERVAL_MAX    60000
#define CSL_UPDATE_INTERVAL_WAIT   2500

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EXPORTED_EVENT_TYPE(CSL_DLL_ENGINE, wxCSL_EVT_PONG, wxID_ANY)
DECLARE_EXPORTED_EVENT_TYPE(CSL_DLL_ENGINE, wxCSL_EVT_MASTER_UPDATE, wxID_ANY)
END_DECLARE_EVENT_TYPES()

class CslPongEvent;
class CslMasterUpdateEvent;

typedef void (wxEvtHandler::*CslPongEventFunction)(CslPongEvent&);
typedef void (wxEvtHandler::*CslMasterUpdateEventFunction)(CslMasterUpdateEvent&);

#define CslPongEventHandler(fn) \
    (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(CslPongEventFunction, &fn)

#define CslMasterUpdateEventHandler(fn) \
    (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(CslMasterUpdateEventFunction, &fn)

#define CSL_EVT_PONG(id, fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
                               wxCSL_EVT_PONG, id, wxID_ANY, \
                               CslPongEventHandler(fn), \
                               (wxObject*)NULL \
                             ),

#define CSL_EVT_MASTER_UPDATE(fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
                               wxCSL_EVT_MASTER_UPDATE, wxID_ANY, wxID_ANY, \
                               CslMasterUpdateEventHandler(fn), \
                               (wxObject*)NULL \
                             ),

class CslPongEvent : public wxEvent
{
    public:
        enum { PONG, UPTIME, PLAYERSTATS, TEAMSTATS, EVENT };

        CslPongEvent(wxInt32 id = wxID_ANY, CslServerInfo *info = NULL) :
                wxEvent(id, wxCSL_EVT_PONG),
                m_info(info)
        { }

        virtual wxEvent* Clone() const
            { return new CslPongEvent(*this); }

        CslServerInfo* GetServerInfo() { return m_info; }
        void SetServerInfo(CslServerInfo *info) { m_info = info; }

    private:
        CslServerInfo *m_info;

        DECLARE_DYNAMIC_CLASS_NO_ASSIGN(CslPongEvent)
};


class CslMasterUpdateEvent : public wxEvent
{
    public:
        enum { PONG, UPTIME, PLAYERSTATS, TEAMSTATS, EVENT };

        CslMasterUpdateEvent(CslMaster *master = NULL, wxInt32 count = -1) :
                wxEvent(wxID_ANY, wxCSL_EVT_MASTER_UPDATE),
                m_master(master),
                m_count(count)
        { }

        virtual wxEvent* Clone() const
            { return new CslMasterUpdateEvent(*this); }

        CslMaster* GetMaster() { return m_master; }
        wxInt32 GetCount() const { return m_count; }

        void SetMaster(CslMaster *master) { m_master = master; }
        void SetCount(wxInt32 count) { m_count = count; }

    private:
        CslMaster *m_master;
        wxInt32 m_count;

        DECLARE_DYNAMIC_CLASS_NO_ASSIGN(CslMasterUpdateEvent)
};


class CSL_DLL_ENGINE CslEngine : public wxEvtHandler, public CslPluginMgr, public CslPluginHost
{
    public:
        CslEngine();
        ~CslEngine();

        bool Init(wxEvtHandler *handler,wxInt32 interval,wxInt32 pingsPerSecond);
        void DeInit();
        bool IsOk() const { return m_ok; }

        void SetUpdateInterval(wxInt32 interval) { m_updateInterval = interval; }

        void DNSResolve(const wxString& host, const void* clientData = NULL, wxEvtHandler *evtHandler = NULL)
            { m_dnsResolver->Resolve(host, clientData, evtHandler); }

        bool AddGame(CslGame *game);
        bool RemoveGame(CslGame *game);
        CslGame* FindGame(const wxString& name);
        CslArrayCslGame& GetGames() { return m_games; }

        bool AddServer(CslGame *game, CslServerInfo *info, wxInt32 masterID = -1);
        bool DeleteServer(CslGame *game, CslServerInfo *info);

        void GetFavourites(CslArrayCslServerInfo& servers);

        bool Ping(CslServerInfo *info,bool force=false);
        bool PingDefault(CslServerInfo *info);
        bool PingExUptime(CslServerInfo *info);
        bool PingExPlayerInfo(CslServerInfo *info,wxInt32 pid=-1,bool force=false);
        bool PingExTeamInfo(CslServerInfo *info,bool force=false);
        wxUint32 PingServers(CslGame *game,bool force=false);
        bool PingEx(CslServerInfo *info,bool force=false);
        wxUint32 PingServersEx(bool force=false);
        bool BroadcastPing(CslGame *game);

        void ResetPingSends(CslGame *game,CslMaster *master);
        void CheckResends();

        bool UpdateFromMaster(CslMaster *master);

        static bool PingOk(const CslServerInfo& info,wxInt32 interval)
            { return info.Ping>-1 && (wxInt32)(info.PingSend-info.PingResp)<interval*2; }

        // function accessable from within plugins
        wxEvtHandler* GetEvtHandler() { return this; }
        CslEngine* GetCslEngine() { return this; }

    private:
        wxInt32 EnumSystemIPV4Addresses();

        bool ParseDefaultPong(CslServerInfo *info, ucharbuf& buf);
        bool ParsePong(CslServerInfo *info, CslNetPacket& packet);

        void OnPong(CslPingEvent& event);
        void OnResolve(CslDNSResolveEvent& event);
        void OnCslProtocolInput(CslProtocolInputEvent& event);

        DECLARE_EVENT_TABLE()

        bool m_ok;
        CslUDP *m_udpSock;
        CslDNSResolver *m_dnsResolver;
        wxUint32 m_lastEnumSystemIPV4Addresses;
        CslArrayCslIPV4Addr m_systemIPV4Addresses;
        CslArrayCslProtocolInput m_masterUpdates;
        wxInt32 m_updateInterval, m_pingRatio;
        CslArrayCslGame m_games;
};


#endif // CSLENGINE_H
