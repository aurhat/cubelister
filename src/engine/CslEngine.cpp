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

#include <wx/protocol/http.h>
#include <wx/sckstrm.h>
#include "CslEngine.h"
#include "CslTools.h"


BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE(wxCSL_EVT_RESOLVE_HOST,wxID_ANY)
END_DECLARE_EVENT_TYPES()

#define CSL_EVT_RESOLVE_HOST(id,fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
                               wxCSL_EVT_RESOLVE_HOST,id,wxID_ANY, \
                               (wxObjectEventFunction)(wxEventFunction) \
                               wxStaticCastEvent(wxCommandEventFunction,&fn), \
                               (wxObject*)NULL \
                             ),

DEFINE_EVENT_TYPE(wxCSL_EVT_RESOLVE_HOST)
DEFINE_EVENT_TYPE(wxCSL_EVT_PONG)

BEGIN_EVENT_TABLE(CslEngine,wxEvtHandler)
    CSL_EVT_PING(wxID_ANY,CslEngine::OnPong)
    CSL_EVT_RESOLVE_HOST(wxID_ANY,CslEngine::OnResolveHost)
END_EVENT_TABLE()

wxThread::ExitCode CslResolverThread::Entry()
{
    m_mutex.Lock();

    while (!m_terminate)
    {
        if (!m_packets.length())
        {
            LOG_DEBUG("Resolver waiting...\n");
            m_condition->Wait();
            LOG_DEBUG("Resolver signaled!\n");
            if (m_terminate)
                break;
        }

        if (!m_packets.length())
            continue;

        m_section.Enter();
        CslResolverPacket *packet=m_packets[0];
        m_packets.remove(0);
        m_section.Leave();

        packet->Address.Hostname(packet->Host);
        packet->Domain=packet->Address.Hostname();

        wxCommandEvent evt(wxCSL_EVT_RESOLVE_HOST);
        evt.SetClientData(packet);
        wxPostEvent(m_evtHandler,evt);
    }

    loopv(m_packets) delete m_packets[i];
    m_mutex.Unlock();
    LOG_DEBUG("Resolver exit.\n");

    return 0;
}

void CslResolverThread::AddPacket(CslResolverPacket *packet)
{
    m_section.Enter();
    m_packets.add(packet);
    m_section.Leave();
    if (m_mutex.TryLock()!=wxMUTEX_NO_ERROR)
        return;
    m_condition->Signal();
    m_mutex.Unlock();
}

void CslResolverThread::Terminate()
{
    m_terminate=true;
    wxMutexLocker lock(m_mutex);
    m_condition->Signal();
}


CslEngine::CslEngine(wxEvtHandler *evtHandler) : wxEvtHandler(),
        m_ok(false),
        m_evtHandler(evtHandler)
{
    SetNextHandler(evtHandler);
    GetTicks();
};

CslEngine::~CslEngine()
{
    wxASSERT(!m_ok);
}

bool CslEngine::Init(wxUint32 interval)
{
    if (m_ok)
        return false;

    m_pingSock=new CslUDP(this);
    m_ok=m_pingSock->IsInit();

    SetUpdateInterval(interval);
    m_gameId=0;

    if (m_ok)
    {
        m_resolveThread=new CslResolverThread(this);
        m_resolveThread->Run();
    }

    return m_ok;
}

void CslEngine::DeInit()
{
    if (!m_ok)
        return;

    if (m_resolveThread)
    {
        m_resolveThread->Terminate();
        m_resolveThread->Delete();
        delete m_resolveThread;
    }

    delete m_pingSock;
    loopv(m_games) delete m_games[i];

    m_ok=false;
}

void CslEngine::ResolveHost(CslServerInfo *info)
{
    m_resolveThread->AddPacket(new CslResolverPacket(info->Host,info->Addr.Service(),info->GetGame().GetId()));
}

bool CslEngine::AddGame(CslGame *game)
{
    loopv(m_games)
    {
        if (m_games[i]==game)
            return false;
    }

    m_games.add(game);
    game->SetGameID(GetNextGameID());

    return true;
}

