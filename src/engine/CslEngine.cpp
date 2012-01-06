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

#include "Csl.h"
#include "CslEngine.h"

DEFINE_EVENT_TYPE(wxCSL_EVT_PONG)
DEFINE_EVENT_TYPE(wxCSL_EVT_RESOLVE)

BEGIN_EVENT_TABLE(CslEngine,wxEvtHandler)
    CSL_EVT_PING(CslEngine::OnPong)
    CSL_EVT_RESOLVE(CslEngine::OnResolve)
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
        }

        if (m_terminate || !m_packets.length())
            continue;

        m_section.Enter();
        CslResolverPacket *packet=m_packets[0];
        m_packets.remove(0);
        m_section.Leave();

        packet->Address.Hostname(packet->Host);
        packet->Domain=packet->Address.Hostname();

        if (m_terminate)
            break;

        CslResolveEvent evt(packet);
        wxPostEvent(m_evtHandler,evt);
    }

    loopvrev(m_packets) delete m_packets[i];
    m_packets.setsizenodelete(0);

    m_mutex.Unlock();
    LOG_DEBUG("Resolver exit.\n");

    return 0;
}

void CslResolverThread::AddPacket(CslResolverPacket *packet)
{
    m_section.Enter();
    if (IsIP(packet->Host))
        m_packets.add(packet);
    else
        m_packets.insert(0,packet);
    m_section.Leave();

    if (m_mutex.TryLock()==wxMUTEX_NO_ERROR)
    {
        m_condition->Signal();
        m_mutex.Unlock();
    }
}

void CslResolverThread::Terminate()
{
    m_terminate=true;
    wxMutexLocker lock(m_mutex);
    m_condition->Signal();
}


CslEngine::CslEngine() : wxEvtHandler(),
        CslPluginMgr(CslPlugin::TYPE_ENGINE),
        m_ok(false), m_udpSock(NULL), m_resolveThread(NULL)
{
}

CslEngine::~CslEngine()
{
    if (m_udpSock)
        delete m_udpSock;

    if (m_resolveThread)
    {
        m_resolveThread->Terminate();
        m_resolveThread->Wait();
        delete m_resolveThread;
    }
}

bool CslEngine::Init(wxEvtHandler *handler,wxInt32 interval,wxInt32 pingRatio)
{
    if (m_ok)
        return false;

    m_updateInterval=interval;
    m_pingRatio=pingRatio;

    GetTicks();
    SetNextHandler(handler);

    if (!m_udpSock)
    {
        m_udpSock=new CslUDP(this);

        if (!(m_ok=m_udpSock->IsOk()))
        {
            delete m_udpSock;
            m_udpSock=NULL;
        }
    }

    if (m_ok && !m_resolveThread)
    {
        m_resolveThread=new CslResolverThread(this);

        if (!m_resolveThread->IsOk() || m_resolveThread->Run()!=wxTHREAD_NO_ERROR)
        {
            delete m_resolveThread;
            m_resolveThread=NULL;
            m_ok=false;
        }
    }

    if (m_ok)
        LoadPlugins(this);

    return m_ok;
}

void CslEngine::DeInit()
{
    SetNextHandler(NULL);

    loopvrev(m_games) delete m_games[i];
    m_games.setsizenodelete(0);

    loopvrev(m_networkInterfaces) delete m_networkInterfaces[i];
    m_networkInterfaces.setsizenodelete(0);

    m_ok=false;
}

void CslEngine::ResolveHost(CslServerInfo *info)
{
    m_resolveThread->AddPacket(new CslResolverPacket(info->Host,info->Addr.Service(),
                               info->GetGame().GetFourCC()));
}

bool CslEngine::AddGame(CslGame *game)
{
    loopv(m_games)
    {
        if (m_games[i]==game)
            return false;
    }

    m_games.add(game);

    return true;
}

CslGame* CslEngine::FindGame(const wxString& name)
{
    loopv(m_games)
    {
        if (m_games[i]->GetName().CmpNoCase(name)==0)
            return m_games[i];
    }

    return NULL;
}

