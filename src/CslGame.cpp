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
    wxUint32 v=CSL_LAST_PROTOCOL_SB-n;
    return (v>=0 && v<sizeof(sb_versions)/sizeof(sb_versions[0])) ?
           sb_versions[v] : wxString::Format(wxT("%d"),n).c_str();
}

const wxChar* GetVersionStrAC(int n)
{
    static const wxChar* ac_versions[] =
    {
        wxT("0.93.x"), wxT("0.92"), wxT("0.91.x"), wxT("0.90")
    };
    wxUint32 v=CSL_LAST_PROTOCOL_AC-n;
    return (v>=0 && v<sizeof(ac_versions)/sizeof(ac_versions[0])) ?
           ac_versions[v] : wxString::Format(wxT("%d"),n).c_str();
}

const wxChar* GetVersionStrBF(int n)
{
    static const wxChar* bf_versions[] =
    {
        wxT("Alpha1")
    };
    wxUint32 v=CSL_LAST_PROTOCOL_BF-n;
    return (v>=0 && v<sizeof(bf_versions)/sizeof(bf_versions[0])) ?
           bf_versions[v] : wxString::Format(wxT("%d"),n).c_str();
}

const wxChar* GetVersionStrCB(int n)
{
    static const wxChar* cb_versions[] =
    {
        wxT("122")
    };
    wxUint32 v=CSL_LAST_PROTOCOL_CB-n;
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

const wxChar* GetWeaponStrSB(int n)
{
    static const wxChar* sb_weapons[] =
    {
        wxT("Fist"), wxT("Shotgun"), wxT("Chaingun"), wxT("Rocketlauncher"),
        wxT("Rifle"), wxT("Grenadelauncher"), wxT("Pistol"), wxT("Fireball"),
        wxT("Iceball"), wxT("Slimeball"), wxT("Bite")
    };
    return (n>=0 && (size_t)n<sizeof(sb_weapons)/sizeof(sb_weapons[0])) ? sb_weapons[n] : _("unknown");
}

const wxChar* GetWeaponStrAC(int n)
{
    static const wxChar* ac_weapons[] =
    {
        wxT("Knife"), wxT("Pistol"), wxT("Shotgun"), wxT("Subgun"),
        wxT("Sniper"), wxT("Assault"), wxT("Grenade")
    };
    return (n>=0 && (size_t)n<sizeof(ac_weapons)/sizeof(ac_weapons[0])) ? ac_weapons[n] : _("unknown");
}

const wxChar* GetWeaponStrCB(int n)
{
    static const wxChar* cb_weapons[] =
    {
        wxT("Fist"), wxT("Shotgun"), wxT("Chaingun"), wxT("Rocketlauncher"),
        wxT("Rifle"), wxT("Fireball"), wxT("Iceball"), wxT("Slimeball"), wxT("Bite")
    };
    return (n>=0 && (size_t)n<sizeof(cb_weapons)/sizeof(cb_weapons[0])) ? cb_weapons[n] : _("unknown");
}

const wxChar* GetGameStr(int n)
{
    static const wxChar *game_names[] =
    {
        CSL_DEFAULT_NAME_SB, CSL_DEFAULT_NAME_AC,
        CSL_DEFAULT_NAME_BF, CSL_DEFAULT_NAME_CB
    };
    return (n>CSL_GAME_START && n<=CSL_GAME_END &&
            (size_t)(n-1)<sizeof(game_names)/sizeof(game_names[0])) ? game_names[n-1] : _("unknown");
}


CslGame::CslGame(CSL_GAMETYPE type) : m_type(type), m_masterID(0)
{
    m_name=GetGameStr(type);
}

CslGame::~CslGame()
{
    DeleteMaster();
    loopv(m_servers)
    {
        m_servers[i]->Destroy();
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
            info->Destroy();
            return true;
        }
    }

    return false;
}

CslServerInfo* CslGame::FindServerByAddr(const wxIPV4address& addr)
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

wxString CslGame::GetWeaponName(const CSL_GAMETYPE type,const wxInt32 weapon)
{
    switch (type)
    {
        case CSL_GAME_SB:
            return GetWeaponStrSB(weapon);
        case CSL_GAME_AC:
            return GetWeaponStrAC(weapon);
        case CSL_GAME_BF:
            //TODO add weapons for BF
            return wxEmptyString;
        case CSL_GAME_CB:
            return GetWeaponStrCB(weapon);
        default:
            break;
    }

    wxASSERT_MSG(!type,wxT("unknown game"));
    return wxEmptyString;
}