bool CslEngine::Ping(CslServerInfo *info,bool force)
{
    if (!info->Pingable)
        return false;

    wxUint32 ticks=GetTicks();
    wxUint32 interval=m_updateInterval;

    if (info->IsWaiting())
    {
        interval=CSL_UPDATE_INTERVAL_WAIT;
        LOG_DEBUG("iswaiting\n");
    }

    //LOG_DEBUG("%s - ticks:%li, pingsend:%li, diff:%li\n",U2A(info.GetBestDescription()),
    //          ticks,info.PingSend,ticks-info.PingSend);
    if (!force && (ticks-info->PingSend)<interval)
        return false;

    info->PingSend=ticks;

    if (!PingDefault(info))
        return false;

    if (CSL_CAP_EXTINFO(info->GetGame().GetCapabilities()))
        PingExUptime(info);

    return true;
}

bool CslEngine::PingDefault(CslServerInfo *info)
{
    uchar ping[16];
    CslUDPPacket *packet=new CslUDPPacket();

    info->PingSend=GetTicks();

    ucharbuf p(ping,sizeof(ping));
    putint(p,info->PingSend);
    packet->Init(info->Addr,ping,p.length());

    return m_pingSock->SendPing(packet);
}

bool CslEngine::PingExUptime(CslServerInfo *info)
{
    uchar ping[16];
    CslUDPPacket *packet=new CslUDPPacket();

    //LOG_DEBUG("uptime %s - %d\n",U2A(info.GetBestDescription()),GetTicks());

    ucharbuf p(ping,sizeof(ping));
    putint(p,0);
    putint(p,CSL_EX_PING_UPTIME);

    packet->Init(info->Addr,ping,p.length());

    return m_pingSock->SendPing(packet);
}

bool CslEngine::PingExPlayerInfo(CslServerInfo *info,const wxInt32 pid,bool force)
{
    if (!force && info->ExtInfoStatus!=CSL_EXT_STATUS_OK)
        return false;

    //LOG_DEBUG("(%s) INFO(%d) %d\n",U2A(info->GetBestDescription()),GetTicks(),pid);

    uchar ping[16];
    CslUDPPacket *packet=new CslUDPPacket();

    if (pid==-1)
        info->PlayerStats.Reset();

    ucharbuf p(ping,sizeof(ping));
    putint(p,0);
    putint(p,CSL_EX_PING_PLAYERSTATS);
    putint(p,pid);

    packet->Init(info->Addr,ping,p.length());

    if (m_pingSock->SendPing(packet))
    {
        info->PlayerStats.m_lastPing=GetTicks();
        return true;
    }

    return false;
}

bool CslEngine::PingExTeamInfo(CslServerInfo *info,bool force)
{
    if (!force && info->ExtInfoStatus!=CSL_EXT_STATUS_OK)
        return false;

    uchar ping[16];
    CslUDPPacket *packet=new CslUDPPacket();

    info->TeamStats.Reset();

    ucharbuf p(ping,sizeof(ping));
    putint(p,0);
    putint(p,CSL_EX_PING_TEAMSTATS);

    packet->Init(info->Addr,ping,p.length());

    return m_pingSock->SendPing(packet);
}

wxUint32 CslEngine::PingServers(CslGame *game,bool force)
{
    wxUint32 c=0;
    CslServerInfo *info;

    {
        vector<CslServerInfo*>& servers=game->GetServers();

        loopv(servers)
        {
            if (Ping(servers[i],force))
                c++;
        }

        if (!force && c>10)
            ResetPingSends(game,NULL);
    }

    vector<CslServerInfo*> servers;
    GetFavourites(servers);

    loopvk(servers)
    {
        info=servers[k];
        if (info->IsFavourite() && Ping(info))
            c++;
    }
#if 0
#ifdef __WXDEBUG__
    if (c)
        LOG_DEBUG("Pinged %d servers\n",c);
#endif
#endif
    return c;
}

bool CslEngine::PingEx(CslServerInfo *info,bool force)
{
    if (!force && (GetTicks()-info->PlayerStats.m_lastPing)<m_updateInterval)
        return false;

    if (!PingExPlayerInfo(info,-1,force))
        return false;
    if (!PingExTeamInfo(info,force))
        return false;

    return true;
}

wxUint32 CslEngine::PingServersEx(bool force)
{
    wxUint32 c=0;

    loopv(m_games)
    {
        vector<CslServerInfo*> servers;
        m_games[i]->GetExtServers(servers);

        loopvj(servers)
        {
            if ((force || servers[j]->ExtInfoStatus==CSL_EXT_STATUS_OK) && PingEx(servers[j],force))
                c++;
        }
    }

#if 0
#ifdef __WXDEBUG__
    if (c)
        LOG_DEBUG("Pinged %d servers\n",c);
#endif
#endif

    return c;
}

