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

#include "CslGame.h"

const wxChar* GetVersionStrSB(int n)
{
    static const wxChar* sb_versions[] =
    {
        wxT("Summer"), wxT("Spring"), wxT("Gui"), wxT("Water"),
        wxT("Normalmap"), wxT("Sp"), wxT("Occlusion"), wxT("Shader"),
        wxT("Physics"), wxT("Mp"), wxT(""), wxT("Agc"), wxT("Quakecon"),
        wxT("Independence")
    };
    size_t v=CSL_LAST_PROTOCOL_SB-n;
    return (v>=0 && v<sizeof(sb_versions)/sizeof(sb_versions[0])) ?
           sb_versions[v] : wxString::Format(wxT("%d"),n).c_str();
}

const wxChar* GetVersionStrAC(int n)
{
    static const wxChar* ac_versions[] =
    {
        wxT("0.93.x"), wxT("0.92"), wxT("0.91.x"), wxT("0.90")
    };
    size_t v=CSL_LAST_PROTOCOL_AC-n;
    return (v>=0 && v<sizeof(ac_versions)/sizeof(ac_versions[0])) ?
           ac_versions[v] : wxString::Format(wxT("%d"),n).c_str();
}

const wxChar* GetVersionStrCB(int n)
{
    static const wxChar* cb_versions[] =
    {
        wxT("122")
    };
    size_t v=CSL_LAST_PROTOCOL_CB-n;
    return (v>=0 && v<sizeof(cb_versions)/sizeof(cb_versions[0])) ?
           cb_versions[v] : wxString::Format(wxT("%d"),n).c_str();
}

const wxChar* GetModeStrSB(int n)
{
    static const wxChar* sb_modenames[] =
    {
        wxT("ffa/default"), wxT("coopedit"), wxT("ffa/duel"), wxT("teamplay"),
        wxT("instagib"), wxT("instagib team"), wxT("efficiency"), wxT("efficiency team"),
        wxT("insta arena"), wxT("insta clan arena"), wxT("tactics arena"), wxT("tactics clan arena"),
        wxT("capture"), wxT("insta capture")
    };
    return (n>=0 && (size_t)n<sizeof(sb_modenames)/sizeof(sb_modenames[0])) ? sb_modenames[n] : _("unknown");
}

const wxChar* GetModeStrAC(int n)
{
    static const wxChar* ac_modenames[] =
    {
        wxT("team deathmatch"), wxT("coopedit"), wxT("deathmatch"), wxT("survivor"),
        wxT("team survivor"), wxT("ctf"), wxT("pistol frenzy"), wxT("bot team deathmatch"),
        wxT("bot deathmatch"), wxT("last swiss standing"), wxT("one shot, one kill"),
        wxT("team one shot, one kill"), wxT("bot one shot, one skill")
    };
    return (n>=0 && (size_t)n<sizeof(ac_modenames)/sizeof(ac_modenames[0])) ? ac_modenames[n] : _("unknown");
}

const wxChar* GetGameStr(int n)
{
    static const wxChar *game_names[] =
    {
        wxT("Sauerbraten"), wxT("AssaultCube"), wxT("Cube")
    };
    return (n>CSL_GAME_START && n<=CSL_GAME_END &&
            (size_t)(n-1)<sizeof(game_names)/sizeof(game_names[0])) ? game_names[n-1] : wxT("none");
}


CslGame::CslGame(CSL_GAMETYPE type) : m_type(type), m_masterID(0)
{
    switch (m_type)
    {
        case CSL_GAME_SB:
            m_name=CSL_DEFAULT_NAME_SB;
            break;
        case CSL_GAME_AC:
            m_name=CSL_DEFAULT_NAME_AC;
            break;
        case CSL_GAME_CB:
            m_name=CSL_DEFAULT_NAME_CB;
            break;
        default:
            wxASSERT_MSG(!m_type,wxT("unknown game"));
            break;
    }
}

CslGame::~CslGame()
{
    DeleteMaster();
    loopv(m_servers) delete m_servers[i];
}

wxInt32 CslGame::AddMaster(CslMaster *master)
{
    CslMaster *ml;

    loopv(m_masters)
    {
        ml=m_masters[i];
        if (*ml==*master)
            return -1;
    }

    wxInt32 id=0;
    loopv(m_masterIDs)    // find a free id
    {
        if (m_masterIDs[i]>i)
        {
            id=i;
            break;
        }
        id++;
    }
    master->Init(this,id);
    m_masters.add(master);
    m_masterIDs.insert(id,id);

    return id;
}

void CslGame::DeleteMaster(CslMaster *master)
{
    CslMaster *ml;
    wxInt32 idpos;

    loopvrev(m_masters)
    {
        if (master)
        {
            ml=m_masters[i];
            if (ml!=master)
                continue;

            m_masters.remove(i);
            idpos=m_masterIDs.find(master->GetID());
            m_masterIDs.remove(idpos);
            delete master;
            return;
        }
        else
            delete m_masters[i];
    }
    if (!master)
        m_masters.setsize(0);
}

CslServerInfo* CslGame::AddServer(CslServerInfo *info,wxInt32 id)
{
    bool ma=false;
    CslMaster *master=NULL;
    CslServerInfo *il;
    vector<CslServerInfo*> *servers;

    if (id>=0)
    {
        loopv(m_masters)
        {
            master=m_masters[i];
            if (id!=master->m_id) // find master with this id
                continue;

            servers=master->GetServers();
            loopvj(*servers)     // check if server was already added
            {
                il=servers->at(j);
                if (*il==*info)
                    return il;
            }
            break;
        }
        if (!master)
            return NULL;
        ma=true;
    }

    loopv(m_servers)
    {
        il=m_servers[i];
        if (*il==*info)
        {
            if (ma)
                master->AddServer(il);
            return il;
        }
    }

    if (ma)
        master->AddServer(info);
    m_servers.add(info);

    return info;
}

bool CslGame::RemoveServer(CslServerInfo *info,CslMaster *master)
{
    if (!master)
        return false;
    return master->RemoveServer(info);
}

// bool CslGame::RemoveServers(CslMaster *master)
// {
//     master->RemoveServers();
// }

bool CslGame::DeleteServer(CslServerInfo *info)
{
    loopv(info->m_masterIDs)
    {
        wxInt32 id=info->m_masterIDs[i];
        loopvj(m_masters)
        {
            CslMaster *master=m_masters[j];
            if (master->GetID()==id)
            {
                master->RemoveServer(info);
                break;
            }
        }
    }

    loopv(m_servers)
    {
        if (m_servers[i]==info)
        {
            m_servers.remove(i);
            delete info;
            return true;
        }
    }

    return false;
}

CslServerInfo* CslGame::FindServerByAddr(const wxIPV4address addr)
{
    CslServerInfo *info;

    loopv(m_servers)
    {
        info=m_servers[i];
        if (info->m_addr.IPAddress()==addr.IPAddress() &&
            info->m_addr.Service()==addr.Service())
            return info;
    }

    return NULL;
}
