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

#include <wx/filename.h>
#include "CslEngine.h"
#include "CslGameSauerbraten.h"
#include "CslTools.h"

#include "../img/sb_24.xpm"
#include "../img/sb_16.xpm"

CslGameSauerbraten::CslGameSauerbraten()
{
    m_name=CSL_DEFAULT_NAME_SB;
    m_defaultMasterConnection=CslMasterConnection(CSL_DEFAULT_MASTER_SB,CSL_DEFAULT_MASTER_PATH_SB);
    m_capabilities=CSL_CAPABILITY_EXTINFO | CSL_CAPABILITY_CUSTOM_CONFIG;
    m_portDelimiter=wxT(":");
#ifdef __WXMAC__
    m_clientSettings.ConfigPath=::wxGetHomeDir();
    m_clientSettings.ConfigPath+=wxT("/Library/Application Support/sauerbraten");
#elif __WXGTK__
    m_clientSettings.ConfigPath=::wxGetHomeDir()+wxT("/.sauerbraten");
#endif
    m_injected=false;
}

CslGameSauerbraten::~CslGameSauerbraten()
{
}

wxString CslGameSauerbraten::GetVersionName(wxInt32 n) const
{
    static const wxChar* versions[] =
    {
        wxT("CTF"),wxT("Assassin"),wxT("Summer"),wxT("Spring"),
        wxT("Gui"),wxT("Water"),wxT("Normalmap"),wxT("Sp"),
        wxT("Occlusion"),wxT("Shader"),wxT("Physics"),wxT("Mp"),
        wxT(""),wxT("Agc"),wxT("Quakecon"),wxT("Independence")
    };
    wxUint32 v=CSL_LAST_PROTOCOL_SB-n;
    return (v>=0 && v<sizeof(versions)/sizeof(versions[0])) ?
           wxString(versions[v]) : wxString::Format(wxT("%d"),n);
}

wxString CslGameSauerbraten::GetModeName(wxInt32 n) const
{
    static const wxChar* modes[] =
    {
        wxT("ffa/default"),wxT("coopedit"),wxT("ffa/duel"),wxT("teamplay"),
        wxT("instagib"),wxT("instagib team"),wxT("efficiency"),wxT("efficiency team"),
        wxT("insta arena"),wxT("insta clan arena"),wxT("tactics arena"),wxT("tactics clan arena"),
        wxT("capture"),wxT("insta capture"),wxT("regen capture"),wxT("assassin"),
        wxT("insta assassin"),wxT("ctf"),wxT("insta ctf")
    };
    return (n>=0 && (size_t)n<sizeof(modes)/sizeof(modes[0])) ?
           wxString(modes[n]) : wxString(_("unknown"));
}

wxString CslGameSauerbraten::GetWeaponName(wxInt32 n) const
{
    static const wxChar* weapons[] =
    {
        wxT("Fist"),wxT("Shotgun"),wxT("Chaingun"),wxT("Rocketlauncher"),
        wxT("Rifle"),wxT("Grenadelauncher"),wxT("Pistol"),wxT("Fireball"),
        wxT("Iceball"),wxT("Slimeball"),wxT("Bite")
    };
    return (n>=0 && (size_t)n<sizeof(weapons)/sizeof(weapons[0])) ?
           wxString(weapons[n]) : wxString(_("unknown"));
}

wxInt32 CslGameSauerbraten::ModeScoreLimit(wxInt32 mode) const
{
    if (ModeHasBases(mode))
        return 10000;
    if (ModeIsCapture(mode))
        return 10;
    return -1;
}

bool CslGameSauerbraten::ParseDefaultPong(ucharbuf& buf,CslServerInfo& info) const
{
    char text[_MAXDEFSTR];
    wxUint32 l;
    vector<int>attr;
    attr.setsize(0);

    info.Players=getint(buf);
    int numattr=getint(buf);
    loopj(numattr) attr.add(getint(buf));
    if (numattr>=1)
    {
        info.Protocol=attr[0];
        info.Version=GetVersionName(info.Protocol);
    }
    if (numattr>=2)
        info.GameMode=GetModeName(attr[1]);
    if (numattr>=3)
    {
        info.TimeRemain=attr[2];
        if (info.Protocol<254)
            info.TimeRemain++;
    }
    if (numattr>=4)
        info.PlayersMax=attr[3];
    if (numattr>=5)
        info.MM=attr[4];

    getstring(text,buf);
    info.Map=A2U(text);
    getstring(text,buf);
    l=(wxUint32)strlen(text);
    StripColours(text,&l,2);
    info.SetDescription(A2U(text));

    return true;
}

bool CslGameSauerbraten::ParsePlayerPong(wxUint32 protocol,ucharbuf& buf,CslPlayerStatsData& info) const
{
    char text[_MAXDEFSTR];

    info.ID=getint(buf);
    getstring(text,buf);
    info.Name=A2U(text);
    getstring(text,buf);
    info.Team=A2U(text);
    info.Frags=getint(buf);
    info.Deaths=getint(buf);
    info.Teamkills=getint(buf);
    if (protocol>=103)
        info.Accuracy=getint(buf);
    info.Health=getint(buf);
    info.Armour=getint(buf);
    info.Weapon=getint(buf);
    info.Privileges=getint(buf);
    info.State=getint(buf);

    return true;
}

bool CslGameSauerbraten::ParseTeamPong(wxUint32 protocol,ucharbuf& buf,CslTeamStatsData& info) const
{
    wxInt32 i;
    char text[_MAXDEFSTR];

    getstring(text,buf);
    info.Name=A2U(text);
    info.Score=getint(buf);
    i=getint(buf);
    if (i>0)
        while (i--)
            info.Bases.add(getint(buf));

    return true;
}

