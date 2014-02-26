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

#include "Csl.h"
#include "CslEngine.h"

DEFINE_EVENT_TYPE(wxCSL_EVT_PONG)
DEFINE_EVENT_TYPE(wxCSL_EVT_MASTER_UPDATE)

BEGIN_EVENT_TABLE(CslEngine, wxEvtHandler)
CSL_EVT_PING(CslEngine::OnPong)
CSL_EVT_DNS_RESOLVE(CslEngine::OnResolve)
CSL_EVT_PROTO_INPUT(wxID_ANY, CslEngine::OnCslProtocolInput)
END_EVENT_TABLE()


IMPLEMENT_DYNAMIC_CLASS(CslPongEvent, wxEvent)
IMPLEMENT_DYNAMIC_CLASS(CslMasterUpdateEvent, wxEvent)


CslEngine::CslEngine() : wxEvtHandler(),
        CslPluginMgr(CSL_PLUGIN_TYPE_ENGINE),
        m_ok(false), m_udpSock(NULL), m_dnsResolver(NULL),
        m_lastEnumSystemIPV4Addresses(0)
{
}

CslEngine::~CslEngine()
{
    if (m_udpSock)
        delete m_udpSock;

    if (m_dnsResolver)
    {
        m_dnsResolver->Terminate();
        m_dnsResolver->Wait();
        delete m_dnsResolver;
    }
}

bool CslEngine::Init(wxEvtHandler *handler,wxInt32 interval,wxInt32 pingRatio)
{
    if (m_ok)
        return false;

    GetTicks();

    m_udpSock = new CslUDP(this);

    if (!m_udpSock || !(m_udpSock->IsOk()))
    {
        delete m_udpSock;
        m_udpSock = NULL;
        goto error;
    }

    m_dnsResolver = new CslDNSResolver(this);

    if (!m_dnsResolver->IsOk() || m_dnsResolver->Run()!=wxTHREAD_NO_ERROR)
    {
        delete m_dnsResolver;
        m_dnsResolver = NULL;
        goto error;
    }

    m_pingRatio = pingRatio;
    m_updateInterval = interval;

    SetNextHandler(handler);

    LoadPlugins(this);

    m_ok = true;

    return true;

error:
    m_ok = true;
    DeInit();

    return false;
}

void CslEngine::DeInit()
{
    if (!m_ok)
        return;

    SetNextHandler(NULL);

    UnloadPlugins(this);

    m_games.Empty();

    FreeSystemIPV4Addresses(m_systemIPV4Addresses);

    m_ok = false;
}

bool CslEngine::AddGame(CslGame *game)
{
    loopv(m_games)
    {
        if (m_games[i]==game)
            return false;
    }

    m_games.push_back(game);

    return true;
}

