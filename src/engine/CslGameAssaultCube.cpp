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
#include "CslGameAssaultCube.h"

#include "../img/ac_24.xpm"
#include "../img/ac_16.xpm"

enum
{
    PONGFLAG_PASSWORD   = 1<<0,
    PONGFLAG_BANNED     = 1<<1,
    PONGFLAG_BLACKLIST  = 1<<2,
    PONGFLAG_MASTERMODE = 1<<6,
    PONGFLAG_NUM        = 1<<7
};

enum { CS_ALIVE, CS_DEAD, CS_SPAWNING, CS_LAGGED, CS_EDITING, CS_SPECTATE };


CslGameAssaultCube::CslGameAssaultCube()
{
    m_name=CSL_DEFAULT_NAME_AC;
    m_defaultMasterConnection=CslMasterConnection(CSL_DEFAULT_MASTER_AC,CSL_DEFAULT_MASTER_PATH_AC);
    m_capabilities=CSL_CAPABILITY_EXTINFO | CSL_CAPABILITY_CUSTOM_CONFIG |
                   CSL_CAPABILITY_CONNECT_PASS | CSL_CAPABILITY_CONNECT_ADMIN_PASS;
#ifdef __WXGTK__
    m_clientSettings.ConfigPath=::wxGetHomeDir()+wxT("/.assaultcube_v1.0");
#endif
#ifdef __WXMAC__
    m_configType=CSL_CONFIG_DIR;
#endif
}

CslGameAssaultCube::~CslGameAssaultCube()
{
}

const wxChar* CslGameAssaultCube::GetVersionName(wxInt32 prot) const
{
    static const wxChar* versions[] =
    {
        wxT("1.0.x"),wxT("1.0beta2"),wxT("1.0beta1"),
        wxT("0.93.x"),wxT("0.92"),wxT("0.91.x"),wxT("0.90")
    };

    wxInt32 v=CSL_LAST_PROTOCOL_AC-prot;

    if (v<0 || (size_t)v>=sizeof(versions)/sizeof(versions[0]))
    {
        static wxString version=wxString::Format(wxT("%d"),prot);
        return version.c_str();
    }
    else
        return versions[v];
}

const wxChar* CslGameAssaultCube::GetModeName(wxInt32 mode) const
{
    static const wxChar* modes[] =
    {
        wxT("team deathmatch"),wxT("coopedit"),wxT("deathmatch"),wxT("survivor"),
        wxT("team survivor"),wxT("ctf"),wxT("pistol frenzy"),wxT("bot team deathmatch"),
        wxT("bot deathmatch"),wxT("last swiss standing"),wxT("one shot,one kill"),
        wxT("team one shot,one kill"),wxT("bot one shot,one kill"),
        wxT("hunt the flag"),wxT("team keep the flag"),wxT("keep the flag")
    };

    return (mode>=0 && (size_t)mode<sizeof(modes)/sizeof(modes[0])) ?
           modes[mode] : T2C(_("unknown"));
}

const wxChar* CslGameAssaultCube::GetWeaponName(wxInt32 n) const
{
    static const wxChar* weapons[] =
    {
        _("Knife"),_("Pistol"),_("Shotgun"),_("Subgun"),
        _("Sniper"),_("Assault"),_("Grenade"),_("Akimbo")
    };

    return (n>=0 && (size_t)n<sizeof(weapons)/sizeof(weapons[0])) ?
           weapons[n] : T2C(_("unknown"));
}

wxInt32 CslGameAssaultCube::GetBestTeam(CslTeamStats& stats,wxInt32 prot) const
{
    wxInt32 i,best=-1;

    if (stats.TeamMode && stats.m_stats.length()>0 && stats.m_stats[0]->Ok)
    {
        best=0;

        for (i=1;i<stats.m_stats.length() && stats.m_stats[i]->Ok;i++)
        {
            if (ModeHasFlags(stats.GameMode,prot))
            {
                if (stats.m_stats[i]->Score2>stats.m_stats[best]->Score2)
                {
                    best=i;
                    continue;
                }
                else if (stats.m_stats[i]->Score2<stats.m_stats[best]->Score2)
                    continue;
            }

            if (stats.m_stats[i]->Score>=stats.m_stats[best]->Score)
                best=i;
        }
    }

    return best;
}