wxInt32 CslGame::ConnectCleanupConfig(const CSL_GAMETYPE type,const wxString& cfg)
{
    if (cfg.IsEmpty())
        return CSL_ERROR_NONE;

    switch (type)
    {
        case CSL_GAME_SB:
        case CSL_GAME_BF:
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

wxInt32 CslGame::ConnectWriteConfig(const CSL_GAMETYPE& type,const wxString& cfg,const wxString& str)
{
    wxUint32 c,l;
    wxFile file;

    switch (type)
    {
        case CSL_GAME_SB:
        case CSL_GAME_BF:
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

wxInt32 CslGame::ConnectPrepareConfig(wxString& out,const CslServerInfo *info,const wxString& path,
                                      const wxString& password,const bool admin)
{
    wxString cfg;
    wxString script;

    switch (info->m_type)
    {
        case CSL_GAME_SB:
        case CSL_GAME_BF:
        {
            wxString src;
            wxString map=wxString(CSL_DEFAULT_INJECT_FIL_SB)+wxString(wxT(".ogz"));
            wxString dst=path+wxString(CSL_DEFAULT_INJECT_DIR_SB)+map;
            cfg=path+wxString(CSL_DEFAULT_INJECT_DIR_SB)+
				     wxString(CSL_DEFAULT_INJECT_FIL_SB)+
					 wxString(wxT(".cfg"));

            if (!::wxFileExists(dst))
            {
                src=DATAPATH+wxString(PATHDIV)+map;
#ifdef __WXGTK__
                if (!::wxFileExists(src))
                {
                    src=::g_basePath+wxT("/data/")+map;
#endif
                    if (!::wxFileExists(src))
                    {
                        out=wxString::Format(_("Couldn't find \"%s\""),map.c_str());
                        return CSL_ERROR_FILE_DONT_EXIST;
                    }
#ifdef __WXGTK__
                }
#endif
                if (!::wxCopyFile(src,dst))
                    return CSL_ERROR_FILE_OPERATION;
            }

            script=wxString::Format(wxT("if (= $csl_connect 1) [ sleep 1000 [ connect %s ] ]\r\n%s\r\n"),
                                    info->m_host.c_str(),wxT("csl_connect = 0"));
            break;
        }

        case CSL_GAME_AC:
        {
            cfg=path+wxString(CSL_DEFAULT_INJECT_DIR_AC)+wxString(CSL_DEFAULT_INJECT_FIL_AC);
            if (::wxFileExists(cfg))
                if (!::wxCopyFile(cfg,cfg+wxString(wxT(".csl"))))
                    return CSL_ERROR_FILE_OPERATION;

            script=wxString::Format(wxT("sleep 1000 [ %s %s %s ]\r\n"),
                                    admin ? wxT("connectadmin") : wxT("connect"),
                                    info->m_host.c_str(),password.c_str());
            break;
        }

        case CSL_GAME_CB:
        {
            char *buf;
            wxFile file;
            bool autoexec=true;
            wxString s=path+wxString(wxT("autoexec.cfg"));

            // check autoexec.cfg
            if (!::wxFileExists(s))
            {
                if (!file.Open(s,wxFile::write))
                    return CSL_ERROR_FILE_OPERATION;
                file.Close();
            }
            if (!file.Open(s,wxFile::read_write))
                return CSL_ERROR_FILE_OPERATION;

            wxUint32 size=file.Length();

            if (size)
            {
                buf=new char[size+1];
                buf[size]=0;

                if (file.Read((void*)buf,size)!=(wxInt32)size)
                {
                    delete[] buf;
                    file.Close();
                    return CSL_ERROR_FILE_OPERATION;
                }

                if (strstr(buf,"alias csl_connect 1"))
                    autoexec=false;

                delete[] buf;
            }

            if (autoexec)
            {
                if (!file.Write(wxT("\r\nalias csl_connect 1\r\n")))
                {
                    file.Close();
                    return CSL_ERROR_FILE_OPERATION;
                }
            }
            file.Close();

            // make a backup of the map config
            cfg=path+wxString(CSL_DEFAULT_INJECT_DIR_CB)+wxString(CSL_DEFAULT_INJECT_FIL_CB);
            if (::wxFileExists(cfg))
            {
                if (!::wxCopyFile(cfg,cfg+wxString(wxT(".csl"))))
                    return CSL_ERROR_FILE_OPERATION;
            }

            if (!password.IsEmpty())
                script=wxT("/password ")+password;
            script+=wxString::Format(wxT("\r\nif (= $csl_connect 1) [ sleep 1000 [ connect %s ] ]\r\n%s\r\n"),
                                     info->m_host.c_str(),wxT("alias csl_connect 0"));
            break;
        }

        default:
        {
            wxASSERT_MSG(false,wxT("unknown game"));
            return CSL_ERROR_GAME_UNKNOWN;
        }
    }

    out=cfg;
    return ConnectWriteConfig(info->m_type,cfg,script);
}