void CslEngine::GetFavourites(CslServerInfos& servers)
{
    loopv(m_games)
    {
        m_games[i]->GetFavourites(servers);
    }
}

bool CslEngine::Ping(CslServerInfo *info,bool force)
{
    if (!info->Pingable)
        return false;

    wxUint32 interval,ticks=GetTicks();

    if (info->ConnectWait>0)
    {
        interval=CSL_UPDATE_INTERVAL_WAIT;
        LOG_DEBUG("is waiting\n");
    }
    else
        interval=m_updateInterval;

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
    CslNetPacket *packet=new CslNetPacket();

    info->PingSend=GetTicks();

    ucharbuf p(ping,sizeof(ping));

    if (!info->GetGame().PingDefault(p,*info))
    putint(p,info->PingSend);

    packet->Init(info->Addr,ping,p.length());

    return m_udpSock->Send(packet);
}

bool CslEngine::PingExUptime(CslServerInfo *info)
{
    uchar ping[16];
    CslNetPacket *packet=new CslNetPacket();

    //LOG_DEBUG("uptime %s - %d\n",U2A(info.GetBestDescription()),GetTicks());

    ucharbuf p(ping,sizeof(ping));
    putint(p,0);
    putint(p,CSL_EX_PING_UPTIME);

    packet->Init(info->Addr,ping,p.length());

    return m_udpSock->Send(packet);
}

bool CslEngine::PingExPlayerInfo(CslServerInfo *info, wxInt32 pid, bool force)
{
    if (!force && info->ExtInfoStatus!=CSL_EXT_STATUS_OK)
        return false;

    //LOG_DEBUG("(%s) INFO(%d) %d\n",U2A(info->GetBestDescription()),GetTicks(),pid);

    uchar ping[16];
    CslNetPacket *packet=new CslNetPacket();

    if (pid==-1)
        info->PlayerStats.Reset();

    ucharbuf p(ping,sizeof(ping));
    putint(p,0);
    putint(p,CSL_EX_PING_PLAYERSTATS);
    putint(p,pid);

    packet->Init(info->Addr,ping,p.length());

    if (m_udpSock->Send(packet))
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
    CslNetPacket *packet=new CslNetPacket();

    info->TeamStats.Reset();

    ucharbuf p(ping,sizeof(ping));
    putint(p,0);
    putint(p,CSL_EX_PING_TEAMSTATS);

    packet->Init(info->Addr,ping,p.length());

    return m_udpSock->Send(packet);
}

wxUint32 CslEngine::PingServers(CslGame *game,bool force)
{
    wxUint32 c=0;

    {
        const CslServerInfos& servers=game->GetServers();

        loopv(servers)
        {
            if (Ping(servers[i],force))
                c++;
        }

        if (!force && m_pingRatio)
        {
            if (c>(wxUint32)(servers.length()*3000/m_updateInterval/m_pingRatio+1))
                ResetPingSends(game,NULL);
        }
    }

    {
        loopv(m_games)
        {
            if (game==m_games[i])
                continue;

            const CslServerInfos& servers=m_games[i]->GetServers();

            loopvj(servers)
            {
                if (!servers[j]->IsFavourite() && servers[j]->PingExt() && Ping(servers[j],force))
                    c++;
            }
        }
    }

    {
        CslServerInfos servers;
        GetFavourites(servers);

        loopv(servers)
        {
            if (Ping(servers[i]))
                c++;
        }
    }
#if 0
#ifdef __DEBUG__
    if (c)
        LOG_DEBUG("Pinged %d servers\n",c);
#endif
#endif
    return c;
}

bool CslEngine::PingEx(CslServerInfo *info,bool force)
{
    wxUint32 diff=GetTicks()-info->PlayerStats.m_lastPing;
    wxUint32 delay=info->Search ? (min(info->Ping,m_updateInterval)/1000+1)*1000:m_updateInterval;

    if (!force && diff<delay)
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
        CslServerInfos servers;
        m_games[i]->GetExtServers(servers);

        loopvj(servers)
        {
            if ((force || servers[j]->ExtInfoStatus==CSL_EXT_STATUS_OK) && PingEx(servers[j],force))
                c++;
        }
    }