void CslGameSauerbraten::SetClientSettings(const CslGameClientSettings& settings)
{
    CslGameClientSettings set=settings;

    if (set.GamePath.IsEmpty() || !::wxDirExists(set.GamePath))
        return;
    if (set.ConfigPath.IsEmpty() || !::wxDirExists(set.ConfigPath))
        set.ConfigPath=set.GamePath;
#ifdef __WXMAC__
    if (set.Binary.IsEmpty())
        set.Binary=set.GamePath+wxT("sauerbraten.app/Contents/MacOS/sauerbraten");
#endif
    if (set.Binary.IsEmpty() || !::wxFileExists(set.Binary))
        return;

    m_clientSettings=set;
}

wxString CslGameSauerbraten::GameStart(CslServerInfo *info,wxUint32 mode,wxString *error)
{
    wxString address,path;
    wxString bin=m_clientSettings.Binary;
    wxString opts=m_clientSettings.Options;
    bool param=false;

    if (m_clientSettings.Binary.IsEmpty() || !::wxFileExists(m_clientSettings.Binary))
    {
        *error=_("Client binary for game Sauerbraten not found!\nCheck your settings.");
        return wxEmptyString;
    }
    if (m_clientSettings.GamePath.IsEmpty() || !::wxDirExists(m_clientSettings.GamePath))
    {
        *error=_("Game path for game Sauerbraten not found!\nCheck check your settings.");
        return wxEmptyString;
    }
    if (m_clientSettings.ConfigPath.IsEmpty() || !::wxDirExists(m_clientSettings.ConfigPath))
    {
        *error=_("Config path for game Sauerbraten not found!\nCheck check your settings.");
        return wxEmptyString;
    }

    // use Prepend() and do not use opts+= here, since -q<path> must be before -r
    path=m_clientSettings.ConfigPath;
#ifdef __WXMSW__
    opts.Prepend(wxString(wxT("-q\""))+path.RemoveLast()+wxString(wxT("\" ")));
#else
    path.Replace(wxT(" "),wxT("\\ "));
    opts.Prepend(wxString(wxT("-q"))+path+wxString(wxT(" ")));
#endif //__WXMSW__

    if (info->MM==MM_PRIVATE)
        param=true;
    else if (info->MM!=MM_LOCKED &&
             (info->Map.IsEmpty() ||
              info->Map.CmpNoCase(CSL_DEFAULT_INJECT_FIL_SB)==0 ||
              info->TimeRemain==0))
        param=true;

    address=info->Host;
    if (GetDefaultPort()!=info->Port)
        address+=m_portDelimiter+wxString::Format(wxT("%d"),info->Port);

    if (param)
    {
#ifdef __WXMSW__
        opts+=wxString(wxT(" -x\"connect "))+address+wxString(wxT("\""));
#else
        opts+=wxString(wxT(" -xconnect\\ "))+address;
#endif
    }
    else
    {
#ifdef __WXMSW__
        opts+=wxString(wxT(" -x\"csl_connect = 1\" -l"))+wxString(CSL_DEFAULT_INJECT_FIL_SB);
#else
        opts+=wxString(wxT(" -xcsl_connect\\ =\\ 1 -l"))+wxString(CSL_DEFAULT_INJECT_FIL_SB);
#endif
    }

    bin.Replace(wxT(" "),wxT("\\ "));
    bin+=wxString(wxT(" "))+opts;

    if (!param)
    {
        if (InjectConfig(address,error)!=CSL_ERROR_NONE)
            return wxEmptyString;
        m_injected=true;
    }

    return bin;
}

wxInt32 CslGameSauerbraten::GameEnd(wxString *error)
{
    return m_injected ? InjectConfig(wxEmptyString,error):CSL_ERROR_NONE;
}

wxInt32 CslGameSauerbraten::InjectConfig(const wxString& address,wxString *error)
{
    wxString dst=m_clientSettings.ConfigPath+wxString(CSL_DEFAULT_INJECT_DIR_SB);

    if (!::wxDirExists(dst))
    {
        if (!wxFileName::Mkdir(dst,0700,wxPATH_MKDIR_FULL))
            return CSL_ERROR_FILE_OPERATION;
    }

    wxString cfg=dst+wxString(CSL_DEFAULT_INJECT_FIL_SB)+wxString(wxT(".cfg"));
    wxString map=wxString(CSL_DEFAULT_INJECT_FIL_SB)+wxString(wxT(".ogz"));
    wxString src,script;

    dst+=map;

    if (!::wxFileExists(dst))
    {
        wxString src=DATAPATH+wxString(PATHDIV)+map;
#ifdef __WXGTK__
        if (!::wxFileExists(src))
        {
            src=::g_basePath+wxT("/data/")+map;
#endif
            if (!::wxFileExists(src))
            {
                *error=wxString::Format(_("Couldn't find \"%s\""),map.c_str());
                return CSL_ERROR_FILE_DONT_EXIST;
            }
#ifdef __WXGTK__
        }
#endif
        if (!::wxCopyFile(src,dst))
            return CSL_ERROR_FILE_OPERATION;
    }

    if (!address.IsEmpty())
        script=wxString::Format(wxT("if (= $csl_connect 1) [ sleep 1000 [ connect %s ] ]\r\n%s\r\n"),
                                address.c_str(),wxT("csl_connect = 0"));

    return WriteTextFile(cfg,script,wxFile::write);
}

const char** CslGameSauerbraten::GetIcon(wxInt32 size) const
{
    switch (size)
    {
        case 16:
            return sb_16_xpm;
        case 24:
            return sb_24_xpm;
        default:
            break;
    }
    return NULL;
}