bool CslGameAssaultCube::ParseDefaultPong(ucharbuf& buf,CslServerInfo& info) const
{
    wxUint32 i,l;
    char text[_MAXDEFSTR];
    bool wasfull=info.IsFull();

    info.Protocol=getint(buf);
    info.Version=GetVersionName(info.Protocol);
    wxInt32 mode=getint(buf);
    info.GameMode=GetModeName(mode);

    i=getint(buf);
    if (info.HasRegisteredEvent(CslServerEvents::EVENT_EMPTY) && info.Players>0 && !i)
        info.SetEvents(CslServerEvents::EVENT_EMPTY);
    else if (info.HasRegisteredEvent(CslServerEvents::EVENT_NOT_EMPTY) && !info.Players && i>0)
        info.SetEvents(CslServerEvents::EVENT_NOT_EMPTY);
    info.Players=i;

    info.TimeRemain=getint(buf);
    if (info.Protocol<1126) // <=0.93
        info.TimeRemain++;
    getstring(text,buf);
    info.Map=A2U(text);
    getstring(text,buf);
    i=(wxInt32)strlen(text);
    FixString(text,&i,1);
    info.SetDescription(A2U(text));

    info.PlayersMax=getint(buf);
    if (info.HasRegisteredEvent(CslServerEvents::EVENT_FULL) && !wasfull && info.IsFull())
        info.SetEvents(CslServerEvents::EVENT_FULL);
    else if (info.HasRegisteredEvent(CslServerEvents::EVENT_NOT_FULL) && wasfull && !info.IsFull())
        info.SetEvents(CslServerEvents::EVENT_NOT_FULL);

    l=info.MM;
    info.MMDescription.Empty();
    info.MM=CSL_SERVER_OPEN;

    if (info.Protocol>=1128 && buf.remaining()) // >1.0.1 (not sure if there is any prot bump for 1.0.2)
    {
        i=getint(buf);

        if (i&PONGFLAG_MASTERMODE)
        {
            if (info.HasRegisteredEvent(CslServerEvents::EVENT_PRIVATE) &&
                CSL_MM_IS_VALID(l) && !CSL_SERVER_IS_PRIVATE(l))
                info.SetEvents(CslServerEvents::EVENT_PRIVATE);
            info.MMDescription=wxT("P");
            info.MM=CSL_SERVER_PRIVATE;
        }
        else
            info.MMDescription=wxT("O");
        if (i&PONGFLAG_BANNED)
        {
            info.MMDescription<<wxT("/BAN");
            info.MM|=CSL_SERVER_BAN;
        }
        if (i&PONGFLAG_BLACKLIST)
        {
            info.MMDescription<<wxT("/BLACK");
            info.MM|=CSL_SERVER_BLACKLIST;
        }

        if (i&PONGFLAG_PASSWORD)
        {
            info.MMDescription<<wxT("/PASS");
            info.MM|=CSL_SERVER_PASSWORD;
        }
    }
    else
        info.MMDescription=_("no data");

    return !buf.overread();
}

bool CslGameAssaultCube::ParsePlayerPong(wxUint32 protocol,ucharbuf& buf,CslPlayerStatsData& info) const
{
    char text[_MAXDEFSTR];

    info.ID=getint(buf);
    if (protocol>=104)
        info.Ping=getint(buf);
    getstring(text,buf);
    info.Name=A2U(text);
    getstring(text,buf);
    info.Team=A2U(text);
    info.Frags=getint(buf);
    if (protocol>=104)
        info.Flagscore=getint(buf);
    info.Deaths=getint(buf);
    info.Teamkills=getint(buf);
    if (protocol>=103)
        info.Accuracy=getint(buf);
    info.Health=getint(buf);
    info.Armour=getint(buf);
    info.Weapon=getint(buf);
    info.Privileges=getint(buf);
    switch (getint(buf))
    {
        case CS_ALIVE:     info.State=CSL_PLAYER_STATE_ALIVE; break;
        case CS_DEAD:      info.State=CSL_PLAYER_STATE_DEAD; break;
        case CS_SPAWNING:  info.State=CSL_PLAYER_STATE_SPAWNING; break;
        case CS_LAGGED:    info.State=CSL_PLAYER_STATE_LAGGED; break;
        case CS_EDITING:   info.State=CSL_PLAYER_STATE_EDITING; break;
        case CS_SPECTATE:  info.State=CSL_PLAYER_STATE_SPECTATOR; break;
        default:           info.State=CSL_PLAYER_STATE_UNKNOWN; break;
    }

    return !buf.overread();
}

bool CslGameAssaultCube::ParseTeamPong(wxUint32 protocol,ucharbuf& buf,CslTeamStatsData& info) const
{
    wxInt32 i;
    char text[_MAXDEFSTR];

    getstring(text,buf);
    info.Name=A2U(text);
    info.Score=getint(buf);
    i=getint(buf);
    if (i>-1)
        info.Score2=i;
    else
        info.Score2=CSL_SCORE_INVALID;
    i=getint(buf);
    if (i>0)
        while (i--)
            info.Bases.add(getint(buf));

    return !buf.overread();
}

void CslGameAssaultCube::SetClientSettings(const CslGameClientSettings& settings)
{
    CslGameClientSettings set=settings;

    if (set.GamePath.IsEmpty() || !::wxDirExists(set.GamePath))
        return;
    if (set.ConfigPath.IsEmpty())
        set.ConfigPath=set.GamePath;
    if (!::wxDirExists(set.ConfigPath))
        if (!::wxDirExists(set.ConfigPath))
            if (!wxFileName::Mkdir(set.ConfigPath,0700,wxPATH_MKDIR_FULL))
                return;

#ifdef __WXMAC__
    if (set.Binary.IsEmpty())
        set.Binary=settings.GamePath+wxT("actioncube.app/Contents/MacOS/actioncube");
#endif
    if (set.Binary.IsEmpty() || !::wxFileExists(set.Binary))
        return;

    m_clientSettings=set;
}

