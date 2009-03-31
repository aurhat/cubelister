/***************************************************************************
 *   Copyright (C) 2007-2009 by Glen Masgai                                *
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
        m_ok(false),m_pingSock(NULL),m_resolveThread(NULL)
{
}

CslEngine::~CslEngine()
{
    if (m_pingSock)
        delete m_pingSock;

    if (m_resolveThread)
    {
        m_resolveThread->Terminate();
        m_resolveThread->Wait();
        delete m_resolveThread;
    }
}

bool CslEngine::Init(wxEvtHandler *handler,wxInt32 interval,wxInt32 pingsPerSecond)
{
    if (m_ok)
        return false;

    m_updateInterval=interval;
    m_pingsPerSecond=pingsPerSecond;
    m_gameId=0;

    GetTicks();
    SetNextHandler(handler);

    if (!m_pingSock)
    {
        m_pingSock=new CslUDP(this);

        if (!(m_ok=m_pingSock->IsOk()))
        {
            delete m_pingSock;
            m_pingSock=NULL;
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

    return m_ok;
}

void CslEngine::DeInit()
{
    SetNextHandler(NULL);
    loopv(m_games) delete m_games[i];
    m_ok=false;
}

void CslEngine::ResolveHost(CslServerInfo *info)
{
    m_resolveThread->AddPacket(new CslResolverPacket(info->Host,info->Addr.Service(),
                               info->GetGame().GetId()));
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

CslGame* CslEngine::FindGame(const wxString& name)
{
    loopv(m_games)
    {
        if (m_games[i]->GetName().CmpNoCase(name)==0)
            return m_games[i];
    }

    return NULL;
}

void CslEngine::GetFavourites(vector<CslServerInfo*>& servers)
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

    {
        const vector<CslServerInfo*>& servers=game->GetServers();

        loopv(servers)
        {
            if (Ping(servers[i],force))
                c++;
        }

        if (!force && m_pingsPerSecond)
        {
            if (c>(wxUint32)(servers.length()*3000/m_updateInterval/m_pingsPerSecond+1))
                ResetPingSends(game,NULL);
        }
    }

    {
        loopv(m_games)
        {
            if (game==m_games[i])
                continue;

            const vector<CslServerInfo*>& servers=m_games[i]->GetServers();

            loopvj(servers)
            {
                if (!servers[j]->IsFavourite() && servers[j]->PingExt() && Ping(servers[j],force))
                    c++;
            }
        }
    }

    {
        vector<CslServerInfo*> servers;
        GetFavourites(servers);

        loopv(servers)
        {
            if (Ping(servers[i]))
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
        wxSocketClient sock(wxSOCKET_BLOCK);
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

    vector<CslServerInfo*>& servers=master ? master->GetServers():game->GetServers();

    loopv(servers) servers[i]->PingSend=GetTicks()-(i*250)%m_updateInterval;
}

void CslEngine::ParseDefaultPong(CslServerInfo *info,ucharbuf& buf,wxUint32 now)
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
        wxCommandEvent evt(wxCSL_EVT_PONG,CSL_PONG_TYPE_PING);
        evt.SetClientData(new CslPongPacket(info,CSL_PONG_TYPE_PING));
        wxPostEvent(handler,evt);
    }
}

void CslEngine::ParsePong(CslServerInfo *info,CslUDPPacket& packet,wxUint32 now)
{
    wxInt32 vi;
    wxUint32 vu;
    wxUint32 exVersion;
    wxInt32 cmd=-1;
    ucharbuf p((uchar*)packet.Data(),packet.Size());
    wxEvtHandler *handler=GetNextHandler();

#ifdef __WXDEBUG__
    wxString dbg_type=wxT("unknown");
#endif

    if ((vu=getint(p))==0) // extended info
    {
        cmd=getint(p);

        switch (cmd)
        {
            case CSL_EX_PING_UPTIME:
            {
#ifdef __WXDEBUG__
                dbg_type=wxT("uptime");
#endif
                vu=p.length(); // remember buffer position
                if (getint(p)!=-1)  // check ack
                {
                    info->ExtInfoStatus=CSL_EXT_STATUS_FALSE;
                    p.len=vu;
                    ParseDefaultPong(info,p,now);
                    break;
                }

                exVersion=getint(p);

                // check protocol
                if (exVersion<CSL_EX_VERSION_MIN || exVersion>CSL_EX_VERSION_MAX)
                {
                    LOG_DEBUG("%s (%s) ERROR: prot=%d\n",U2A(dbg_type),
                              U2A(info->GetBestDescription()),exVersion);
                    info->ExtInfoStatus=CSL_EXT_STATUS_ERROR;
                    PingDefault(info);
                    return;
                }

                info->Uptime=getint(p);

                if (p.overread())
                {
                    LOG_DEBUG("%s (%s) OVERREAD!\n",U2A(dbg_type),U2A(info->GetBestDescription()));
                    info->ExtInfoStatus=CSL_EXT_STATUS_ERROR;
                    PingDefault(info);
                    return;
                }

                info->ExtInfoVersion=exVersion;
                info->ExtInfoStatus=CSL_EXT_STATUS_OK;

                break;
            }

            case CSL_EX_PING_PLAYERSTATS:
            {
#ifdef __WXDEBUG__
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
                    return;
                }

                exVersion=getint(p);
                info->ExtInfoVersion=exVersion;

                // check protocol
                if (exVersion<CSL_EX_VERSION_MIN || exVersion>CSL_EX_VERSION_MAX)
                {
                    LOG_DEBUG("%s (%s) ERROR: prot=%d\n",U2A(dbg_type),
                              U2A(info->GetBestDescription()),exVersion);
                    info->ExtInfoStatus=CSL_EXT_STATUS_ERROR;
                    return;
                }

                vi=getint(p); // get error code

                if (p.overread())
                {
                    LOG_DEBUG("%s (%s) OVERREAD!\n",U2A(dbg_type),U2A(info->GetBestDescription()));
                    return;
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
                            wxCommandEvent evt(wxCSL_EVT_PONG);
                            evt.SetClientData((void*)new CslPongPacket(info,CSL_PONG_TYPE_PLAYERSTATS));
                            wxPostEvent(handler,evt);
                        }
                    }

                    break;
                }

                vi=getint(p); // get subcommand

                if (p.overread())
                {
                    LOG_DEBUG("%s (%s) OVERREAD!\n",U2A(dbg_type),U2A(info->GetBestDescription()));
                    return;
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
                            wxCommandEvent evt(wxCSL_EVT_PONG);
                            evt.SetClientData((void*)new CslPongPacket(info,CSL_PONG_TYPE_PLAYERSTATS));
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
                        LOG_DEBUG("%s (%s) OVERREAD!\n",U2A(dbg_type),
                                  U2A(info->GetBestDescription()));
                        stats.RemoveStats(data);
                        stats.Reset();
                        return;
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
                            wxCommandEvent evt(wxCSL_EVT_PONG);
                            evt.SetClientData((void*)new CslPongPacket(info,CSL_PONG_TYPE_PLAYERSTATS));
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
#ifdef __WXDEBUG__
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

                exVersion=getint(p);
                info->ExtInfoVersion=exVersion;

                // check protocol
                if (exVersion<CSL_EX_VERSION_MIN || exVersion>CSL_EX_VERSION_MAX)
                {
                    LOG_DEBUG("%s (%s) ERROR: prot=%d\n",U2A(dbg_type),
                              U2A(info->GetBestDescription()),exVersion);
                    return;
                }

                stats.TeamMode=!getint(p);  // check for teammode (send as error code)

                // wrong order in the AC extinfo 103
                // but don't include AssaultCube.h here
                if (exVersion==103 && info->GetGame().GetName()==wxT("AssaultCube"))
                {
                    stats.TimeRemain=getint(p);  // remaining time

                    if (stats.TeamMode)
                        stats.GameMode=getint(p);
                }
                else
                {
                    if (exVersion>=103)
                        stats.GameMode=getint(p);

                    stats.TimeRemain=getint(p);  // remaining time
                }

                if (p.overread())
                {
                    LOG_DEBUG("%s (%s) OVERREAD!\n",U2A(dbg_type),
                              U2A(info->GetBestDescription()));
                    stats.Reset();
                    return;
                }

                info->ExtInfoStatus=CSL_EXT_STATUS_OK;

                if (!stats.TeamMode)  // check error (no teammode)
                {
                    stats.Reset();

                    if (handler)
                    {
                        wxCommandEvent evt(wxCSL_EVT_PONG);
                        evt.SetClientData((void*)new CslPongPacket(info,CSL_PONG_TYPE_TEAMSTATS));
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
                    wxCommandEvent evt(wxCSL_EVT_PONG);
                    evt.SetClientData((void*)new CslPongPacket(info,CSL_PONG_TYPE_TEAMSTATS));
                    wxPostEvent(handler,evt);
                }

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

#ifdef __WXDEBUG__
    if (!p.overread() && p.remaining())
        LOG_DEBUG("%s: %d bytes left (type=%s)\n",U2A(info->GetBestDescription()),
                  packet.Size()-p.length(),U2A(dbg_type));
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

#ifdef __WXDEBUG__
                wxInt32 _diff=stats.m_lastPong-stats.m_lastPing;
#endif
                loopvk(stats.m_ids)
                {
                    LOG_DEBUG("%s  Diff: %d  Ping: %d  ID: %d\n",U2A(info->GetBestDescription()),
                              _diff,info->Ping,stats.m_ids[k]);
                    PingExPlayerInfo(info,stats.m_ids[k]);
                }
            }
#if 0
            else if (stats.m_status==CslPlayerStats::CSL_STATS_NEED_IDS)
            {
                if ((wxUint32)info->Ping>m_updateInterval || (stats.m_lastPong-stats.m_lastPing)<(wxUint32)info->Ping)
                    return;
                LOG_DEBUG("Resend: %s\n",U2A(info->GetBestDescription()));
                PingExPlayerInfo(info);
            }
#endif
        }
    }
}

void CslEngine::OnPong(wxCommandEvent& event)
{
    CslUDPPacket *packet;

    if (!(packet=(CslUDPPacket*)event.GetClientData()))
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

    delete packet;
}

void CslEngine::OnResolveHost(wxCommandEvent& event)
{
    CslResolverPacket *packet;

    if (!(packet=(CslResolverPacket*)event.GetClientData()))
        return;

    CslGame *game=NULL;
    CslServerInfo *info=NULL;

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
