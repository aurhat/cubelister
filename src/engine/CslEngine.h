/***************************************************************************
 *   Copyright (C) 2007-2011 by Glen Masgai                                *
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

/**
 @author Glen Masgai <mimosius@users.sourceforge.net>
*/

#include <CslUDP.h>
#include <CslGame.h>

class CslEngine;

#include <CslPlugin.h>

#define CSL_UPDATE_INTERVAL_MIN    5000
#define CSL_UPDATE_INTERVAL_MAX    60000
#define CSL_UPDATE_INTERVAL_WAIT   2500

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE(wxCSL_EVT_PONG,wxID_ANY)
END_DECLARE_EVENT_TYPES()

class CslPongEvent;

typedef void (wxEvtHandler::*CslPongEventFunction)(CslPongEvent&);

#define CslPongEventHandler(fn) \
    (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(CslPongEventFunction, &fn)

#define CSL_EVT_PONG(id,fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
                               wxCSL_EVT_PONG,id,wxID_ANY, \
                               CslPongEventHandler(fn), \
                               (wxObject*)NULL \
                             ),

class CslPongEvent : public wxEvent
{
    public:
        enum { PONG=0, UPTIME, PLAYERSTATS, TEAMSTATS, EVENT };

        CslPongEvent(wxInt32 id=wxID_ANY, CslServerInfo *info=NULL) :
                wxEvent(id, wxCSL_EVT_PONG),
                m_info(info) { }

        virtual wxEvent* Clone() const
        {
            return new CslPongEvent(*this);
        }

        CslServerInfo* GetServerInfo() { return m_info; }

    private:
        CslServerInfo *m_info;
};


BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE(wxCSL_EVT_RESOLVE, wxID_ANY)
END_DECLARE_EVENT_TYPES()

class CslResolveEvent;
class CslResolverPacket;

typedef void (wxEvtHandler::*CslResolveEventFunction)(CslResolveEvent&);

#define CslResolveEventHandler(fn) \
    (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(CslResolveEventFunction, &fn)

#define CSL_EVT_RESOLVE(fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
                               wxCSL_EVT_RESOLVE, wxID_ANY, wxID_ANY, \
                               CslResolveEventHandler(fn), \
                               (wxObject*)NULL \
                             ),

class CslResolveEvent : public wxEvent
{
    public:
        CslResolveEvent(CslServerInfo *info) :
                wxEvent(wxID_ANY, wxCSL_EVT_RESOLVE),
                m_info(info), m_packet(NULL) { }
        CslResolveEvent(CslResolverPacket *packet) :
                wxEvent(wxID_ANY, wxCSL_EVT_RESOLVE),
                m_info(NULL), m_packet(packet) { }

        virtual wxEvent* Clone() const
        {
            return new CslResolveEvent(*this);
        }

        CslServerInfo* GetServerInfo() { return m_info; }
        CslResolverPacket* GetPacket() { return m_packet; }

    private:
        CslServerInfo *m_info;
        CslResolverPacket *m_packet;
};

class CslResolverPacket
{
    public:
        CslResolverPacket(const wxString& host, wxUint16 port, wxUint32 fourcc) :
                Host(host), GameFourCC(fourcc)
        {
            Address.Service(port);
        }

        wxString Host;
        wxString Domain;
        wxIPV4address Address;
        wxUint32 GameFourCC;
};

class CslResolverThread : public wxThread
{
    public:
        CslResolverThread(wxEvtHandler *evtHandler) :
                wxThread(wxTHREAD_JOINABLE),m_evtHandler(evtHandler),m_terminate(false)
        {
            m_condition=new wxCondition(m_mutex);
            m_ok=Create()==wxTHREAD_NO_ERROR;
        }
        virtual ~CslResolverThread() { delete m_condition; }

        bool IsOk() { return m_ok; }
        void AddPacket(CslResolverPacket *packet);
        void Terminate();

    private:
        wxEvtHandler *m_evtHandler;
        wxMutex m_mutex;
        wxCondition *m_condition;
        bool m_terminate;
        bool m_ok;

        wxCriticalSection m_section;
        vector<CslResolverPacket*> m_packets;
        wxIPV4address m_addr;

    protected:
        virtual wxThread::ExitCode Entry();
};


class CslEngine : public wxEvtHandler, public CslPluginMgr, public CslPluginHostInfo
{
    public:
        CslEngine();
        ~CslEngine();

        bool Init(wxEvtHandler *handler,wxInt32 interval,wxInt32 pingsPerSecond);
        void DeInit();
        bool IsOk() const { return m_ok; }

        void SetUpdateInterval(wxInt32 interval) { m_updateInterval=interval; }

        void ResolveHost(CslServerInfo *info);

        bool AddGame(CslGame *game);
        CslGames& GetGames() { return m_games; }
        CslGame* FindGame(const wxString& name);

        void GetFavourites(CslServerInfos& servers);

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

        wxInt32 UpdateFromMaster(CslMaster *master);

        static bool PingOk(const CslServerInfo& info,wxInt32 interval)
        {
            return info.Ping>-1 && (wxInt32)(info.PingSend-info.PingResp)<interval*2;
        }

        // function accessable from within plugins
        wxEvtHandler* GetEvtHandler() { return this; }
        CslEngine* GetCslEngine() { return this; }

    private:
        bool m_ok;

        CslUDP *m_udpSock;
        vector<CslIPV4Addr*> m_networkInterfaces;

        CslResolverThread *m_resolveThread;

        CslGames m_games;

        wxInt32 m_updateInterval,m_pingRatio;

        bool ParseDefaultPong(CslServerInfo *info, ucharbuf& buf, wxUint32 now);
        bool ParsePong(CslServerInfo *info, CslNetPacket& packet, wxUint32 now);

        wxInt32 EnumNetworkInterfaces();

        void OnPong(CslPingEvent& event);
        void OnResolve(CslResolveEvent& event);

        DECLARE_EVENT_TABLE()
};


#endif // CSLENGINE_H