bool CslEngine::RemoveGame(CslGame *game)
{
    wxInt32 pos = m_games.Index(game);

    if (pos>=0)
    {
        m_games.RemoveAt(pos);
        return true;
    }

    return false;
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

bool CslEngine::AddServer(CslGame *game, CslServerInfo *info, wxInt32 masterID)
{
    loopv(m_games)
    {
        if (m_games[i]->GetFourCC()==info->GetGame().GetFourCC())
        {
            if (!m_games[i]->AddServer(info, masterID))
                return false;

            if (!IsIPV4(info->Host))
                DNSResolve(info->Host, info, this);

            return true;
        }
    }

    return false;
}

bool CslEngine::DeleteServer(CslGame *game, CslServerInfo *info)
{
    return game->DeleteServer(info);
}

void CslEngine::GetFavourites(CslArrayCslServerInfo& servers)
{
    loopv(m_games) m_games[i]->GetFavourites(servers);
}

bool CslEngine::Ping(CslServerInfo *info, bool force)
{
    wxUint32 interval, ticks = GetTicks();

    if (info->ConnectWait>0)
    {
        interval = CSL_UPDATE_INTERVAL_WAIT;
        CSL_LOG_DEBUG("is waiting\n");
    }
    else
        interval = m_updateInterval;

    //CSL_LOG_DEBUG("%s - ticks:%li, pingsend:%li, diff:%li\n",U2C(info.GetBestDescription()),
    //          ticks,info.PingSend,ticks-info.PingSend);
    if (!force && (ticks-info->PingSend)<interval)
        return false;

    if (!PingDefault(info))
        return false;

    if (CSL_CAP_EXTINFO(info->GetGame().GetCapabilities()))
        PingExUptime(info);

    return true;
}

bool CslEngine::PingDefault(CslServerInfo *info)
{
    if (!info->Pingable())
        return false;

    uchar ping[16];

    info->PingSend = GetTicks();

    ucharbuf p(ping, sizeof(ping));

    if (!info->GetGame().PingDefault(p, *info))
        putint(p, info->PingSend);

    CslNetPacket packet(p.length(), ping, &info->Address());

    return m_udpSock->Send(packet);
}

bool CslEngine::PingExUptime(CslServerInfo *info)
{
     if (!info->Pingable())
        return false;

    uchar ping[16];

    //CSL_LOG_DEBUG("uptime %s - %d\n",U2C(info.GetBestDescription()),GetTicks());
    ucharbuf p(ping, sizeof(ping));

    if (!info->GetGame().PingEx(p, *info))
        putint(p, 0);

    putint(p, CSL_EX_PING_UPTIME);

    CslNetPacket packet(p.length(), ping, &info->Address());

    return m_udpSock->Send(packet);
}

bool CslEngine::PingExPlayerInfo(CslServerInfo *info, wxInt32 pid, bool force)
{
    if (!info->Pingable())
        return false;

    if (!force && info->ExtInfoStatus!=CSL_EXT_STATUS_OK)
        return false;

    //CSL_LOG_DEBUG("(%s) INFO(%d) %d\n",U2C(info->GetBestDescription()),GetTicks(),pid);

    uchar ping[16];

    if (pid==-1)
        info->PlayerStats.Reset();

    ucharbuf p(ping, sizeof(ping));

    if (!info->GetGame().PingEx(p, *info))
        putint(p, 0);

    putint(p, CSL_EX_PING_PLAYERSTATS);
    putint(p, pid);

    CslNetPacket packet(p.length(), ping, &info->Address());

    if (m_udpSock->Send(packet))
    {
        info->PlayerStats.m_lastPing = GetTicks();
        return true;
    }

    return false;
}

bool CslEngine::PingExTeamInfo(CslServerInfo *info, bool force)
{
    if (!info->Pingable())
        return false;

    if (!force && info->ExtInfoStatus!=CSL_EXT_STATUS_OK)
        return false;

    uchar ping[16];

    info->TeamStats.Reset();

    ucharbuf p(ping, sizeof(ping));

    if (!info->GetGame().PingEx(p, *info))
        putint(p, 0);

    putint(p, CSL_EX_PING_TEAMSTATS);

    CslNetPacket packet(p.length(), ping, &info->Address());

    return m_udpSock->Send(packet);
}

wxUint32 CslEngine::PingServers(CslGame *game, bool force)
{
    wxUint32 c=0;

    {
        const CslArrayCslServerInfo& servers=game->GetServers();

        loopv(servers)
        {
            if (Ping(servers[i],force))
                c++;
        }

        if (!force && m_pingRatio)
        {
            if (c>(wxUint32)(servers.size()*3000/m_updateInterval/m_pingRatio+1))
                ResetPingSends(game,NULL);
        }
    }

    {
        loopv(m_games)
        {
            if (game==m_games[i])
                continue;

            const CslArrayCslServerInfo& servers=m_games[i]->GetServers();

            loopvj(servers)
            {
                CslServerInfo *server=servers[j];

                if (!server->IsFavourite() && server->PingExt() && Ping(server, force))
                    c++;
            }
        }
    }

    {
        CslArrayCslServerInfo servers;
        servers.Alloc(100);

        GetFavourites(servers);

        loopv(servers)
        {
            if (Ping(servers[i]))
                c++;
        }
    }
#if 0
    if (c)
        CSL_LOG_DEBUG("Pinged %d servers\n",c);
#endif
    return c;
}

bool CslEngine::PingEx(CslServerInfo *info, bool force)
{
    wxUint32 diff = GetTicks()-info->PlayerStats.m_lastPing;
    wxUint32 delay = info->Search ? (min(info->Ping, m_updateInterval)/1000+1)*1000 : m_updateInterval;

    if (!force && diff<delay)
        return false;

    if (!PingExPlayerInfo(info, -1, force))
        return false;
    if (!PingExTeamInfo(info, force))
        return false;

    return true;
}

wxUint32 CslEngine::PingServersEx(bool force)
{
    wxUint32 c=0;

    loopv(m_games)
    {
        CslArrayCslServerInfo servers;
        m_games[i]->GetExtServers(servers);

        loopvj(servers)
        {
            CslServerInfo *info = servers[j];

            if ((force || info->ExtInfoStatus==CSL_EXT_STATUS_OK) && PingEx(info, force))
                c++;
        }
    }

#if 0
#ifdef CSL_DEBUG
    if (c)
        CSL_LOG_DEBUG("Pinged %d servers\n",c);
#endif
#endif

    return c;
}

wxInt32 CslEngine::BroadcastPing(CslGame *game, bool limited)
{
    wxUint16 port;
    wxInt32 count = 0;

    if (!(port=game->GetBroadcastPort()))
        return -1;

    if (limited)
    {
        CSL_LOG_DEBUG("bcast: 255.255.255.255\n");

        CslServerInfo info(game, wxT("255.255.255.255"), port, port);

        if (PingDefault(&info))
            count++;
    }
    else
    {
        EnumSystemIPV4Addresses();

        loopv(m_systemIPV4Addresses)
        {
            CSL_LOG_DEBUG("bcast: %s\n", U2C(m_systemIPV4Addresses[i]->Format(wxT("%b"))));

            CslServerInfo info(game, m_systemIPV4Addresses[i]->Format(wxT("%b")), port, port);

            if (PingDefault(&info))
                count++;
        }
    }

    return count;
}

bool CslEngine::UpdateFromMaster(CslMaster *master)
{
    if (master->IsUpdating())
        return false;

    const wxURI& uri = master->GetURI();

    loopv(m_masterUpdates) if (m_masterUpdates[i]->GetInput()==uri)
        return false;

    CslProtocolInput *proto = new CslProtocolInput(this, wxID_ANY, uri);

    proto->SetMaxRead(65536);
    proto->SetChunkSize(65536);
    proto->SetClientData(master);

    if (proto->Run()!=wxTHREAD_NO_ERROR)
    {
        delete proto;
        return false;
    }

    master->SetIsUpdating(true);
    m_masterUpdates.push_back(proto);

    return true;
}

void CslEngine::ResetPingSends(CslGame *game,CslMaster *master)
{
    if (!(game || master))
        return;

    CslArrayCslServerInfo& servers=master ? master->GetServers():game->GetServers();

    loopv(servers) servers[i]->PingSend=GetTicks()-(i*250)%m_updateInterval;
}

wxInt32 CslEngine::EnumSystemIPV4Addresses()
{
    wxUint32 now = GetTicks();

    if (!m_lastEnumSystemIPV4Addresses || now-m_lastEnumSystemIPV4Addresses>=1000*60)
    {
        m_lastEnumSystemIPV4Addresses = now;

        FreeSystemIPV4Addresses(m_systemIPV4Addresses);
        GetSystemIPV4Addresses(m_systemIPV4Addresses);
    }

    return m_systemIPV4Addresses.size();
}

bool CslEngine::ParseDefaultPong(CslServerInfo *info, ucharbuf& buf)
{
    info->PingResp = GetTicks();
    info->Ping = info->PingResp-info->PingSend;
    info->ClearEvents();

    if (!info->GetGame().ParseDefaultPong(buf, *info))
        info->Ping = -1;

    info->LastSeen = wxDateTime::Now().GetTicks();

    CslPongEvent evt(CslPongEvent::PONG, info);
    AddPendingEvent(evt);

    if (info->HasEvents())
    {
        CslPongEvent evt(CslPongEvent::EVENT, info);
        AddPendingEvent(evt);
    }

    return info->Ping!=-1;
}

bool CslEngine::ParsePong(CslServerInfo *info, CslNetPacket& packet)
{
    wxInt32 vi;
    wxUint32 vu;
    bool ret=true;
    ucharbuf p((uchar*)packet.Data(),packet.Size());

#ifdef CSL_DEBUG
    wxString msg_type=wxT("unknown");
#endif

    if (!getint(p)) // extended info
    {
        wxInt32 cmd=getint(p);

        switch (cmd)
        {
            case CSL_EX_PING_UPTIME:
            {
#ifdef CSL_DEBUG
                msg_type=wxT("uptime");
#endif
                vu=p.length(); // remember buffer position
                if (getint(p)!=-1)  // check ack
                {
                    info->ExtInfoStatus=CSL_EXT_STATUS_FALSE;
                    p.len=vu;
                    ret=ParseDefaultPong(info, p);
                    break;
                }

                info->ExtInfoVersion=getint(p);

                // check protocol
                if (info->ExtInfoVersion<CSL_EX_VERSION_MIN || info->ExtInfoVersion>CSL_EX_VERSION_MAX)
                {
                    CSL_LOG_DEBUG("%s (%s) ERROR: prot=%d\n",U2C(msg_type),
                                  U2C(info->GetBestDescription()), info->ExtInfoVersion);
                    info->ExtInfoStatus=CSL_EXT_STATUS_ERROR;
                    PingDefault(info);
                    return false;
                }

                info->Uptime=getint(p);

                if (p.overread())
                {
                    CSL_LOG_DEBUG("%s (%s) OVERREAD!\n",U2C(msg_type),U2C(info->GetBestDescription()));
                    info->ExtInfoStatus=CSL_EXT_STATUS_ERROR;
                    PingDefault(info);
                    return false;
                }

                info->ExtInfoStatus=CSL_EXT_STATUS_OK;

                CslPongEvent evt(CslPongEvent::UPTIME, info);
                AddPendingEvent(evt);
                break;
            }

            case CSL_EX_PING_PLAYERSTATS:
            {
#ifdef CSL_DEBUG
                msg_type=wxT("playerstats");
#endif
                CslPlayerStats& stats=info->PlayerStats;
                stats.m_lastPong=GetTicks();

                info->ExtInfoStatus=CSL_EXT_STATUS_ERROR;

                wxInt32 rid=getint(p);  // resend id or -1

                if (getint(p)!=-1)  // check ack
                {
                    CSL_LOG_DEBUG("%s (%s) ERROR: missing ACK\n",U2C(msg_type),
                                  U2C(info->GetBestDescription()));
                    info->ExtInfoStatus=CSL_EXT_STATUS_FALSE;
                    return false;
                }

                info->ExtInfoVersion=getint(p);

                // check protocol
                if (info->ExtInfoVersion<CSL_EX_VERSION_MIN || info->ExtInfoVersion>CSL_EX_VERSION_MAX)
                {
                    CSL_LOG_DEBUG("%s (%s:%d) ERROR: prot=%d\n",U2C(msg_type),
                                  U2C(info->GetBestDescription()), info->Address().GetPort(), info->ExtInfoVersion);
                    return false;
                }

                vi=getint(p); // get error code

                if (p.overread())
                {
                    CSL_LOG_DEBUG("%s (%s) OVERREAD!\n",U2C(msg_type),U2C(info->GetBestDescription()));
                    return false;
                }

                if (vi>0)  // check error
                {
                    CSL_LOG_DEBUG("%s (%s) INFO: player doesn't exist (%d)\n",U2C(msg_type),
                                  U2C(info->GetBestDescription()),rid);

                    info->ExtInfoStatus=CSL_EXT_STATUS_OK;

                    if (rid>-1)  // check for resend error, client doesn't exist anymore
                    {
                        stats.RemoveId(rid);

                        if (stats.m_ids.size()==0)
                        {
                            CslPongEvent evt(CslPongEvent::PLAYERSTATS, info);
                            AddPendingEvent(evt);
                        }
                    }

                    break;
                }

                vi=getint(p); // get subcommand

                if (p.overread())
                {
                    CSL_LOG_DEBUG("%s (%s) OVERREAD!\n",U2C(msg_type),U2C(info->GetBestDescription()));
                    return false;
                }

                if (vi==-10) // check for following ID's
                {
                    if (rid>-1 && stats.m_ids.Index(rid)==wxNOT_FOUND)
                    {
                        CSL_LOG_DEBUG("%s (%s) ERROR: resend id not found (%d | %lu).\n", U2C(msg_type),
                                      U2C(info->GetBestDescription()), rid, stats.m_ids.GetCount());
                        break;
                    }

                    while (p.remaining())
                    {
                        vi=getint(p);
#if 0
                        CSL_LOG_DEBUG("%s (%s) INFO(%d): got id (%d).\n", U2C(msg_type),
                                      U2C(info->GetBestDescription()), GetTicks(), vi);
#endif
                        if (rid==-1 && !stats.AddId(vi))
                        {
                            stats.Reset();
                            CSL_LOG_DEBUG("%s (%s) ERROR: AddId(%d)\n", U2C(msg_type),
                                          U2C(info->GetBestDescription()), vi);
                            break;
                        }
                    }
                    stats.SetWaitForStats();
                    //CSL_LOG_DEBUG("wait for stats %s: %d\n",U2C(info->GetBestDescription()),stats.m_status);

                    info->ExtInfoStatus=CSL_EXT_STATUS_OK;

                    if (rid==-1 && stats.m_ids.size()==0)
                    {
                        CslPongEvent evt(CslPongEvent::PLAYERSTATS, info);
                        AddPendingEvent(evt);
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
                        CSL_LOG_DEBUG("%s (%s) OVERREAD!\n",U2C(msg_type),
                                      U2C(info->GetBestDescription()));
                        stats.RemoveStats(data);
                        stats.Reset();
                        return false;
                    }

                    info->ExtInfoStatus=CSL_EXT_STATUS_OK;

                    if (!stats.AddStats(data))
                    {
                        CSL_LOG_DEBUG("%s (%s) ERROR: AddStats()\n",U2C(msg_type),
                                      U2C(info->GetBestDescription()));
                        stats.RemoveStats(data);
                        stats.Reset();
                        break;
                    }
#if 0
                    CSL_LOG_DEBUG("%s (%s) INFO(%d): player: %s, IP:%d.%d.%d.%d (%u)\n",
                                  U2C(msg_type),U2C(info->GetBestDescription()),
                                  GetTicks(),U2C(data->Name),
                                  data->IP>>24,data->IP>>16&0xff,data->IP>>8&0xff,data->IP&0xff,data->IP);
#endif
                    if (stats.m_ids.size()==0)
                    {
                        CslPongEvent evt(CslPongEvent::PLAYERSTATS, info);
                        AddPendingEvent(evt);
                    }
                }
                else
                {
                    stats.RemoveStats(data);
                    stats.Reset();
                    info->ExtInfoStatus=CSL_EXT_STATUS_FALSE;
                    CSL_LOG_DEBUG("%s (%s) ERROR: ParsePlayerPong()\n",U2C(msg_type),
                                  U2C(info->GetBestDescription()));
                }
                break;
            }

            case CSL_EX_PING_TEAMSTATS:
            {
#ifdef CSL_DEBUG
                msg_type=wxT("teamstats");
#endif
                CslTeamStats& stats=info->TeamStats;

                info->ExtInfoStatus=CSL_EXT_STATUS_ERROR;

                if (getint(p)!=-1)  // check ack
                {
                    CSL_LOG_DEBUG("%s(%s) ERROR: missing ACK\n",U2C(msg_type),
                                  U2C(info->GetBestDescription()));
                    info->ExtInfoStatus=CSL_EXT_STATUS_FALSE;
                    break;
                }

                info->ExtInfoVersion=getint(p);

                // check protocol
                if (info->ExtInfoVersion<CSL_EX_VERSION_MIN || info->ExtInfoVersion>CSL_EX_VERSION_MAX)
                {
                    CSL_LOG_DEBUG("%s (%s) ERROR: prot=%d\n",U2C(msg_type),
                                  U2C(info->GetBestDescription()), info->ExtInfoVersion);
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
                    CSL_LOG_DEBUG("%s (%s) OVERREAD!\n",U2C(msg_type),
                                  U2C(info->GetBestDescription()));
                    stats.Reset();
                    return false;
                }

                info->ExtInfoStatus=CSL_EXT_STATUS_OK;

                if (!stats.TeamMode)  // check error (no teammode)
                {
                    stats.Reset();

                    CslPongEvent evt(CslPongEvent::TEAMSTATS, info);
                    AddPendingEvent(evt);
#if 0
                    CSL_LOG_DEBUG("%s (%s) ERROR: received - no teammode\n",
                                  U2C(msg_type),U2C(info->GetBestDescription()));
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
                        CSL_LOG_DEBUG("%s (%s): team:%s, score:%d, bases:%d, remain:%d\n",
                                      U2C(msg_type),U2C(info->GetBestDescription()),
                                      U2C(data->Name),data->Score,data->Bases.length(),stats.TimeRemain);
#endif
                    }
                    else
                    {
                        CSL_LOG_DEBUG("%s (%s) ERROR: ParseTeamPong()\n",U2C(msg_type),
                                      U2C(info->GetBestDescription()));
                        data->Reset();
                        stats.RemoveStats(data);
                        info->ExtInfoStatus=CSL_EXT_STATUS_FALSE;
                        break;
                    }
                }

                if (info->ExtInfoStatus==CSL_EXT_STATUS_FALSE)
                    break;

                CslPongEvent evt(CslPongEvent::TEAMSTATS, info);
                AddPendingEvent(evt);
                break;
            }

            default:
            {
#ifdef CSL_DEBUG
                msg_type=wxT("unknown tag");
#endif
                CSL_LOG_DEBUG("%s: ERROR: unknown tag: %d\n",U2C(info->GetBestDescription()),cmd);
                break;
            }
        }
    }
    else
    {
        p.len=0;
        ret=ParseDefaultPong(info, p);
    }

#ifdef CSL_DEBUG
    if (!p.overread() && p.remaining())
        CSL_LOG_DEBUG("%s: %d bytes left (type=%s)\n",U2C(info->GetBestDescription()),
                      packet.Size()-p.length(),U2C(msg_type));
#endif

    return ret && !p.overread();
}