#if 0
#ifdef __DEBUG__
    if (c)
        LOG_DEBUG("Pinged %d servers\n",c);
#endif
#endif

    return c;
}

bool CslEngine::BroadcastPing(CslGame *game)
{
    wxUint16 port;

    if (!(port=game->GetBroadcastPort()))
        return false;

    CslServerInfo info(game, wxT("255.255.255.255"), port, port);

    return PingDefault(&info);
}

wxInt32 CslEngine::UpdateFromMaster(CslMaster *master)
{
    wxInt32 num=0;
    char buf[32768];
    const CslMasterConnection& connection=master->GetConnection();

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
        wxSocketClient sock(wxSOCKET_BLOCK|wxSOCKET_WAITALL);
        sock.SetTimeout(10);
        wxIPV4address addr;
        addr.Hostname(connection.GetAddress());
        addr.Service(connection.GetPort());

        if (!sock.Connect(addr,true))
            return -1;

        sock.Write((void*)"list\n",5);
        if (sock.LastCount()!=5)
            return -1;

        sock.Read((void*)buf,32768);
        buf[sock.LastCount()]=0;
    }
    else
        return -1;

    master->UnrefServers();

    CslGame *game=master->GetGame();
    char *p,*port,*iport,*host=(char*)buf;
    bool parse=true;

    while (parse && (host=strstr(host,"addserver")))
    {
        host+=9;
        port=iport=NULL;

        if (!(host=strpbrk(host,"0123456789")))
            return num ? num:-1;
        if (!(p=strpbrk(host," \t\r\n")))
            parse=false;
        else if (*p=='\r' || *p=='\n')
            *p=0;
        else
        {
            if ((port=strpbrk(p,"0123456789")))
            {
                *p=0;
                p=port+strspn(port,"0123456789");

                if ((iport=strpbrk(p," \t\r\n")) && (*p==' ' || *p=='\t') && *p!='\r' && *p!='\n')
                {
                    *p=0;

                    if ((iport=strpbrk(iport+1,"0123456789")))
                        p=iport+strspn(iport,"0123456789");
                }
                else
                    iport=NULL;
            }
            *p=0;
        }

        CslServerInfo *info=new CslServerInfo(game,A2U(host),port ? atoi(port):0,iport ? atoi(iport):0);

        if (game->AddServer(info,master->GetId()))
            ResolveHost(info);
        else
            delete info;

        num++;

        if (parse)
            host=p+1;
    }

    ResetPingSends(NULL,master);

    return num;
}

void CslEngine::ResetPingSends(CslGame *game,CslMaster *master)
{
    if (!(game || master))
        return;

    CslServerInfos& servers=master ? master->GetServers():game->GetServers();

    loopv(servers) servers[i]->PingSend=GetTicks()-(i*250)%m_updateInterval;
}

bool CslEngine::ParseDefaultPong(CslServerInfo *info, ucharbuf& buf, wxUint32 now)
{
    info->PingResp=GetTicks();
    info->Ping=info->PingResp-info->PingSend;
    info->LastSeen=now;
    info->ClearEvents();

    if (!info->GetGame().ParseDefaultPong(buf,*info))
        info->Ping=-1;

    wxEvtHandler *handler;

    if ((handler=GetNextHandler()))
    {
        CslPongEvent evt(CslPongEvent::PONG, info);
        wxPostEvent(handler, evt);

        if (info->HasEvents())
        {
            CslPongEvent evt(CslPongEvent::EVENT, info);
        wxPostEvent(handler,evt);
    }
}

    return info->Ping!=-1;
}

