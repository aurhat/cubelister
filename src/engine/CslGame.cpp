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
#include "CslGame.h"


/**
 *  class CslMaster
 */

CslMaster::CslMaster() : m_game(NULL),m_id(0) {}

CslMaster::CslMaster(const CslMasterConnection& connection):
        m_game(NULL),m_id(0),m_connection(connection) {}

CslMaster::~CslMaster()
{
    UnrefServers();
}

void CslMaster::Create(const CslMasterConnection& connection)
{
    m_connection=connection;
}

void CslMaster::AddServer(CslServerInfo *info)
{
    loopv(m_servers)
    {
        if (*m_servers[i]==*info)
        {
            info->AddMaster(m_id);
            return;
        }
    }

    m_servers.add(info);
    info->AddMaster(m_id);
}

bool CslMaster::UnrefServer(CslServerInfo *info,wxInt32 pos)
{
    if (pos<0)
        if ((pos=m_servers.find(info))<0)
            return false;

    m_servers.remove(pos);
    info->RemoveMaster(m_id);

    return true;
}

void CslMaster::UnrefServers()
{
    loopvrev(m_servers) UnrefServer(m_servers[i],i);
}

void CslMaster::Init(CslGame *game,wxUint32 id)
{
    m_game=game;
    m_id=id;
}

/**
 *  class CslGame
 */

CslGame::CslGame() :
        m_gameId(-1),m_capabilities(0),
        m_configType(CSL_CONFIG_EXE)
{
}

CslGame::~CslGame()
{
    DeleteMasters();
    loopv(m_servers) m_servers[i]->Destroy();
}

wxInt32 CslGame::AddMaster(CslMaster *master)
{
    loopv(m_masters)
    {
        if (*m_masters[i]==*master)
            return -1;
    }

    wxInt32 id=m_masters.length();
    master->Init(this,id);
    m_masters.add(master);

    return id;
}

bool CslGame::DeleteMaster(wxInt32 masterID,wxInt32 pos)
{
    if (pos<0)
    {
        loopv(m_masters)
        {
            if (m_masters[i]->GetId()==masterID)
            {
                pos=i;
                break;
            }
        }
        if (pos<0)
            return false;
    }

    delete m_masters[pos];
    m_masters.remove(pos);

    return true;
}

void CslGame::DeleteMasters()
{
    loopvrev(m_masters) DeleteMaster(m_masters[i]->GetId(),i);
}

bool CslGame::AddServer(CslServerInfo *info,wxInt32 masterID)
{
    CslMaster *master=NULL;
    CslServerInfo *server=NULL;
    bool found=false;

    loopv(m_servers)
    {
        if (*m_servers[i]==*info)
        {
            server=m_servers[i];
            found=true;
            break;
        }
    }

    if (!found)
    {
        m_servers.add(info);
        server=info;
    }

    if (masterID>-1)
    {
        loopv(m_masters)
        {
            master=m_masters[i];
            if (masterID==master->GetId())
            {
                master->AddServer(server);
                break;
            }
            else
                master=NULL;
        }
        if (!master)
        {
            if (!found)
                m_servers.drop();
            return false;
        }
    }

    return !found;
}

bool CslGame::DeleteServer(CslServerInfo *info)
{
    loopv(info->m_masterIDs)
    {
        wxInt32 id=info->m_masterIDs[i];
        loopvj(m_masters)
        {
            CslMaster *master=m_masters[j];
            if (master->GetId()==id)
            {
                master->UnrefServer(info);
                break;
            }
        }
    }

    loopv(m_servers)
    {
        if (m_servers[i]==info)
        {
            m_servers.remove(i);
            info->Destroy();
            return true;
        }
    }

    return false;
}

CslServerInfo* CslGame::FindServerByAddr(const wxString host,wxUint16 port)
{
    CslServerInfo *info;

    loopv(m_servers)
    {
        info=m_servers[i];
        if (info->Host==host && info->Addr.Service()==port)
            return info;
    }

    return NULL;
}

CslServerInfo* CslGame::FindServerByAddr(const wxIPV4address& addr)
{
    CslServerInfo *info;

    loopv(m_servers)
    {
        info=m_servers[i];

        if (info->Addr.IPAddress()==addr.IPAddress() &&
            info->Addr.Service()==addr.Service())
            return info;
    }

    return NULL;
}