void CslEngine::CheckResends()
{
    loopv(m_games)
    {
        CslArrayCslServerInfo servers;
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

            if (stats.m_ids.size())
            {
                if (info->Ping>m_updateInterval ||
                    stats.m_lastPong<stats.m_lastPing ||
                    GetTicks()-stats.m_lastPong<(wxUint32)min(info->Ping*2,500))
                    continue;

                CSL_DEF_DEBUG(wxInt32 _diff=stats.m_lastPong-stats.m_lastPing);

                loopvk(stats.m_ids)
                {
                    CSL_LOG_DEBUG("%s  Diff: %d  Ping: %d  ID: %d\n",
                                  U2C(info->GetBestDescription()),
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

                CSL_LOG_DEBUG("Resend: %s\n",U2C(info->GetBestDescription()));
                PingExPlayerInfo(info);
            }
#endif
        }
    }
}

void CslEngine::OnPong(CslPingEvent& event)
{
    CslNetPacket *packet = event.GetPacket();

    if (!m_ok)
    {
        CslNetPacket::Destroy(packet, true);
        return;
    }

    CslServerInfo *info = NULL;
    const CslIPV4Addr& addr = packet->Address();

    if (!addr.IsOk())
        goto cleanup;

    loopv(m_games)
    {
        if ((info = m_games[i]->FindServerByAddr(addr)))
        {
            if (!ParsePong(info, *packet))
                goto cleanup;
            break;
        }
    }

    // check for broadcast response
    if (!info)
    {
        EnumSystemIPV4Addresses();

        if (!IsLocalIPV4(addr, &m_systemIPV4Addresses))
        {
            CSL_LOG_DEBUG("non local packet from %s\n", U2C(addr.Format(wxT("%i:%p"))));
            goto cleanup;
        }

        wxString ip = addr.GetIPString();
        wxUint16 port = addr.GetPort();

        info = new CslServerInfo;

        loopv(m_games)
        {
            info->Init(m_games[i], ip, port-1, port);

            if (ParsePong(info, *packet))
            {
                info->SetFavourite();
                m_games[i]->AddServer(info);
                goto cleanup;
            }
        }

        delete info;
    }

cleanup:
    CslNetPacket::Destroy(packet, true);
}

void CslEngine::OnResolve(CslDNSResolveEvent& event)
{
    wxEvtHandler *handler = event.GetEvtHandler();

    if (!m_ok || (handler && handler!=this))
        return;

    const wxString& ip = event.GetIP();

    if (ip!=wxT("0.0.0.0") && ip!=wxT("255.255.255.255")) loopv(m_games)
    {
        CslGame *game;
        CslServerInfo *info = (CslServerInfo*)event.GetClientData();

        if ((game = m_games[i])->GetFourCC()==info->GetGame().GetFourCC())
        {
            CslArrayCslServerInfo& servers = game->GetServers();

            if (servers.Index(info)!=wxNOT_FOUND)
            {
                info->m_addr.Create(ip, info->m_addr.GetPort());
                info->Domain = event.GetDomain();
            }
            break;
        }
    }
}

void CslEngine::OnCslProtocolInput(CslProtocolInputEvent& event)
{
    if (event.IsTerminate())
        return;

    CslProtocolInput *proto = NULL;
    CslMaster *master = (CslMaster*)event.GetClientData();

    const wxURI& uri = master->GetURI();

    loopv(m_masterUpdates) if (m_masterUpdates[i]->GetInput()==uri)
    {
        proto = m_masterUpdates[i];
        break;
    }

    if (!proto)
    {
        wxASSERT(proto!=NULL);
        return;
    }

    CslMasterUpdateEvent evt(master);

    if (!event.IsError())
    {
        wxMemoryBuffer& buf = event.GetBuffer();
        char *response = (char*)buf.GetData();
        size_t size = buf.GetDataLen();

        if (buf.GetDataLen())
        {
            CslGame *game = master->GetGame();
            wxInt32 count = game->ParseMasterResponse(master, response, size);

            if (count)
                ResetPingSends(NULL, master);

            evt.SetCount(count);
        }
    }

    ProcessEvent(evt);

    master->SetIsUpdating(false);

    loopv(m_masterUpdates) if (m_masterUpdates[i]==proto)
    {
        m_masterUpdates.erase(m_masterUpdates.begin()+i);
        break;
    }

    delete proto;
}