bool CslEngine::ParsePong(CslServerInfo *info, CslNetPacket& packet, wxUint32 now)
{
    wxInt32 vi;
    wxUint32 vu;
    bool ret=-1;
    ucharbuf p((uchar*)packet.Data(),packet.Size());
    wxEvtHandler *handler=GetNextHandler();

#ifdef __DEBUG__
    wxString dbg_type=wxT("unknown");
#endif

    if (!getint(p)) // extended info
    {
        wxInt32 cmd=getint(p);

        switch (cmd)
        {
            case CSL_EX_PING_UPTIME:
            {
#ifdef __DEBUG__
                dbg_type=wxT("uptime");
#endif
                vu=p.length(); // remember buffer position
                if (getint(p)!=-1)  // check ack
                {
                    info->ExtInfoStatus=CSL_EXT_STATUS_FALSE;
                    p.len=vu;
                    ret=ParseDefaultPong(info, p, now);
                    break;
                }

                info->ExtInfoVersion=getint(p);

                // check protocol
                if (info->ExtInfoVersion<CSL_EX_VERSION_MIN || info->ExtInfoVersion>CSL_EX_VERSION_MAX)
                {
                    LOG_DEBUG("%s (%s) ERROR: prot=%d\n",U2A(dbg_type),
                              U2A(info->GetBestDescription()), info->ExtInfoVersion);
                    info->ExtInfoStatus=CSL_EXT_STATUS_ERROR;
                    PingDefault(info);
                    return false;
                }

                info->Uptime=getint(p);

                if (p.overread())
                {
                    LOG_DEBUG("%s (%s) OVERREAD!\n",U2A(dbg_type),U2A(info->GetBestDescription()));
                    info->ExtInfoStatus=CSL_EXT_STATUS_ERROR;
                    PingDefault(info);
                    return false;
                }

                info->ExtInfoStatus=CSL_EXT_STATUS_OK;

                if (handler)
                {
                    CslPongEvent evt(CslPongEvent::UPTIME, info);
                    wxPostEvent(handler, evt);
                }

                break;
            }

            case CSL_EX_PING_PLAYERSTATS:
            {
#ifdef __DEBUG__
                dbg_type=wxT("playerstats");
#endif
                CslPlayerStats& stats=info->PlayerStats;
                stats.m_lastPong=GetTicks();

                info->ExtInfoStatus=CSL_EXT_STATUS_ERROR;

                wxInt32 rid=getint(p);  // resend id or -1

                if (getint(p)!=-1)  // check ack
                {
                    LOG_DEBUG("%s (%s) ERROR: missing ACK\n",U2A(dbg_type),
                              U2A(info->GetBestDescription()));
                    info->ExtInfoStatus=CSL_EXT_STATUS_FALSE;
                    return false;
                }

                info->ExtInfoVersion=getint(p);

                // check protocol
                if (info->ExtInfoVersion<CSL_EX_VERSION_MIN || info->ExtInfoVersion>CSL_EX_VERSION_MAX)
                {
                    LOG_DEBUG("%s (%s) ERROR: prot=%d\n",U2A(dbg_type),
                              U2A(info->GetBestDescription()), info->ExtInfoVersion);
                    info->ExtInfoStatus=CSL_EXT_STATUS_ERROR;
                    return false;
                }

                vi=getint(p); // get error code

                if (p.overread())
                {
                    LOG_DEBUG("%s (%s) OVERREAD!\n",U2A(dbg_type),U2A(info->GetBestDescription()));
                    return false;
                }

                if (vi>0)  // check error
                {
                    LOG_DEBUG("%s (%s) INFO: player doesn't exist (%d)\n",U2A(dbg_type),
                              U2A(info->GetBestDescription()),rid);

                    info->ExtInfoStatus=CSL_EXT_STATUS_OK;

                    if (rid>-1)  // check for resend error, client doesn't exist anymore
                    {
                        stats.RemoveId(rid);

                        if (handler && stats.m_ids.length()==0)
                        {
                            CslPongEvent evt(CslPongEvent::PLAYERSTATS, info);
                            wxPostEvent(handler,evt);
                        }
                    }

                    break;
                }

                vi=getint(p); // get subcommand

                if (p.overread())
                {
                    LOG_DEBUG("%s (%s) OVERREAD!\n",U2A(dbg_type),U2A(info->GetBestDescription()));
                    return false;
                }

                if (vi==-10) // check for following ID's
                {
                    if (rid>-1 && stats.m_ids.find(rid)<0)
                    {
                        LOG_DEBUG("%s (%s) ERROR(%d): resend id not found (%d | %d).\n",U2A(dbg_type),
                                  U2A(info->GetBestDescription()),GetTicks(),rid,stats.m_ids.length());
                        break;
                    }

                    while (p.remaining())
                    {
                        vi=getint(p);
#if 0
                        LOG_DEBUG("%s (%s) INFO(%d): got id (%d).\n",U2A(dbg_type),
                                  U2A(info->GetBestDescription()),GetTicks(),vi);
#endif
                        if (rid==-1 && !stats.AddId(vi))
                        {
                            stats.Reset();
                            LOG_DEBUG("%s (%s) ERROR: AddId(%d)\n",U2A(dbg_type),
                                      U2A(info->GetBestDescription()),vi);
                            break;
                        }
                    }
                    stats.SetWaitForStats();
                    //LOG_DEBUG("wait for stats %s: %d\n",U2A(info->GetBestDescription()),stats.m_status);

                    info->ExtInfoStatus=CSL_EXT_STATUS_OK;

                    if (rid==-1 && stats.m_ids.length()==0)
                    {
                        if (handler)
                        {
                            CslPongEvent evt(CslPongEvent::PLAYERSTATS, info);
                            wxPostEvent(handler,evt);
                        }
                    }

                    break;
                }
                else if (vi!=-11)  // check for following stats
                    break;

                CslPlayerStatsData *data=stats.GetNewStats();

                if (info->GetGame().ParsePlayerPong(info->ExtInfoVersion,p,*data))
                {
                    data->IP=(wxUint32)-1;

                    if (info->ExtInfoVersion>=103)
                        p.get((unsigned char*)&data->IP, 3);
                    else if (info->ExtInfoVersion==102)
                    {
                        p.get((unsigned char*)&data->IP,3);
#if wxBYTE_ORDER == wxLITTLE_ENDIAN
                        data->IP=wxUINT32_SWAP_ALWAYS(data->IP<<8);
#else
                        data->IP>>=8;
#endif
                    }

                    if (p.overread())
                    {
                        LOG_DEBUG("%s (%s) OVERREAD!\n",U2A(dbg_type),
                                  U2A(info->GetBestDescription()));
                        stats.RemoveStats(data);
                        stats.Reset();
                        return false;
                    }

                    info->ExtInfoStatus=CSL_EXT_STATUS_OK;

                    if (!stats.AddStats(data))
                    {
                        LOG_DEBUG("%s (%s) ERROR: AddStats()\n",U2A(dbg_type),
                                  U2A(info->GetBestDescription()));
                        stats.RemoveStats(data);
                        stats.Reset();
                        break;
                    }
#if 0
                    LOG_DEBUG("%s (%s) INFO(%d): player: %s, IP:%d.%d.%d.%d (%u)\n",
                              U2A(dbg_type),U2A(info->GetBestDescription()),
                              GetTicks(),U2A(data->Name),
                              data->IP>>24,data->IP>>16&0xff,data->IP>>8&0xff,data->IP&0xff,data->IP);
#endif
                    if (stats.m_ids.length()==0)
                    {
                        if (handler)
                        {
                            CslPongEvent evt(CslPongEvent::PLAYERSTATS, info);
                            wxPostEvent(handler,evt);
                        }
                    }
                }
                else
                {
                    stats.RemoveStats(data);
                    stats.Reset();
                    info->ExtInfoStatus=CSL_EXT_STATUS_FALSE;
                    LOG_DEBUG("%s (%s) ERROR: ParsePlayerPong()\n",U2A(dbg_type),
                              U2A(info->GetBestDescription()));
                }
                break;
            }

            case CSL_EX_PING_TEAMSTATS:
            {
#ifdef __DEBUG__
                dbg_type=wxT("teamstats");
#endif
                CslTeamStats& stats=info->TeamStats;

                info->ExtInfoStatus=CSL_EXT_STATUS_ERROR;

                if (getint(p)!=-1)  // check ack
                {
                    LOG_DEBUG("%s(%s) ERROR: missing ACK\n",U2A(dbg_type),
                              U2A(info->GetBestDescription()));
                    info->ExtInfoStatus=CSL_EXT_STATUS_FALSE;
                    break;
                }

                info->ExtInfoVersion=getint(p);

                // check protocol
                if (info->ExtInfoVersion<CSL_EX_VERSION_MIN || info->ExtInfoVersion>CSL_EX_VERSION_MAX)
                {
                    LOG_DEBUG("%s (%s) ERROR: prot=%d\n",U2A(dbg_type),
                              U2A(info->GetBestDescription()), info->ExtInfoVersion);
                    return false;
                }

                stats.TeamMode=!getint(p);  // check for teammode (send as error code)

                // wrong order in the AC extinfo 103
                // but don't include AssaultCube.h here
                if (info->ExtInfoVersion==103 && info->GetGame().GetFourCC()==CSL_BUILD_FOURCC("ATCE"))
                {
                    stats.TimeRemain=getint(p);  // remaining time

                    if (stats.TeamMode)
                        stats.GameMode=getint(p);
                }
                else
                {
                    if (info->ExtInfoVersion>=103)
                        stats.GameMode=getint(p);

                    stats.TimeRemain=max(0, getint(p));  // remaining time

                    if (info->ExtInfoVersion<=104)
                        stats.TimeRemain*=60;
                }

                if (p.overread())
                {
                    LOG_DEBUG("%s (%s) OVERREAD!\n",U2A(dbg_type),
                              U2A(info->GetBestDescription()));
                    stats.Reset();
                    return false;
                }

                info->ExtInfoStatus=CSL_EXT_STATUS_OK;

                if (!stats.TeamMode)  // check error (no teammode)
                {
                    stats.Reset();

                    if (handler)
                    {
                        CslPongEvent evt(CslPongEvent::TEAMSTATS, info);
                        wxPostEvent(handler,evt);
                    }
#if 0
                    LOG_DEBUG("%s (%s) ERROR: received - no teammode\n",
                              U2A(dbg_type),U2A(info->GetBestDescription()));
#endif
                    break;
                }

                while (p.remaining())
                {
                    CslTeamStatsData *data=stats.GetNewStats();

                    if (info->GetGame().ParseTeamPong(info->ExtInfoVersion,p,*data))
                    {
                        stats.AddStats(data);
#if 0
                        LOG_DEBUG("%s (%s): team:%s, score:%d, bases:%d, remain:%d\n",
                                  U2A(dbg_type),U2A(info->GetBestDescription()),
                                  U2A(data->Name),data->Score,data->Bases.length(),stats.TimeRemain);
#endif
                    }
                    else
                    {
                        LOG_DEBUG("%s (%s) ERROR: ParseTeamPong()\n",U2A(dbg_type),
                                  U2A(info->GetBestDescription()));
                        data->Reset();
                        stats.RemoveStats(data);
                        info->ExtInfoStatus=CSL_EXT_STATUS_FALSE;
                        break;
                    }
                }

                if (info->ExtInfoStatus==CSL_EXT_STATUS_FALSE)
                    break;

                if (handler)
                {
                    CslPongEvent evt(CslPongEvent::TEAMSTATS, info);
                    wxPostEvent(handler,evt);
                }

                break;
            }

            default:
            {
#ifdef __DEBUG__
                dbg_type=wxT("unknown tag");
#endif
                LOG_DEBUG("%s: ERROR: unknown tag: %d\n",U2A(info->GetBestDescription()),cmd);
                break;
            }
        }
    }
    else
    {
        p.len=0;
        ret=ParseDefaultPong(info, p, now);
    }

#ifdef __DEBUG__
    if (!p.overread() && p.remaining())
        LOG_DEBUG("%s: %d bytes left (type=%s)\n",U2A(info->GetBestDescription()),
                  packet.Size()-p.length(),U2A(dbg_type));
#endif

    return ret>-1 ? ret!=0 : !p.overread();
}

