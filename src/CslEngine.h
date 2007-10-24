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



#define CSL_EX_VERSION_MIN      101
#define CSL_EX_VERSION_MAX      102
// extended info commands
#define CSL_EX_PING_UPTIME      0
#define CSL_EX_PING_PLAYERSTATS 1
#define CSL_EX_PING_TEAMSTATS   2

enum { CSL_PONG_TYPE_PING=0, CSL_PONG_TYPE_PLAYERSTATS, CSL_PONG_TYPE_TEAMSTATS };

class CslPongPacket : public wxObject
{
    public:
        CslPongPacket(CslServerInfo* info=NULL,wxInt32 type=-1) :
                m_info(info),m_type(type) {}

        CslServerInfo *m_info;
        wxInt32 m_type;
};

class CslResolverPacket
{
    public:
        CslResolverPacket(wxIPV4address address,CSL_GAMETYPE type) :
                m_address(address),m_type(type) {}
        wxString m_domain;
        wxIPV4address m_address;
        CSL_GAMETYPE m_type;
};

class CslResolverThread : public wxThread
{
    public:
        CslResolverThread(wxEvtHandler *evtHandler) :
                wxThread(wxTHREAD_JOINABLE),m_evtHandler(evtHandler),m_terminate(false)
        {
            m_condition=new wxCondition(m_mutex);
            Create();
        }
        ~CslResolverThread() { delete m_condition; }

        virtual void *Entry();
        void AddPacket(CslResolverPacket *packet);
        void Terminate();

    private:
        wxEvtHandler *m_evtHandler;
        wxMutex m_mutex;
        wxCondition *m_condition;
        bool m_terminate;

        wxCriticalSection m_packetSection;
        vector<CslResolverPacket*> m_packets;
        wxIPV4address m_addr;
};

class CslEngine : public wxEvtHandler
{
    public:
        CslEngine(wxEvtHandler *evtHandler=NULL);
        ~CslEngine();

        bool InitEngine(wxUint32 updateInterval);
        void DeInitEngine();
        void SetUpdateInterval(wxUint32 interval) { m_updateInterval=interval; }

        CslGame* AddMaster(CslMaster *master);
        void DeleteMaster(CslMaster *master);

        CslServerInfo* AddServer(CslServerInfo *info,wxInt32 masterid);
        bool AddServerToFavourites(CslServerInfo *info);
        void RemoveServerFromFavourites(CslServerInfo *info);
        void RemoveServerFromMaster(CslServerInfo *info);
        void DeleteServer(CslServerInfo *info);

        void FuzzPingSends();

        CslGame* FindGame(CSL_GAMETYPE type);
        bool SetCurrentGame(CslGame *game,CslMaster *master);
        CslGame* GetCurrentGame() { return m_currentGame; }
        CslMaster* GetCurrentMaster() { return m_currentMaster; }
        vector<CslGame*>* GetGames() { return &m_games; }
        vector<CslServerInfo*>* GetFavourites() { return &m_favourites; }

        bool Ping(CslServerInfo *info,bool force=false);
        bool PingExUptime(CslServerInfo *info);
        bool PingExPlayerInfo(CslServerInfo *info,wxInt32 pid=-1);
        bool PingExTeamInfo(CslServerInfo *info);
        wxUint32 PingServers();

        int UpdateMaster();

    private:
        bool m_isOk;
        bool m_firstPing;
        wxEvtHandler *m_evtHandler;

        wxMutex m_mutex;

        CslResolverThread *m_resolveThread;

        wxUint32 m_updateInterval;
        CslUDP *m_pingSock;

        CslGame *m_currentGame;
        CslMaster *m_currentMaster;
        vector<CslGame*> m_games;
        vector<CslServerInfo*> m_favourites;

        void UpdateServerInfo(CslServerInfo *info,ucharbuf *buf,wxUint32 now);
        void ParsePongCmd(CslServerInfo *info,CslUDPPacket *packet,wxUint32 now);

        void OnPong(wxCommandEvent& event);
        void OnResolve(wxCommandEvent& event);

        DECLARE_EVENT_TABLE()

    protected:
        CslGame* FindOrCreateGame(CSL_GAMETYPE type);
};


#endif // CSLENGINE_H