void CslGame::GetFavourites(vector <CslServerInfo*>& servers)
{
    loopv(m_servers)
    {
        if (m_servers[i]->IsFavourite())
            servers.add(m_servers[i]);
    }
}

void CslGame::GetExtServers(vector <CslServerInfo*>& servers,bool all)
{
    loopv(m_servers)
    {
        if ((all || m_servers[i]->PingExt()) && m_servers[i]->ExtInfoStatus==CSL_EXT_STATUS_OK)
            servers.add(m_servers[i]);
    }
}

void CslGame::GetPlayerstatsDescriptions(vector<wxString>& desc) const
{
    desc.add(_("Player"));
    desc.add(_("Team"));
    desc.add(_("Frags"));
    desc.add(_("Deaths"));
    desc.add(_("Teamkills"));
    desc.add(_("Ping"));
    desc.add(_("Accuracy"));
    desc.add(_("Health"));
    desc.add(_("Armour"));
    desc.add(_("Weapon"));
}

/**
 *  class CslServerInfo
 */

CslServerInfo::CslServerInfo(CslGame *game,
                             const wxString& host,
                             wxUint16 gamePort,wxUint16 infoPort,
                             wxUint32 view,wxUint32 lastSeen,
                             wxUint32 playedLast,wxUint32 playTimeLast,
                             wxUint32 playTimeTotal,wxUint32 connectedTimes,
                             const wxString& oldDescription,
                             const wxString& pass,const wxString& passAdm)
{
    m_game=game;
    Host=host;
    GamePort=gamePort ? gamePort:game ? game->GetDefaultGamePort():0;
    InfoPort=infoPort ? infoPort:GamePort ? GamePort+1:0;
    if (IsIP(host))
    {
        Addr.Hostname(host);
        Pingable=GamePort && InfoPort;
    }
    else
        Pingable=false;
    Addr.Service(InfoPort);
    Protocol=-1;
    Ping=-1;
    TimeRemain=-2;
    Players=-1;
    PlayersMax=-1;
    View=view;
    PingSend=0;
    PingResp=0;
    LastSeen=lastSeen;
    PlayedLast=playedLast;
    PlayTimeLast=playTimeLast;
    PlayTimeTotal=playTimeTotal;
    ConnectedTimes=connectedTimes;
    ConnectWait=0;
    DescriptionOld=oldDescription;
    MMDescription=wxEmptyString;
    MM=-1;
    Password=pass;
    PasswordAdmin=passAdm;
    Search=false;
    m_lock=0;
}

bool CslServerInfo::Create(CslGame *game,const wxString& host,
                           wxUint16 gamePort,wxUint16 infoPort,
                           const wxString& pass,
                           const wxString& passAdmin)
{
    if (!game)
        return false;

    m_game=game;
    Host=host;

    GamePort=gamePort ? gamePort:m_game->GetDefaultGamePort();
    InfoPort=infoPort ? infoPort:GamePort+1;

    if (IsIP(host))
    {
        Addr.Hostname(host);
        Pingable=GamePort && InfoPort;
    }
    else
        Pingable=false;
    Addr.Service(InfoPort);

    View=CSL_VIEW_FAVOURITE;

    Password=pass;
    PasswordAdmin=passAdmin;

    return true;
}

void CslServerInfo::Destroy()
{
    delete this;
}


void CslServerInfo::SetLastPlayTime(wxUint32 time)
{
    PlayTimeLast=time;
    PlayTimeTotal+=time;
}

void CslServerInfo::SetDescription(const wxString& description)
{
    Description=description;
    DescriptionOld=description;
}

wxString CslServerInfo::GetBestDescription() const
{
    if (Description.IsEmpty())
    {
        if (DescriptionOld.IsEmpty()) return Host;
        else return DescriptionOld;
    }
    return Description;
}

void CslServerInfo::Lock(bool lock)
{
    lock ? m_lock++:m_lock-=m_lock>0;
}

void CslServerInfo::AddMaster(wxInt32 id)
{
    m_masterIDs.add(id);
    SetDefault();
}
void CslServerInfo::RemoveMaster(wxInt32 id)
{
    wxInt32 i=m_masterIDs.find(id);
    if (id>=0) m_masterIDs.remove(i);
    if (m_masterIDs.length()==0) RemoveDefault();
}

bool CslServerInfo::HasStats() const
{
    return PlayedLast>0 || PlayTimeLast>0 ||
           PlayTimeTotal>0 || ConnectedTimes>0;
}