wxInt32 CslEngine::UpdateFromMaster(CslMaster *master)
{
    wxInt32 num=0;
    char buf[32768];
    CslMasterConnection &connection=master->GetConnection();

    if (connection.GetType()==CslMasterConnection::CONNECTION_HTTP)
    {
        wxHTTP http;
        wxInputStream *stream;
        http.SetTimeout(10);

        if (!http.Connect(connection.GetAddress(),connection.GetPort()))
            return -1;
        http.SetHeader(wxT("User-Agent"),GetHttpAgent());
        if (!(stream=http.GetInputStream(connection.GetPath())))
            return -1;

        if (http.GetResponse()!=200)
            return -1;

        if (!stream->GetSize())
            return -1;

        stream->Read((void*)buf,32768);
        buf[stream->LastRead()]=0;

        delete stream;
    }
    else if (connection.GetType()==CslMasterConnection::CONNECTION_OTHER)
    {
        wxSocketClient sock(wxSOCKET_BLOCK);
        sock.SetTimeout(10);
        wxIPV4address addr;
        addr.Hostname(connection.GetAddress());
        addr.Service(connection.GetPort());

        if (!sock.Connect(addr,true))
            return -1;

        sock.Write((void*)"list",4);
        if (sock.LastCount()!=4)
            return -1;

        sock.Read((void*)buf,32768);
        buf[sock.LastCount()]=0;
    }
    else
        return -1;

    master->UnrefServers();

    CslGame *game=master->GetGame();
    char *port,*pend,*host=(char*)buf;
    bool end=false;

    while ((host=strstr(host,"addserver"))!=NULL)
    {
        port=NULL;
        host=strpbrk(host,"0123456789");
        if (!host)
            return num ? num:-1;
        pend=strpbrk(host," \t\r\n");
        if (!pend)
            end=true;
        else if (*pend=='\r' || *pend=='\n')
            *pend=0;
        else
        {
            port=strpbrk(pend,"0123456789");
            *pend=0;
            if (port)
            {
                pend=port+strspn(port,"0123456789");
                *pend=0;
            }
        }

        CslServerInfo *info=new CslServerInfo(game,A2U(host),port ? atoi(port):0);
        if (game->AddServer(info,master->GetId()))
            ResolveHost(info);
        else
            delete info;

        num++;

        if (end)
            break;

        host=pend+1;
    }

    ResetPingSends(NULL,master);

    return num;
}

void CslEngine::ResetPingSends(CslGame *game,CslMaster *master)
{
    if (!(game || master))
        return;

    vector<CslServerInfo*>& servers=master ? master->GetServers():game->GetServers();

    loopv(servers) servers[i]->PingSend=GetTicks()-(i*250)%m_updateInterval;
}

void CslEngine::ParseDefaultPong(CslServerInfo *info,ucharbuf& buf,wxUint32 now)
{
    info->PingResp=GetTicks();
    info->Ping=info->PingResp-info->PingSend;
    info->LastSeen=now;

    info->GetGame().ParseDefaultPong(buf,*info);

    wxCommandEvent evt(wxCSL_EVT_PONG,CSL_PONG_TYPE_PING);
    evt.SetClientData(new CslPongPacket(info,CSL_PONG_TYPE_PING));
    wxPostEvent(m_evtHandler,evt);
}

