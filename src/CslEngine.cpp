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
#include <wx/platinfo.h>
#include "CslEngine.h"
#include "CslTools.h"
#include "CslVersion.h"


BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE(wxCSL_EVT_RESOLVE,wxID_ANY)
END_DECLARE_EVENT_TYPES()

#define CSL_EVT_RESOLVE(id,fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
                               wxCSL_EVT_RESOLVE,id,wxID_ANY, \
                               (wxObjectEventFunction)(wxEventFunction) \
                               wxStaticCastEvent(wxCommandEventFunction,&fn), \
                               (wxObject*)NULL \
                             ),

DEFINE_EVENT_TYPE(wxCSL_EVT_RESOLVE)
DEFINE_EVENT_TYPE(wxCSL_EVT_PING_STATS)

BEGIN_EVENT_TABLE(CslEngine,wxEvtHandler)
    CSL_EVT_PING(wxID_ANY,CslEngine::OnPong)
    CSL_EVT_RESOLVE(wxID_ANY,CslEngine::OnResolve)
END_EVENT_TABLE()


#ifdef __WXDEBUG__
void Debug_Printf(const char *DbgFunc, const char *FmtStr,...)
{
    va_list ArgList;
    va_start(ArgList,FmtStr);
    fprintf(stdout,"%s(): ",DbgFunc);
    vfprintf(stdout,FmtStr,ArgList);
    fflush(stdout);
    va_end(ArgList);
}
#endif


void putint(ucharbuf &p, int n)
{
    if (n<128 && n>-127) p.put(n);
    else if (n<0x8000 && n>=-0x8000)
    {
        p.put(0x80);
        p.put(n);
        p.put(n>>8);
    }
    else
    {
        p.put(0x81);
        p.put(n);
        p.put(n>>8);
        p.put(n>>16);
        p.put(n>>24);
    }
}

int getint(ucharbuf &p)
{
    int c = (char)p.get();
    if (c==-128)
    {
        int n = p.get();
        n |= char(p.get())<<8;
        return n;
    }
    else if (c==-127)
    {
        int n = p.get();
        n |= p.get()<<8;
        n |= p.get()<<16;
        return n|(p.get()<<24);
    }
    else return c;
}

void putstring(const char *t, ucharbuf &p)
{
    while (*t) putint(p,*t++);
    putint(p,0);
}

void getstring(char *text, ucharbuf &p, int len)
{
    char *t=text;
    do
    {
        if (t>=&text[len])
        {
            text[len-1]=0;
            return;
        }
        if (!p.remaining())
        {
            *t=0;
            return;
        }
        *t=getint(p);
    }
    while (*t++);
}


void *CslResolverThread::Entry()
{
    m_mutex.Lock();

    while (!m_terminate)
    {
        if (!m_packets.length())
        {
            LOG_DEBUG("waiting\n");
            m_condition->Wait();
            LOG_DEBUG("got signal\n");
            if (m_terminate)
                break;
        }

        if (!m_packets.length())
            continue;

        m_packetSection.Enter();
        CslResolverPacket *packet=m_packets[0];
        m_packets.remove(0);
        m_packetSection.Leave();

        packet->m_domain=packet->m_address.Hostname();

        wxCommandEvent evt(wxCSL_EVT_RESOLVE);
        evt.SetClientData(packet);
        wxPostEvent(m_evtHandler,evt);
    }

    loopv(m_packets) delete m_packets[i];
    m_mutex.Unlock();
    LOG_DEBUG("exit\n");

    return NULL;
}

void CslResolverThread::AddPacket(CslResolverPacket *packet)
{
    m_packetSection.Enter();
    m_packets.add(packet);
    m_packetSection.Leave();
    //wxMutexLocker lock(m_mutex);
    m_condition->Signal();
}

void CslResolverThread::Terminate()
{
    m_terminate=true;
    wxMutexLocker lock(m_mutex);
    m_condition->Signal();
}


CslEngine::CslEngine(wxEvtHandler *evtHandler) : wxEvtHandler(),
        m_isOk(false),
        m_evtHandler(evtHandler),
        m_resolveThread(NULL),
        m_currentGame(NULL),
        m_currentMaster(NULL)
{
    if (m_evtHandler)
        SetNextHandler(m_evtHandler);
    GetTicks();
};

CslEngine::~CslEngine()
{
    DeInitEngine();
}