void CslEngine::CheckResends()
{
    loopv(m_games)
    {
        CslServerInfos servers;
        m_games[i]->GetExtServers(servers);

        loopvj(servers)
        {
            CslServerInfo *info=servers[j];
            CslPlayerStats& stats=info->PlayerStats;

            if (!PingOk(*info,m_updateInterval))
            {
                stats.Reset();
                info->ExtInfoStatus=CSL_EXT_STATUS_FALSE;
                continue;
            }

            if (stats.m_ids.length())
            {
                if (info->Ping>m_updateInterval ||
                    stats.m_lastPong<stats.m_lastPing ||
                    GetTicks()-stats.m_lastPong<(wxUint32)min(info->Ping*2,500))
                    continue;

#ifdef __DEBUG__
                wxInt32 _diff=stats.m_lastPong-stats.m_lastPing;
#endif
                loopvk(stats.m_ids)
                {
                    LOG_DEBUG("%s  Diff: %d  Ping: %d  ID: %d\n",
                              U2A(info->GetBestDescription()),
                              _diff,info->Ping,stats.m_ids[k]);
                    PingExPlayerInfo(info,stats.m_ids[k]);
                }
            }
#if 0
            else if (stats.m_status==CslPlayerStats::CSL_STATS_NEED_IDS)
            {
                if ((wxUint32)info->Ping>m_updateInterval ||
                        (stats.m_lastPong-stats.m_lastPing)<(wxUint32)info->Ping)
                    return;
                LOG_DEBUG("Resend: %s\n",U2A(info->GetBestDescription()));
                PingExPlayerInfo(info);
            }
#endif
        }
    }
}