void CslEngine::ParsePong(CslServerInfo *info,CslUDPPacket& packet,wxUint32 now)
{
    wxInt32 vi;
    wxUint32 vu;
    wxUint32 exVersion;
    wxInt32 cmd=-1;
    ucharbuf p((uchar*)packet.Data(),packet.Size());

#ifdef __WXDEBUG__
    wxString dbg_type=wxT("unknown");
#endif

    vu=getint(p);

    while (packet.Size() > (wxUint32)p.length())
    {
        if (vu==0) // extended info
        {
            cmd=getint(p);

            switch (cmd)
            {
                case CSL_EX_PING_UPTIME:
                {
#ifdef __WXDEBUG__
                    dbg_type=wxT("uptime");
#endif
                    vu=p.length();
                    if (getint(p)!=-1)  // check ack
                    {
                        info->ExtInfoStatus=CSL_EXT_STATUS_FALSE;
                        p.len=vu;
                        ParseDefaultPong(info,p,now);
                        break;
                    }

                    exVersion=getint(p);
                    info->ExtInfoVersion=exVersion;
                    // check protocol
                    if (exVersion<CSL_EX_VERSION_MIN || exVersion>CSL_EX_VERSION_MAX)
                    {
                        LOG_DEBUG("%s (%s) ERROR: prot=%d\n",dbg_type.c_str(),
                                  U2A(info->GetBestDescription()),exVersion);
                        info->ExtInfoStatus=CSL_EXT_STATUS_ERROR;
                        PingDefault(info);
                        return;
                    }

                    info->Uptime=getint(p);

                    if (p.overread())
                    {
                        LOG_DEBUG("%s (%s) OVERREAD!\n",dbg_type.c_str(),U2A(info->GetBestDescription()));
                        info->ExtInfoStatus=CSL_EXT_STATUS_ERROR;
                        PingDefault(info);
                        return;
                    }

                    info->ExtInfoStatus=CSL_EXT_STATUS_OK;
                    break;
                }

                case CSL_EX_PING_PLAYERSTATS:
                {

                    CslPlayerStats& stats=info->PlayerStats;

                    stats.m_lastPong=GetTicks();
#ifdef __WXDEBUG__
                    dbg_type=wxT("playerstats");
#endif
                    wxInt32 rid=getint(p);  // resent id or -1

                    if (getint(p)!=-1)  // check ack
                    {
                        LOG_DEBUG("%s (%s) ERROR: missing ACK\n",dbg_type.c_str(),
                                  U2A(info->GetBestDescription()));
                        info->ExtInfoStatus=CSL_EXT_STATUS_FALSE;
                        return;
                    }

                    exVersion=getint(p);
                    info->ExtInfoVersion=exVersion;

                    // check protocol
                    if (exVersion<CSL_EX_VERSION_MIN || exVersion>CSL_EX_VERSION_MAX)
                    {
                        LOG_DEBUG("%s (%s) ERROR: prot=%d\n",dbg_type.c_str(),
                                  U2A(info->GetBestDescription()),exVersion);
                        info->ExtInfoStatus=CSL_EXT_STATUS_ERROR;
                        return;
                    }

                    info->ExtInfoStatus=CSL_EXT_STATUS_OK;

                    vi=getint(p);
                    if (vi>0)  // check error
                    {
                        LOG_DEBUG("%s (%s) INFO: player doesn't exist (%d)\n",dbg_type.c_str(),
                                  U2A(info->GetBestDescription()),rid);
                        if (rid>-1)  // check for resend error, client doesn't exist anymore
                        {
                            stats.RemoveId(rid);

                            wxCommandEvent evt(wxCSL_EVT_PONG);
                            evt.SetClientData((void*)new CslPongPacket(info,CSL_PONG_TYPE_PLAYERSTATS));
                            wxPostEvent(m_evtHandler,evt);
                            return;
                        }
                        return;
                    }

                    vi=getint(p);
                    if (vi==-10) // check for following ids
                    {
#ifdef __WXDEBUG__
                        if (rid>-1 && stats.m_ids.find(rid)<0)
                            LOG_DEBUG("%s (%s) ERROR(%d): resend id not found (%d | %d).\n",dbg_type.c_str(),
                                      U2A(info->GetBestDescription()),GetTicks(),rid,stats.m_ids.length());
#endif
                        while (!p.overread())
                        {
                            vi=getint(p);
                            if (p.overread())
                                break;
#if 0
                            LOG_DEBUG("%s (%s) INFO(%d): got id (%d).\n",dbg_type.c_str(),
                                      U2A(info->GetBestDescription()),GetTicks(),vi);
#endif
                            if (rid==-1 && !stats.AddId(vi))
                            {
                                stats.Reset();
                                LOG_DEBUG("%s (%s) ERROR: AddId(%d)\n",dbg_type.c_str(),
                                          U2A(info->GetBestDescription()),vi);
                                return;
                            }
                        }
                        stats.SetWaitForStats();

                        if (rid==-1 && stats.m_ids.length()==0)
                        {
                            wxCommandEvent evt(wxCSL_EVT_PONG);
                            evt.SetClientData((void*)new CslPongPacket(info,CSL_PONG_TYPE_PLAYERSTATS));
                            wxPostEvent(m_evtHandler,evt);
                        }

                        break;
                    }
                    else if (vi!=-11)  // check for following stats
                        return;

                    CslPlayerStatsData *data=stats.GetNewStats();

                    if (!info->GetGame().ParsePlayerPong(info->ExtInfoVersion,p,*data))
                    {
                        stats.RemoveStats(data);
                        stats.Reset();
                        info->ExtInfoStatus=CSL_EXT_STATUS_FALSE;
                        LOG_DEBUG("%s (%s) ERROR: ParsePlayerPong()\n",dbg_type.c_str(),
                                  U2A(info->GetBestDescription()));
                    }
                    else
                    {
                        if (exVersion>=103)
                        {
                            p.get((unsigned char*)&data->IP,3);
#if wxBYTE_ORDER == wxLITTLE_ENDIAN
                            data->IP=wxUINT32_SWAP_ALWAYS(data->IP);
#endif
                        }
                        else if (exVersion==102)
                        {
                            p.get((unsigned char*)&data->IP,3);
#if wxBYTE_ORDER == wxLITTLE_ENDIAN
                            data->IP<<=8;
#else
                            data->IP>>=8;
                            data->IP=wxUINT32_SWAP_ALWAYS(data->IP);
#endif
                        }

                        if (p.overread())
                        {
                            LOG_DEBUG("%s (%s) OVERREAD!\n",dbg_type.c_str(),
                                      U2A(info->GetBestDescription()));
                            return;
                        }

                        if (!stats.AddStats(data))
                        {
                            stats.RemoveStats(data);
                            stats.Reset();
                            LOG_DEBUG("%s (%s) ERROR: AddStats()\n",dbg_type.c_str(),
                                      U2A(info->GetBestDescription()));
                            break;
                        }
#if 0
                        LOG_DEBUG("%s (%s) INFO(%d): player: %s, IP:%d.%d.%d.%d (%u)\n",
                                  dbg_type.c_str(),U2A(info->GetBestDescription()),
                                  GetTicks(),U2A(data->Name),
                                  data->IP>>24,data->IP>>16&0xff,data->IP>>8&0xff,data->IP&0xff,data->IP);
#endif
                        if (stats.m_ids.length()==0)
                        {
                            wxCommandEvent evt(wxCSL_EVT_PONG);
                            evt.SetClientData((void*)new CslPongPacket(info,CSL_PONG_TYPE_PLAYERSTATS));
                            wxPostEvent(m_evtHandler,evt);
                        }
                    }
                    break;
                }

                case CSL_EX_PING_TEAMSTATS:
                {
                    CslTeamStats& stats=info->TeamStats;
#ifdef __WXDEBUG__
                    dbg_type=wxT("teamstats");
#endif
                    if (getint(p)!=-1)  // check ack
                    {
                        LOG_DEBUG("%s(%s) ERROR: missing ACK\n",dbg_type.c_str(),
                                  U2A(info->GetBestDescription()));
                        info->ExtInfoStatus=CSL_EXT_STATUS_FALSE;
                        return;
                    }

                    exVersion=getint(p);
                    info->ExtInfoVersion=exVersion;

                    // check protocol
                    if (exVersion<CSL_EX_VERSION_MIN || exVersion>CSL_EX_VERSION_MAX)
                    {
                        LOG_DEBUG("%s (%s) ERROR: prot=%d\n",dbg_type.c_str(),
                                  U2A(info->GetBestDescription()),exVersion);
                        info->ExtInfoStatus=CSL_EXT_STATUS_ERROR;
                        return;
                    }

                    info->ExtInfoStatus=CSL_EXT_STATUS_OK;

                    stats.m_teamplay=!getint(p);  // error
                    if (exVersion>=103)
                        getint(p); //FIXME dummy for gamemode
                    stats.m_remain=getint(p);  // remaining time

                    if (!stats.m_teamplay)  // check error (no teammode)
                    {
                        stats.Reset();
                        wxCommandEvent evt(wxCSL_EVT_PONG);
                        evt.SetClientData((void*)new CslPongPacket(info,CSL_PONG_TYPE_TEAMSTATS));
                        wxPostEvent(m_evtHandler,evt);
#if 0
                        LOG_DEBUG("%s (%s) ERROR: received - no teammode\n",
                                  dbg_type.c_str(),U2A(info->GetBestDescription()));
#endif
                        return;
                    }

                    while (!p.overread())
                    {
                        CslTeamStatsData *data=stats.GetNewStats();

                        if (info->GetGame().ParseTeamPong(info->ExtInfoVersion,p,*data))
                        {
                            stats.AddStats(data);

                            if (p.overread())
                            {
                                data->Reset();
                                break;
                            }
#if 0
                            LOG_DEBUG("%s (%s): team:%s, score:%d, bases:%d, remain:%d\n",
                                      dbg_type.c_str(),U2A(info->GetBestDescription()),
                                      U2A(data->m_team),data->m_score,data->m_bases.length(),stats.m_remain);
#endif
                        }
                        else
                        {
                            LOG_DEBUG("%s (%s) ERROR: ParseTeamPong()\n",dbg_type.c_str(),
                                      U2A(info->GetBestDescription()));
                            data->Reset();
                            info->ExtInfoStatus=CSL_EXT_STATUS_FALSE;
                            break;
                        }
                    }

                    if (info->ExtInfoStatus==CSL_EXT_STATUS_FALSE)
                        break;

                    wxCommandEvent evt(wxCSL_EVT_PONG);
                    evt.SetClientData((void*)new CslPongPacket(info,CSL_PONG_TYPE_TEAMSTATS));
                    wxPostEvent(m_evtHandler,evt);

                    break;
                }

                default:
                {
#ifdef __WXDEBUG__
                    dbg_type=wxT("unknown tag");
#endif
                    LOG_DEBUG("%s: ERROR: unknown tag: %d\n",U2A(info->GetBestDescription()),cmd);
                    break;
                }
            }
        }
        else
            ParseDefaultPong(info,p,now);

        break;
    }

#ifdef __WXDEBUG__
    if (p.length()<(wxInt32)packet.Size())
        LOG_DEBUG("%s: %d bytes left (type=%s)\n",U2A(info->GetBestDescription()),
                  packet.Size()-p.length(),U2A(dbg_type.c_str()));
#endif
}