bool CslEngine::InitEngine(wxUint32 updateInterval)
{
    if (m_isOk)
        return false;

    SetUpdateInterval(updateInterval);
    m_pingSock=new CslUDP(this);
    m_isOk=m_pingSock->IsInit();

    if (m_isOk)
    {
        m_resolveThread=new CslResolverThread(this);
        m_resolveThread->Run();
    }

    m_firstPing=true;

    return m_isOk;
}

void CslEngine::DeInitEngine()
{
    if (m_resolveThread)
    {
        m_resolveThread->Terminate();
        m_resolveThread->Delete();
        delete m_resolveThread;
    }

    delete m_pingSock;
    loopv(m_games) delete m_games[i];
}

CslGame* CslEngine::AddMaster(CslMaster *master)
{
    if (!master)
    {
        wxASSERT(master);
        return NULL;
    }
    CslGame *game=FindOrCreateGame(master->GetType());
    if (game->AddMaster(master)>=0)
        return game;

    return NULL;
}

void CslEngine::DeleteMaster(CslMaster *master)
{
    if (!master)
    {
        wxASSERT(master);
        return;
    }
    master->GetGame()->DeleteMaster(master);
}

CslServerInfo* CslEngine::AddServer(CslServerInfo *info,wxInt32 masterid)
{
    wxMutexLocker lock(m_mutex);

    CslGame *game=FindOrCreateGame(info->m_type);
    if (!game)
        return NULL;

    CslServerInfo *ret=game->AddServer(info,masterid);
    if (ret && ret->IsFavourite())
    {
        if (m_favourites.find(ret)<0)
            m_favourites.add(ret);
    }

    if (ret)
    {
        CslResolverPacket *packet=new CslResolverPacket(ret->m_addr,ret->m_type);
        m_resolveThread->AddPacket(packet);
    }

    return ret;
}

bool CslEngine::AddServerToFavourites(CslServerInfo *info)
{
    wxMutexLocker lock(m_mutex);

    if (m_favourites.find(info)>=0)
        return false;

    info->SetFavourite();
    m_favourites.add(info);

    return true;
}

void CslEngine::RemoveServerFromMaster(CslServerInfo *info)
{
    wxMutexLocker lock(m_mutex);
    m_currentGame->RemoveServer(info,m_currentMaster);
}

void CslEngine::RemoveServerFromFavourites(CslServerInfo *info)
{
    wxMutexLocker lock(m_mutex);

    wxInt32 i=m_favourites.find(info);
    if (i>=0)
        m_favourites.remove(i);
    info->RemoveFavourite();
}

void CslEngine::DeleteServer(CslServerInfo *info)
{
    wxMutexLocker lock(m_mutex);

    if (info->IsLocked())
        return;

    if (info->IsFavourite())
    {
        wxUint32 i=m_favourites.find(info);
        if (i>=0) m_favourites.remove(i);
    }

    CslGame *game=FindGame(info->m_type);
    if (!game)
    {
        wxASSERT(game);
        return;
    }
    game->DeleteServer(info);
}

CslGame* CslEngine::FindGame(CSL_GAMETYPE type)
{
    CslGame *game;

    loopv(m_games)
    {
        game=m_games[i];
        if (game->GetType()==type)
            return game;
    }
    return NULL;
}

CslGame* CslEngine::FindOrCreateGame(CSL_GAMETYPE type)
{
    CslGame *game=FindGame(type);

    if (game)
        return game;

    game=new CslGame(type);
    m_games.add(game);

    return game;
}

bool CslEngine::SetCurrentGame(CslGame *game,CslMaster *master)
{
    wxMutexLocker lock(m_mutex);

    if (game)
        if (m_games.find(game)<0)
            return false;
    if (master)
        if (game->m_masters.find(master)<0)
            return false;

    m_currentGame=game;
    m_currentMaster=master;

    FuzzPingSends();

    return true;
}

