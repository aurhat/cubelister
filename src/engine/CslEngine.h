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

#ifndef CSLENGINE_H
#define CSLENGINE_H

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
#include "CslUDP.h"
#include "CslGame.h"

#define CSL_UPDATE_INTERVAL_MIN    5000
#define CSL_UPDATE_INTERVAL_MAX    60000
#define CSL_UPDATE_INTERVAL_WAIT   2500

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE(wxCSL_EVT_PONG,wxID_ANY)
END_DECLARE_EVENT_TYPES()

#define CSL_EVT_PONG(id,fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
                               wxCSL_EVT_PONG,id,wxID_ANY, \
                               (wxObjectEventFunction)(wxEventFunction) \
                               wxStaticCastEvent(wxCommandEventFunction,&fn), \
                               (wxObject*)NULL \
                             ),


enum { CSL_PONG_TYPE_PING=0, CSL_PONG_TYPE_PLAYERSTATS, CSL_PONG_TYPE_TEAMSTATS };

class CslPongPacket : public wxObject
{
    public:
        CslPongPacket(CslServerInfo* info=NULL,wxInt32 type=-1) :
                Info(info),Type(type) {}

        CslServerInfo *Info;
        wxInt32 Type;
};


class CslResolverPacket
{
    public:
        CslResolverPacket(const wxString& host,wxUint16 port,wxInt32 id) :
                Host(host),GameId(id)
        {
            Address.Service(port);
        }

        wxString Host;
        wxString Domain;
        wxIPV4address Address;
        wxInt32 GameId;
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
        ~CslResolverThread() { delete m_condition; }

        virtual ExitCode Entry();

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
};


class CslEngine : public wxEvtHandler
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
        wxInt32 GetNextGameID() { return ++m_gameId; }
        vector<CslGame*>& GetGames() { return m_games; }
        void GetFavourites(vector<CslServerInfo*>& servers)
        {
            loopv(m_games) m_games[i]->GetFavourites(servers);
        }

        bool Ping(CslServerInfo *info,bool force=false);
        bool PingDefault(CslServerInfo *info);
        bool PingExUptime(CslServerInfo *info);
        bool PingExPlayerInfo(CslServerInfo *info,wxInt32 pid=-1,bool force=false);
        bool PingExTeamInfo(CslServerInfo *info,bool force=false);
        wxUint32 PingServers(CslGame *game,bool force=false);
        bool PingEx(CslServerInfo *info,bool force=false);
        wxUint32 PingServersEx(bool force=false);
        void ResetPingSends(CslGame *game,CslMaster *master);
        void CheckResends();

        wxInt32 UpdateFromMaster(CslMaster *master);

        static bool PingOk(const CslServerInfo& info,wxInt32 interval)
        {
            return info.Ping>-1 && (wxInt32)(info.PingSend-info.PingResp)<interval*2;
        }

    private:
        bool m_ok;

        CslUDP *m_pingSock;
        CslResolverThread *m_resolveThread;

        wxInt32 m_gameId;
        vector<CslGame*> m_games;

        wxInt32 m_updateInterval,m_pingsPerSecond;

        void ParseDefaultPong(CslServerInfo *info,ucharbuf& buf,wxUint32 now);
        void ParsePong(CslServerInfo *info,CslUDPPacket& packet,wxUint32 now);

        void OnPong(wxCommandEvent& event);
        void OnResolveHost(wxCommandEvent& event);

        DECLARE_EVENT_TABLE()
};


#endif // CSLENGINE_H