void CslEngine::CheckResends()
{
    loopv(m_games)
    {
        vector<CslServerInfo*> servers;
        m_games[i]->GetExtServers(servers);

        loopvj(servers)
        {
            CslServerInfo *info=servers[j];
            CslPlayerStats& stats=info->PlayerStats;

            if (stats.m_ids.length())
            {
                if ((wxUint32)info->Ping>m_updateInterval ||
                    stats.m_lastPong<stats.m_lastPing ||
                    GetTicks()-stats.m_lastPong<min((wxUint32)info->Ping*2,500))
                    return;

                loopvk(stats.m_ids)
                {
                    LOG_DEBUG("Resend: %s Diff: %d  Ping: %d (%d)\n",info->GetBestDescription().c_str(),
                              stats.m_lastPong-stats.m_lastPing,info->Ping,stats.m_ids[k]);
                    PingExPlayerInfo(info,stats.m_ids[k]);
                }
                return;
            }
#if 0
            else if (stats.m_status==CslPlayerStats::CSL_STATS_NEED_IDS)
            {
                if ((wxUint32)info->Ping>m_updateInterval || (stats.m_lastPong-stats.m_lastPing)<(wxUint32)info->Ping)
                    return;
                LOG_DEBUG("Resend: %s\n",info->GetBestDescription().c_str());
                PingExPlayerInfo(info);
            }
#endif
        }
    }
}