bool CslEngine::Ping(CslServerInfo *info,bool force)
{
    CslUDPPacket *packet;
    uchar ping[16];

    wxUint32 ticks=GetTicks();
    wxUint32 interval=m_updateInterval;

    if (info->m_waiting)
    {
        interval=CSL_UPDATE_INTERVAL_WAIT;
        LOG_DEBUG("iswaiting\n");
    }

    //LOG_DEBUG("%s - ticks:%li, pingsend:%li, diff:%li\n",U2A(info->GetBestDescription()),
    //          ticks,info->m_pingSend,ticks-info->m_pingSend);
    if (!force && (ticks-info->m_pingSend)<interval)
        return false;
    if (info->m_addr.IPAddress()==wxT("255.255.255.255"))
        return false;

    info->m_pingSend=ticks;

#ifdef CSL_EXT_SERVER_INFO
    // Send uptime cmd first and check for ack in UpdateServerInfo()
    if ((info->m_type==CSL_GAME_SB) &&
        (info->m_ping<0 || info->m_extended) &&
        (info->CanPingUptime()))
    {
        return PingUptime(info);
    }
#endif

    // default ping packet
    packet=new CslUDPPacket();
    ucharbuf p(ping,sizeof(ping));
    putint(p,info->m_pingSend);
    packet->Set(info->m_addr,ping,p.length());
    m_pingSock->SendPing(packet);

    //LOG_DEBUG("Ping %s - %d\n",U2A(info->GetBestDescription()),ticks);

    return true;
}

bool CslEngine::PingUptime(CslServerInfo *info)
{
    //LOG_DEBUG("uptime %s - %d\n",U2A(info->GetBestDescription()),GetTicks());
    uchar ping[16];

    CslUDPPacket *packet=new CslUDPPacket();
    ucharbuf p(ping,sizeof(ping));
    putint(p,0);
    putstring("uptime",p);
    packet->Set(info->m_addr,ping,p.length());
    m_pingSock->SendPing(packet);

    return true;
}

bool CslEngine::PingStats(CslServerInfo *info)
{
    if (!info->m_extended)
        return false;

    CslUDPPacket *packet;
    uchar ping[16];

    ucharbuf p(ping, sizeof(ping));
    putint(p,0);
//    putstring("stats",p);

    packet=new CslUDPPacket();
    packet->Set(info->m_addr,ping,p.length());
    return m_pingSock->SendPing(packet);
}

wxUint32 CslEngine::PingServers()
{
    if (m_currentGame==NULL)
        return 0;

    if (m_mutex.TryLock()==wxMUTEX_BUSY)
    {
        LOG_DEBUG("wxMUTEX_BUSY\n");
        return 0;
    }

    wxUint32 pc=0;

    CslServerInfo *info;
    vector<CslServerInfo*> *servers=m_currentGame->GetServers();

    loopv(*servers)
    {
        info=servers->at(i);
        /*if (m_currentMaster)
        {
        CslMaster *master=info->m_master;
        if (master && m_currentMaster->Address()!=master->Address())
        continue;
        }*/

        bool force=m_firstPing&&i<5;
        if (Ping(info,force))
            pc++;
    }

    loopv(m_favourites)
    {
        info=m_favourites[i];
        if (Ping(info))
            pc++;
    }

//  #ifdef __WXDEBUG__
//      if (pc)
//          LOG_DEBUG("Pinged %d of %d\n",pc,servers->length()+m_favourites.length());
//  #endif

    m_mutex.Unlock();

    m_firstPing=false;

    return pc;
}

int CslEngine::UpdateMaster()
{
    int num=0;
    uchar *buf=NULL;

    if (m_currentMaster==NULL)
        return -1;

    wxPlatformInfo pinfo;
    wxOperatingSystemId id=pinfo.GetOperatingSystemId();
    wxString os=pinfo.GetOperatingSystemFamilyName(id);
    wxString agent=wxT("Csl/");
    agent+=CSL_VERSION_LONG_STR;
    agent+=wxT("(")+os+wxT(")");

    wxHTTP http;
    if (!http.Connect(m_currentMaster->GetAddress(),80))
        return -2;
    http.SetHeader(wxT("User-Agent"),agent);
    wxInputStream *data=http.GetInputStream(m_currentMaster->GetPath());

    wxUint32 code=http.GetResponse();

    if (!data||code!=200)
        return -3;

    size_t size=data->GetSize();

    if (!size)
    {
        num=-4;
        goto finish;
    }
    // soee server do not report the size of the content
    // somehow set a limit
    if (size>32767)
        size=32767;

    buf=(uchar*)calloc(1,size+1);
    if (!buf)
    {
        num=-5;
        goto finish;
    }

    data->Read((void*)buf,size);

    if (strstr((char*)buf,"<html>")||strstr((char*)buf,"<HTML>"))
        num=-4;
    else
    {
        vector <CslServerInfo*> *servers=m_currentMaster->GetServers();
        loopv(*servers) RemoveServerFromMaster(servers->at(i));

        char *p=(char*)buf;
        char *pend;
        bool end=false;
        CslServerInfo *info,*ret;

        while ((p=strstr(p,"addserver"))!=NULL)
        {
            p=strpbrk(p,"123456789 \t");
            while (*p && (*p==' ' || *p=='\t'))
                *p++;
            if (!p)
                return -3;
            pend=strpbrk(p," \t\r\n");
            if (!pend)
                end=true;
            else
                *pend=0;

            info=new CslServerInfo(A2U(p),m_currentMaster->GetType());
            ret=AddServer(info,m_currentMaster->GetID());
            if (!ret || ret!=info)
                delete info;
            num++;

            if (end)
                break;

            p=pend+1;
        }
    }

finish:
    if (buf) free(buf);
    if (data) delete data;

    FuzzPingSends();

    return num;
}

