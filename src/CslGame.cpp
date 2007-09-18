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

#include <wx/file.h>
#include "CslGame.h"
#include "CslTools.h"

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
    loopv(m_servers)
    {
        m_servers[i]->DeleteStats();
        delete m_servers[i];
    }
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
            info->DeleteStats();
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

wxInt32 CslGame::ConnectCleanup(const CSL_GAMETYPE type,const wxString& cfg)
{
    if (cfg.IsEmpty())
        return CSL_ERROR_NONE;

    switch (type)
    {
        case CSL_GAME_SB:
            return ConnectWriteConfig(type,cfg,wxT("\r\n"));

        case CSL_GAME_AC:
        case CSL_GAME_CB:
        {
            wxString bak=cfg+wxT(".csl");
            if (!::wxFileExists(cfg) || !::wxFileExists(bak))
                return CSL_ERROR_FILE_DONT_EXIST;
            if (!::wxRenameFile(bak,cfg))
                return CSL_ERROR_FILE_OPERATION;
            break;
        }

        default:
            return CSL_ERROR_GAME_UNKNOWN;
    }

    return CSL_ERROR_NONE;
}

wxInt32 CslGame::ConnectWriteConfig(const CSL_GAMETYPE type,const wxString& cfg,const wxString& str)
{
    wxUint32 c,l;
    wxFile file;

    switch (type)
    {
        case CSL_GAME_SB:
            file.Open(cfg,wxFile::write);
            break;

        case CSL_GAME_AC:
        case CSL_GAME_CB:
            file.Open(cfg,wxFile::write_append);
            break;

        default:
            return CSL_ERROR_GAME_UNKNOWN;
    }

    if (!file.IsOpened())
        return CSL_ERROR_FILE_OPERATION;
    l=str.Len();
    c=file.Write(U2A(str),l);
    file.Close();
    if (l!=c)
        return CSL_ERROR_FILE_OPERATION;

    return 0;
}

wxInt32 CslGame::ConnectPrepare(const CslServerInfo *info,const wxString& path,wxString *out)
{
    wxString s;
    wxString src;
    wxString cfg;

    switch (info->m_type)
    {
        case CSL_GAME_SB:
        {
            wxString map=wxString(CSL_DEFAULT_INJECT_FIL_SB)+wxString(wxT(".ogz"));
            wxString dst=path+CSL_DEFAULT_INJECT_DIR_SB+map;
            cfg=path+CSL_DEFAULT_INJECT_DIR_SB+CSL_DEFAULT_INJECT_FIL_SB+wxT(".cfg");

            if (!::wxFileExists(dst))
            {
                src=A2U(DATADIR)+PATHDIV+map;
                if (!::wxFileExists(src))
                {
                    src=::wxPathOnly(wxTheApp->argv[0])+PATHDIV+wxT("data")+PATHDIV+map;
                    if (!::wxFileExists(src))
                    {
                        *out=wxString::Format(_("Couldn't find \"%s\""),map.c_str());
                        return CSL_ERROR_FILE_DONT_EXIST;
                    }
                }
                if (!::wxCopyFile(src,dst))
                    return CSL_ERROR_FILE_OPERATION;
            }
            break;
        }

        case CSL_GAME_AC:
            cfg=path+CSL_DEFAULT_INJECT_DIR_AC+CSL_DEFAULT_INJECT_FIL_AC;
            if (::wxFileExists(cfg))
                if (!::wxCopyFile(cfg,cfg+wxT(".csl")))
                    return CSL_ERROR_FILE_OPERATION;
            break;

        case CSL_GAME_CB:
            cfg=path+CSL_DEFAULT_INJECT_DIR_CB+CSL_DEFAULT_INJECT_FIL_CB;
            if (::wxFileExists(cfg))
                if (!::wxCopyFile(cfg,cfg+wxT(".csl")))
                    return CSL_ERROR_FILE_OPERATION;
            break;

        default:
            wxASSERT_MSG(false,wxT("unknown game"));
            return CSL_ERROR_GAME_UNKNOWN;
    }

    *out=cfg;
    s=wxString::Format(wxT("\r\n/sleep 1000 [ connect %s ]\r\n"),info->m_host.c_str());
    return ConnectWriteConfig(info->m_type,cfg,s);
}