wxInt32 CslEngine::EnumNetworkInterfaces()
{
    loopv(m_networkInterfaces) delete m_networkInterfaces[i];
    m_networkInterfaces.setsizenodelete(0);

    GetSystemIPAddresses(m_networkInterfaces);

    return m_networkInterfaces.length();
}

void CslEngine::OnPong(CslPingEvent& event)
{
    CslNetPacket *packet;
    CslServerInfo *info=NULL;
    wxUint32 ticks=wxDateTime::Now().GetTicks();

    if (!(packet=event.GetPacket()))
        goto cleanup;

    if (!packet->Size())
        goto cleanup;

    loopv(m_games)
    {
        if ((info=m_games[i]->FindServerByAddr(packet->Address())))
        {
            ParsePong(info,*packet,ticks);
            goto cleanup;
        }
    }

    // check for broadcast response
    if (!info)
    {
        EnumNetworkInterfaces();

        if (!IsLocalIP(packet->Address().IPAddress(), &m_networkInterfaces))
            goto cleanup;

        info=new CslServerInfo;
        wxUint16 port=packet->Address().Service();

        loopv(m_games)
        {
            info->Create(m_games[i], packet->Address().IPAddress(), port-1, port);

            if (ParsePong(info,*packet, ticks))
            {
                m_games[i]->AddServer(info);
                goto cleanup;
            }
        }

        delete info;
    }

cleanup:
    delete packet;
}

void CslEngine::OnResolve(CslResolveEvent& event)
{
    CslResolverPacket *packet;

    if (!(packet=event.GetPacket()))
        return;

    CslGame *game;
    CslServerInfo *info;
    wxEvtHandler *handler;

    loopv(m_games)
    {
        if ((game=m_games[i])->GetFourCC()==packet->GameFourCC)
        {
            if ((info=game->FindServerByAddr(packet->Host, packet->Address.Service())))
    {
                if (packet->Address.IPAddress()!=wxT("0.0.0.0") &&
                    packet->Address.IPAddress()!=wxT("255.255.255.255"))
        {
            info->Pingable=true;
            info->Addr.Hostname(packet->Address.IPAddress());
            info->Domain=packet->Domain;
        }

                if ((handler=GetNextHandler()))
                {
                    CslResolveEvent evt(info);
                    wxPostEvent(handler, evt);
                }
                break;
            }
        }
    }

    delete packet;
}