void CslEngine::FuzzPingSends()
{
    vector<CslServerInfo*> *servers;

    if (m_currentMaster)
        servers=m_currentMaster->GetServers();
    else if (m_currentGame)
        servers=m_currentGame->GetServers();
    else
        return;

    wxUint32 ticks=GetTicks();

    CslServerInfo *info;
    loopv(*servers)
    {
        info=servers->at(i);
        info->m_pingSend=ticks-(i*500)%m_updateInterval;
    }
}

void CslEngine::UpdateServerInfo(CslServerInfo *info,ucharbuf *buf,wxUint32 now)
{
    char text[128];

    info->m_pingResp=GetTicks();
    info->m_ping=info->m_pingResp-info->m_pingSend;
    info->m_lastSeen=now;

    switch (info->m_type)
    {
        case CSL_GAME_SB:
        case CSL_GAME_BF:
        {
            info->m_players=getint(*buf);
            int numattr=getint(*buf);
            vector<int>attr;
            attr.setsize(0);
            loopj(numattr) attr.add(getint(*buf));
            if (numattr>=1)
                info->m_protocol=attr[0];
            if (numattr>=2)
                info->m_gameMode=GetModeStrSB(attr[1]);
            if (numattr>=3)
            {
                info->m_timeRemain=attr[2];
                if (info->m_protocol<254)
                    info->m_timeRemain++;
            }
            if (numattr>=4)
                info->m_playersMax=attr[3];
            if (numattr>=5)
                info->m_mm=attr[4];

            getstring(text,*buf,sizeof(text));
            info->m_map=A2U(text);
            getstring(text,*buf,sizeof(text));
            info->m_desc=A2U(text);
            break;
        }

        case CSL_GAME_CB:
        {
            // this sucks!
            wxInt32 i;
            for (i=0;i<buf->maxlength();i++)
                *buf->at(i)^=0x61;
            // dont break, nearly same as AC
        }
        case CSL_GAME_AC:
        {

            wxInt32 prot=getint(*buf);
            if (info->m_type==CSL_GAME_CB && prot!=CSL_LAST_PROTOCOL_CB)
                return;
            info->m_protocol=prot;
            info->m_gameMode=info->m_type==CSL_GAME_AC ?
                             GetModeStrAC(getint(*buf)):
                             GetModeStrSB(getint(*buf));
            info->m_players=getint(*buf);
            info->m_timeRemain=getint(*buf);
            getstring(text,*buf,sizeof(text));
            info->m_map=A2U(text);
            getstring(text,*buf,sizeof(text));
            info->m_desc=A2U(text);
            if (info->m_type==CSL_GAME_AC)
                info->m_playersMax=getint(*buf);
            break;
        }

        default:
            wxASSERT(info->m_type);
    }
}

