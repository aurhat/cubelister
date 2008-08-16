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
#include "CslGameAssaultCube.h"
#include "CslTools.h"

#include "../img/ac_24.xpm"
#include "../img/ac_16.xpm"

CslGameAssaultCube::CslGameAssaultCube()
{
    m_name=CSL_DEFAULT_NAME_AC;
    m_defaultMasterConnection=CslMasterConnection(CSL_DEFAULT_MASTER_AC,CSL_DEFAULT_MASTER_PATH_AC);
    m_capabilities=CSL_CAPABILITY_EXTINFO | CSL_CAPABILITY_CUSTOM_CONFIG |
                   CSL_CAPABILITY_CONNECT_PASS | CSL_CAPABILITY_CONNECT_ADMIN_PASS;
#ifdef __WXGTK__
    m_clientSettings.ConfigPath=::wxGetHomeDir()+wxT("/.assaultcube");
#endif
#ifdef __WXMAC__
    m_configType=CSL_CONFIG_DIR;
#endif
}

CslGameAssaultCube::~CslGameAssaultCube()
{
}

wxString CslGameAssaultCube::GetVersionName(wxInt32 n) const
{
    static const wxChar* versions[] =
    {
        wxT("1.0"),wxT("0.93.x"),wxT("0.92"),wxT("0.91.x"),wxT("0.90")
    };
    wxUint32 v=CSL_LAST_PROTOCOL_AC-n;
    return (v>=0 && v<sizeof(versions)/sizeof(versions[0])) ?
           wxString(versions[v]) : wxString::Format(wxT("%d"),n);
}

wxString CslGameAssaultCube::GetModeName(wxInt32 n) const
{
    static const wxChar* modes[] =
    {
        wxT("team deathmatch"),wxT("coopedit"),wxT("deathmatch"),wxT("survivor"),
        wxT("team survivor"),wxT("ctf"),wxT("pistol frenzy"),wxT("bot team deathmatch"),
        wxT("bot deathmatch"),wxT("last swiss standing"),wxT("one shot,one kill"),
        wxT("team one shot,one kill"),wxT("bot one shot,one skill")
    };
    return (n>=0 && (size_t)n<sizeof(modes)/sizeof(modes[0])) ?
           wxString(modes[n]) : wxString(_("unknown"));
}

wxString CslGameAssaultCube::GetWeaponName(wxInt32 n) const
{
    static const wxChar* weapons[] =
    {
        wxT("Knife"),wxT("Pistol"),wxT("Shotgun"),wxT("Subgun"),
        wxT("Sniper"),wxT("Assault"),wxT("Grenade"),wxT("Pistol (auto)")
    };
    return (n>=0 && (size_t)n<sizeof(weapons)/sizeof(weapons[0])) ?
           wxString(weapons[n]) : wxString(_("unknown"));
}

bool CslGameAssaultCube::ParseDefaultPong(ucharbuf& buf,CslServerInfo& info) const
{
    char text[_MAXDEFSTR];
    wxUint32 l;

    info.Protocol=getint(buf);
    info.Version=GetVersionName(info.Protocol);
    wxInt32 mode=getint(buf);
    info.GameMode=GetModeName(mode);
    info.Players=getint(buf);
    info.TimeRemain=getint(buf);
    if (info.Protocol<1126) // <=0.93
        info.TimeRemain++;
    getstring(text,buf);
    info.Map=A2U(text);
    getstring(text,buf);
    l=(wxUint32)strlen(text);
    StripColours(text,&l,2);
    info.SetDescription(A2U(text));
    info.PlayersMax=getint(buf);
    return true;
}

bool CslGameAssaultCube::ParsePlayerPong(wxUint32 protocol,ucharbuf& buf,CslPlayerStatsData& info) const
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

    return true;
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

wxString CslGameAssaultCube::GameStart(CslServerInfo *info,wxUint32 mode,wxString *error)
{
    wxString address,password,path;
    wxString bin=m_clientSettings.Binary;
    wxString configpath=m_clientSettings.ConfigPath;
    wxString opts=m_clientSettings.Options;

    if (m_clientSettings.Binary.IsEmpty() || !::wxFileExists(m_clientSettings.Binary))
    {
        *error=_("Client binary for game AssaultCube not found!\nCheck your settings.");
        return wxEmptyString;
    }
    if (m_clientSettings.GamePath.IsEmpty() || !::wxDirExists(m_clientSettings.GamePath))
    {
        *error=_("Game path for game AssaultCube not found!\nCheck check your settings.");
        return wxEmptyString;
    }
    if (m_clientSettings.ConfigPath.IsEmpty() || !::wxDirExists(m_clientSettings.ConfigPath))
    {
        *error=_("Config path for game AssaultCube not found!\nCheck check your settings.");
        return wxEmptyString;
    }

    // use Prepend() and do not use opts+= here, since -q<path> must be before -r
    path=configpath;
#ifdef __WXMSW__
    opts.Prepend(wxString(wxT("--home=\""))+path.RemoveLast()+wxString(wxT("\" ")));
#else
    path.Replace(wxT(" "),wxT("\\ "));
    opts.Prepend(wxString(wxT("--home="))+configpath+wxString(wxT(" ")));
#endif //__WXMSW__

    address=info->Host;
    if (GetDefaultPort()!=info->Port)
        address+=m_portDelimiter+wxString::Format(wxT("%d"),info->Port);

    bin.Replace(wxT(" "),wxT("\\ "));
    bin+=wxString(wxT(" "))+opts;

    password=mode==CSL_CONNECT_PASS ? info->Password:
             mode==CSL_CONNECT_ADMIN_PASS ? info->PasswordAdmin:wxString(wxEmptyString);

    configpath+=wxString(CSL_DEFAULT_INJECT_DIR_AC);

    if (!::wxDirExists(configpath))
        if (!wxFileName::Mkdir(configpath,0700,wxPATH_MKDIR_FULL))
            return wxEmptyString;

    configpath+=wxString(CSL_DEFAULT_INJECT_FILE_AC);

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

    wxString script=wxString::Format(wxT("sleep 1000 [ %s %s %s ]\r\n"),
                                     mode==CSL_CONNECT_ADMIN_PASS ? wxT("connectadmin") : wxT("connect"),
                                     address.c_str(),password.c_str());

    return WriteTextFile(configpath,script,wxFile::write_append)==CSL_ERROR_NONE ? bin:wxString(wxEmptyString);
}

wxInt32 CslGameAssaultCube::GameEnd(wxString *error)
{
    wxString cfg=m_clientSettings.ConfigPath+wxString(CSL_DEFAULT_INJECT_DIR_AC)+wxString(CSL_DEFAULT_INJECT_FILE_AC);
    wxString bak=cfg+wxT(".csl");

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