wxString CslGameAssaultCube::GameStart(CslServerInfo *info,wxUint32 mode,wxString& error)
{
    wxString address,password,path;
    wxString bin=m_clientSettings.Binary;
    wxString configpath=m_clientSettings.ConfigPath;
    wxString opts=m_clientSettings.Options;
    wxString preScript=m_clientSettings.PreScript;
    wxString postScript=m_clientSettings.PostScript;

    if (m_clientSettings.Binary.IsEmpty() || !::wxFileExists(m_clientSettings.Binary))
    {
        error=_("Client binary for game AssaultCube not found!\nCheck your settings.");
        return wxEmptyString;
    }
    if (m_clientSettings.GamePath.IsEmpty() || !::wxDirExists(m_clientSettings.GamePath))
    {
        error=_("Game path for game AssaultCube not found!\nCheck your settings.");
        return wxEmptyString;
    }
    if (m_clientSettings.ConfigPath.IsEmpty() || !::wxDirExists(m_clientSettings.ConfigPath))
    {
        error=_("Config path for game AssaultCube not found!\nCheck your settings.");
        return wxEmptyString;
    }

    path=configpath;
#ifdef __WXMSW__
    //binary must be surrounded by quotes if the path contains spaces
    bin=wxT("\"")+m_clientSettings.Binary+wxT("\"");
    // use Prepend() and do not use opts+= here, since --home=<path> must be before --init
    opts.Prepend(wxT("--home=\"")+path.RemoveLast()+wxT("\" "));
#else
    CmdlineEscapeSpaces(bin);
    CmdlineEscapeSpaces(path);
    // use Prepend() and do not use opts+= here, since --home=<path> must be before --init
    opts.Prepend(wxT("--home=")+configpath+wxT(" "));
#endif //__WXMSW__

    bin<<wxT(" ")+opts;

    password=mode==CslServerInfo::CSL_CONNECT_PASS ? info->Password :
             mode==CslServerInfo::CSL_CONNECT_ADMIN_PASS ?
             info->PasswordAdmin : wxString(wxEmptyString);

    address=info->Host;

    // apply port on version >=1.0.0, otherwise
    // password based connect attemps don't work
    if (info->GamePort!=GetDefaultGamePort() || (info->Protocol>=1128 && !password.IsEmpty()))
        address<<wxString::Format(wxT(" %d"),info->GamePort);

    configpath<<wxString(CSL_DEFAULT_INJECT_DIR_AC);

    if (!::wxDirExists(configpath))
        if (!wxFileName::Mkdir(configpath,0700,wxPATH_MKDIR_FULL))
            return wxEmptyString;

    configpath<<wxString(CSL_DEFAULT_INJECT_FILE_AC);

    if (::wxFileExists(configpath))
    {
        if (!::wxCopyFile(configpath,configpath+wxT(".csl")))
            return wxEmptyString;
    }
    else
    {
        wxFile file;
        file.Create(configpath+wxT(".csl"),false,wxS_IRUSR|wxS_IWUSR);
        file.Close();
    }

    ProcessScript(*info,mode,preScript);
    ProcessScript(*info,mode,postScript);

    // use saycommand [/connect ...] on version >=1.0.0,
    // otherwise password bassed connect attemps don't work
    bool say=!(password.IsEmpty() || info->Protocol<=1126);

    wxString script=wxString::Format(wxT("\r\n%s\r\n%s%s %s \"%s\"%s\r\n%s\r\n"),
                                     preScript.IsEmpty() ? wxT("") : preScript.c_str(),
                                     say ? wxT("saycommand [/") : wxT(""),
                                     mode==CslServerInfo::CSL_CONNECT_ADMIN_PASS ?
                                     wxT("connectadmin"):wxT("connect"),
                                     address.c_str(),password.c_str(),
                                     say ? wxT("]") : wxT(""),
                                     postScript.IsEmpty() ? wxT("") : postScript.c_str());

	LOG_DEBUG("start client: %s\n",U2A(bin));

    return WriteTextFile(configpath,script,wxFile::write_append)==CSL_ERROR_NONE ? bin:wxString(wxEmptyString);
}

wxInt32 CslGameAssaultCube::GameEnd(wxString& error)
{
    wxString cfg,bak;
    cfg<<m_clientSettings.ConfigPath<<CSL_DEFAULT_INJECT_DIR_AC<<wxString(CSL_DEFAULT_INJECT_FILE_AC);
    bak<<cfg<<wxT(".csl");

    if (!::wxFileExists(bak))
        return CSL_ERROR_FILE_DONT_EXIST;
    if (!::wxRenameFile(bak,cfg))
        return CSL_ERROR_FILE_OPERATION;

    return CSL_ERROR_NONE;
}

const char** CslGameAssaultCube::GetIcon(wxInt32 size) const
{
    switch (size)
    {
        case 16:
            return ac_16_xpm;
        case 24:
            return ac_24_xpm;
        default:
            break;
    }
    return NULL;
}