void CslEngine::ParsePongCmd(CslServerInfo *info,CslUDPPacket *packet,wxUint32 now)
{
    char text[128];
    wxUint32 v=0;
    bool extended=false;
    ucharbuf p((uchar*)packet->Data(),packet->Size());

#ifdef __WXDEBUG__
    wxString dbg_type;
#endif

    v=getint(p);
    if (!v)
        extended=true;

    while (packet->Size() > (wxUint32)p.length())
    {
        switch (info->m_type)
        {
            case CSL_GAME_SB:
            case CSL_GAME_BF:
            {
                if (extended)
                {
                    getstring(text,p,sizeof(text));

                    if (strcmp(text,"uptime")==0)
                    {
#ifdef __WXDEBUG__
                        dbg_type=wxT("uptime");
#endif
                        v=p.length();
                        if (getint(p)!=-1)
                        {
                            p.len=v;
                            UpdateServerInfo(info,&p,now);
                            break;
                        }
                        else
                            Ping(info,true);

                        info->m_uptime=getint(p);
                        if (info->m_uptime)
                            info->m_extended=true;
                        LOG_DEBUG("uptime (%s) %s\n",U2A(info->GetBestDescription()),
                                  U2A(FormatSeconds(info->m_uptime)));
                    }
                    else if (strcmp(text,"stats")==0)
                    {
#ifdef __WXDEBUG__
                        dbg_type=wxT("stats");
#endif
                        if (getint(p)!=-1)
                            break;

                        wxUint32 extProt=getint(p);
                        if (extProt<101)
                            return;

                        LOG_DEBUG("stats (%s) prot:%d\n",U2A(info->GetBestDescription()),extProt);

                        CslPlayerStats *stats=NULL;
                        loopv(info->m_playerStats)
                        {
                            if (info->m_playerStats[i]->m_ok)
                                continue;
                            stats=info->m_playerStats.at(i);
                        }

                        if (!stats)
                        {
                            stats=new CslPlayerStats;
                            info->m_playerStats.add(stats);
                        }

                        stats->m_id=getint(p);
                        getstring(text,p,sizeof(text));
                        stats->m_player=A2U(text);
                        getstring(text,p,sizeof(text));
                        stats->m_team=A2U(text);
                        stats->m_frags=getint(p);
                        stats->m_deaths=getint(p);
                        stats->m_teamkills=getint(p);

                        if (p.overread())
                        {
                            LOG_DEBUG("stats(%s) OVERREAD!\n",U2A(info->GetBestDescription()));
                            break;
                        }

                        stats->m_ok=true;

                        LOG_DEBUG("stats (%s): player:%s, team:%s, frags:%d, deaths:%d, tk:%d,\n",
                                  U2A(info->GetBestDescription()),
                                  U2A(stats->m_player),U2A(stats->m_team),
                                  stats->m_frags,stats->m_deaths,stats->m_teamkills);

                        wxCommandEvent evt(wxCSL_EVT_PING_STATS);
                        evt.SetClientData((void*)info);
                        wxPostEvent(m_evtHandler,evt);
                    }
                    else
                    {
#ifdef __WXDEBUG__
                        dbg_type=wxT("unknown");
#endif
                        break;
                    }
                }
                else
                    UpdateServerInfo(info,&p,now);

                break;
            }

            case CSL_GAME_AC:
            case CSL_GAME_CB:
                UpdateServerInfo(info,&p,now);
                break;

            default:
                break;
        }
#ifdef __WXDEBUG__
        if (p.length()<(wxInt32)packet->Size())
            LOG_DEBUG("%s: %d bytes left (type=%s)\n",U2A(info->GetBestDescription()),
                      packet->Size()-p.length(),U2A(dbg_type));
#endif
    }
}

void CslEngine::OnPong(wxCommandEvent& event)
{
    CslUDPPacket *packet=(CslUDPPacket*)event.GetClientData();
    if (!packet)
        return;

    if (!packet->Size() || !m_currentGame)
    {
        delete packet;
        return;
    }

    wxMutexLocker lock(m_mutex);

    bool skip=false;
    CslServerInfo *info=NULL;

    wxDateTime now=wxDateTime::Now();
    wxUint32 ticks=now.GetTicks();

    if ((info=m_currentGame->FindServerByAddr(packet->Address()))!=NULL)
    {
        skip=true;
        ParsePongCmd(info,packet,ticks);
    }

    if (!skip)
    {
        loopv(m_favourites)
        {
            info=m_favourites[i];
            if (info->m_addr.IPAddress()==packet->Address().IPAddress() &&
                info->m_addr.Service()==packet->Address().Service())
            {
                skip=true;
                ParsePongCmd(info,packet,ticks);
                break;
            }
        }
    }

    delete packet;

    if (skip && m_evtHandler)
    {
        event.SetClientData((void*)info);
        event.Skip();
    }
}

void CslEngine::OnResolve(wxCommandEvent& event)
{
    CslServerInfo *info;
    CslGame *game=NULL;
    CslResolverPacket *packet=(CslResolverPacket*)event.GetClientData();

    if (packet->m_domain.IsEmpty())
        goto finish;

    loopv(m_games)
    {
        game=m_games[i];
        if (game->GetType()==packet->m_type)
        {
            wxMutexLocker lock(m_mutex);

            if ((info=game->FindServerByAddr(packet->m_address))!=NULL)
                info->m_domain=packet->m_domain;
            break;
        }
    }

finish:
    delete packet;
}