wxInt32 CslEngine::GetNextGameID()
{
    return ++m_gameId;
}

void CslEngine::OnPong(wxCommandEvent& event)
{
    CslUDPPacket *packet=(CslUDPPacket*)event.GetClientData();
    if (!packet)
        return;

    if (!packet->Size())
    {
        delete packet;
        return;
    }

    CslServerInfo *info=NULL;
    wxUint32 ticks=wxDateTime::Now().GetTicks();

    loopv(m_games)
    {
        if ((info=m_games[i]->FindServerByAddr(packet->Address())))
        {
            ParsePong(info,*packet,ticks);
            break;
        }
    }

    event.SetClientData((void*)info);
    event.Skip();

    delete packet;
}

void CslEngine::OnResolveHost(wxCommandEvent& event)
{
    CslServerInfo *info=NULL;
    CslGame *game=NULL;
    CslResolverPacket *packet=(CslResolverPacket*)event.GetClientData();

    if (!packet)
        return;

    loopv(m_games)
    {
        if (m_games[i]->GetId()==packet->GameId)
        {
            game=m_games[i];
            break;
        }
    }

    if (game)
    {
        if (packet->Address.IPAddress()!=wxT("255.255.255.255") &&
            (info=game->FindServerByAddr(packet->Host,packet->Address.Service()))!=NULL)
        {
            info->Pingable=true;
            info->Addr.Hostname(packet->Address.IPAddress());
            info->Domain=packet->Domain;
        }
    }

    delete packet;
}
