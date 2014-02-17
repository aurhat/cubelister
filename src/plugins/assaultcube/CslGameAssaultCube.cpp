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
#include "CslGameAssaultCube.h"

#include "../img/ac_16_png.h"
#include "../img/ac_24_png.h"

// extra info requestable since 1.0.2, prot 1128
enum
{
    PONGFLAG_PASSWORD   = 0,
    PONGFLAG_BANNED     = 1,
    PONGFLAG_BLACKLIST,
    PONGFLAG_MASTERMODE = 6,
    PONGFLAG_NUM
};

enum
{
    AC_EXTPING_NOP = 0,
    AC_EXTPING_NAMELIST,   //since 1.0.2, prot 1128
    AC_EXTPING_SERVERINFO, //since 1.0.4, prot 1128
    AC_EXTPING_MAPROT,     //since 1.0.4, prot 1128
    AC_EXTPING_NUM
};

enum { CS_ALIVE, CS_DEAD, CS_SPAWNING, CS_LAGGED, CS_EDITING, CS_SPECTATE };


CslGameAssaultCube::CslGameAssaultCube()
{
    m_name=CSL_DEFAULT_NAME_AC;
    m_fourcc=CSL_BUILD_FOURCC(CSL_FOURCC_AC);
    m_capabilities=CSL_CAPABILITY_EXTINFO | CSL_CAPABILITY_CUSTOM_CONFIG |
                   CSL_CAPABILITY_CONNECT_PASS | CSL_CAPABILITY_CONNECT_ADMIN_PASS;
    m_defaultMasterURI = CSL_DEFAULT_MASTER_AC;
#if defined(__WXGTK__) || defined(__WXX11__)
    m_clientSettings.ConfigPath=::wxGetHomeDir()+wxT("/.assaultcube_v1.0");
#endif

    AddIcon(CSL_BITMAP_TYPE_PNG, 16, ac_16_png, sizeof(ac_16_png));
    AddIcon(CSL_BITMAP_TYPE_PNG, 24, ac_24_png, sizeof(ac_24_png));
}

CslGameAssaultCube::~CslGameAssaultCube()
{
}

inline wxString CslGameAssaultCube::GetVersionName(wxInt32 prot) const
{
    switch (prot)
    {
        case 1201: return wxT("1.2.0.x");
        case 1200: return wxT("1.2.0.0");
        case 1132: return wxT("1.1.0.4");
        case 1131: return wxT("1.1.0.2");
        case 1130: return wxT("1.1.0.1");
        case 1129: return wxT("1.1.0.0");
        case 1128: return wxT("1.1.x");
        case 1127: return wxT("1.1.0b2");
        case 1126: return wxT("1.1.0b1");
        case 1125: return wxT("0.93.x");
        case 1124: return wxT("0.92");
        case 1123: return wxT("0.91.x");
        case 1122: return wxT("0.90");
    }

    return wxString::Format(wxT("%d"), prot);
}

inline const wxChar* CslGameAssaultCube::GetModeName(wxInt32 mode) const
{
    static const wxChar* modes[] =
    {
        wxT("team deathmatch"), wxT("coopedit"), wxT("deathmatch"), wxT("survivor"),
        wxT("team survivor"), wxT("ctf"), wxT("pistol frenzy"), wxT("bot team deathmatch"),
        wxT("bot deathmatch"), wxT("last swiss standing"), wxT("one shot, one kill"),
        wxT("team one shot, one kill"), wxT("bot one shot, one kill"),
        wxT("hunt the flag"), wxT("team keep the flag"), wxT("keep the flag"),
        wxT("team pistol frenzy"), wxT("team last swiss standing"), wxT("bot pistol frenzy"),
        wxT("bot last swiss standing"), wxT("bot team survivor"), wxT("bot team one shot, one kill")
    };

    return (mode>=0 && (size_t)mode<sizeof(modes)/sizeof(modes[0])) ?
           modes[mode] : T2C(_("unknown"));
}

inline const wxString& CslGameAssaultCube::GetWeaponName(wxInt32 n, wxInt32 prot) const
{
    static const wxString unknown(_("unknown"));

    if (prot<=1128) // < 1.1.x series
    {
        static const wxString weapons[] =
        {
            _("Knife"), _("Pistol"), _("Shotgun"), _("Subgun"),
            _("Sniper"), _("Assault"), _("Grenade"), _("Akimbo")
        };

        return (n>=0 && (size_t)n<sizeof(weapons)/sizeof(weapons[0])) ?
                weapons[n] : unknown;
    }
    else if (prot<1200) // 1.1.0.x series
    {
        static const wxString weapons[] =
        {
            _("Knife"), _("Pistol"), _("Carbine"), _("Shotgun"), _("Subgun"),
            _("Sniper"), _("Assault"), _("Combat Pistol"), _("Grenade"), _("Akimbo")
        };

        return (n>=0 && (size_t)n<sizeof(weapons)/sizeof(weapons[0])) ?
                weapons[n] : unknown;
    }
    else // 1.2.0.x series
    {
        static const wxString weapons[] =
        {
            _("Knife"), _("Pistol"), _("TMP-M&A CB"), _("V19-SG"), _("A-ARD/10"),
            _("AD-81 SR"), _("MTP-57"), _("Combat Pistol"), _("Grenade"), _("Akimbo")
        };

        return (n>=0 && (size_t)n<sizeof(weapons)/sizeof(weapons[0])) ?
                weapons[n] : unknown;
    }

    return unknown;
}

inline wxInt32 CslGameAssaultCube::GetBestTeam(CslTeamStats& stats, wxInt32 prot) const
{
    wxInt32 i, best=-1;

    if (stats.TeamMode && stats.m_stats.size()>0 && stats.m_stats[0]->Ok)
    {
        best=0;

        for (i=1; i<(wxInt32)stats.m_stats.size() && stats.m_stats[i]->Ok; i++)
        {
            if (ModeHasFlags(stats.GameMode, prot))
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

bool CslGameAssaultCube::PingDefault(ucharbuf& buf, CslServerInfo& info) const
{
    putint(buf, m_fourcc);

    if (info.Protocol==-1 || info.Protocol>=1128) // >=1.0.x
    {
        if (info.InfoText.IsEmpty())
        {
            putint(buf, AC_EXTPING_SERVERINFO);
            putint(buf, 'e'); putint(buf, 'n');
            return true;
        }
    }

    putint(buf, AC_EXTPING_NOP);

    return true;
}

bool CslGameAssaultCube::ParseDefaultPong(ucharbuf& buf, CslServerInfo& info) const
{
    wxInt32 i, l, q;
    char text[MAXSTRLEN];
    bool wasfull=info.IsFull();

    if ((wxUint32)getint(buf)!=m_fourcc)
        return false;

    q=getint(buf);
    if (q==AC_EXTPING_SERVERINFO)
        loopi(2) getint(buf);

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

    info.TimeRemain=max(0, getint(buf));
    if (info.Protocol<1126) // <= 0.93
        info.TimeRemain++;
    if (info.Protocol>=1200) // <= 1.2.0.x
        info.TimeRemain*=60;
    getstring(text, buf);
    info.Map=C2U(FilterCubeString(text, 1));
    getstring(text, buf);
    info.SetDescription(C2U(FilterCubeString(text, 1)));

    info.PlayersMax=getint(buf);
    if (info.HasRegisteredEvent(CslServerEvents::EVENT_FULL) && !wasfull && info.IsFull())
        info.SetEvents(CslServerEvents::EVENT_FULL);
    else if (info.HasRegisteredEvent(CslServerEvents::EVENT_NOT_FULL) && wasfull && !info.IsFull())
        info.SetEvents(CslServerEvents::EVENT_NOT_FULL);

    l=info.MM;
    info.MMDescription.Empty();
    info.MM=CSL_SERVER_OPEN;

    if (info.Protocol>=1128 && buf.remaining()) // >=1.0.x
    {
        i = getint(buf);
        wxInt32 mm = i>>PONGFLAG_MASTERMODE;

        if (mm==1)
        {
            if (info.HasRegisteredEvent(CslServerEvents::EVENT_PRIVATE) &&
                CSL_MM_IS_VALID(l) && !CSL_SERVER_IS_PRIVATE(l))
                info.SetEvents(CslServerEvents::EVENT_PRIVATE);
            info.MMDescription=wxT("P");
            info.MM=CSL_SERVER_PRIVATE;
        }
        else if (mm==2)
        {
            if (info.HasRegisteredEvent(CslServerEvents::EVENT_LOCKED) &&
                CSL_MM_IS_VALID(l) && !CSL_SERVER_IS_LOCKED(l))
                info.SetEvents(CslServerEvents::EVENT_LOCKED);
            info.MMDescription=wxT("L");
            info.MM=CSL_SERVER_LOCKED;
        }
        else
            info.MMDescription=wxT("O");

        if (i&(1<<PONGFLAG_BANNED))
        {
            info.MMDescription<<wxT("/BAN");
            CSL_FLAG_SET(info.MM, CSL_SERVER_BAN);
        }
        if (i&(1<<PONGFLAG_BLACKLIST))
        {
            info.MMDescription<<wxT("/BLACK");
            CSL_FLAG_SET(info.MM, CSL_SERVER_BLACKLIST);
        }
        if (i&(1<<PONGFLAG_PASSWORD))
        {
            info.MMDescription<<wxT("/PASS");
            CSL_FLAG_SET(info.MM, CSL_SERVER_PASSWORD);
        }

        if (buf.remaining() && getint(buf)==q) // >=1.0.2
        {
            switch (q)
            {
                case AC_EXTPING_SERVERINFO:    // >=1.0.4
                {
                    if (getstring(text, buf)==2)
                    {
                        i=0;
                        info.InfoText.Empty();

                        while (buf.remaining())
                        {
                            getstring(text, buf);

                            if (!*text)
                                break;
                            if (i++)
                                info.InfoText<<CSL_NEWLINE_WX;
                            if (strcmp(text, "."))
                                info.InfoText<<C2U(FilterCubeString(text, 1, true, false, true));
                        }
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }
    else
        info.MMDescription=_("no data");

    return !buf.overread();
}

bool CslGameAssaultCube::ParsePlayerPong(wxInt32 protocol, ucharbuf& buf, CslPlayerStatsData& info) const
{
    char text[MAXSTRLEN];

    info.ID=getint(buf);
    if (protocol>=104)
        info.Ping=getint(buf);
    getstring(text, buf);
    info.Name=C2U(FilterCubeString(text, 1));
    getstring(text, buf);
    info.Team=C2U(FilterCubeString(text, 1));
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
    info.KpD=max(0, info.Frags)/(float)max(1, info.Deaths);
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

bool CslGameAssaultCube::ParseTeamPong(wxInt32 protocol, ucharbuf& buf, CslTeamStatsData& info) const
{
    wxInt32 l;
    char text[MAXSTRLEN];

    getstring(text, buf);
    info.Name=C2U(FilterCubeString(text, 1));
    info.Score=getint(buf);
    l=getint(buf);
    if (l>-1)
        info.Score2=l;
    else
        info.Score2=CSL_SCORE_INVALID;
    l=getint(buf);
    while (l-->0)
        info.Bases.push_back(getint(buf));

    return !buf.overread();
}

CslGameClientSettings CslGameAssaultCube::GuessClientSettings(const wxString& path) const
{
    return CslGameClientSettings();
}

wxString CslGameAssaultCube::ValidateClientSettings(CslGameClientSettings& settings) const
{
    return wxEmptyString;
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
            if (!wxFileName::Mkdir(set.ConfigPath, 0700, wxPATH_MKDIR_FULL))
                return;

#ifdef __WXMAC__
    if (set.Binary.IsEmpty())
        set.Binary=settings.GamePath+wxT("actioncube.app/Contents/MacOS/actioncube");
#endif
    if (set.Binary.IsEmpty() || !::wxFileExists(set.Binary))
        return;

    m_clientSettings=set;
}

wxString CslGameAssaultCube::GameStart(CslServerInfo *info, wxInt32 mode, wxString& error)
{
    wxString address, password, path;
    wxString bin=m_clientSettings.Binary;
    wxString configpath=m_clientSettings.ConfigPath;
    wxString opts=m_clientSettings.Options;
    wxString preScript=m_clientSettings.PreScript;
    wxString postScript=m_clientSettings.PostScript;

    if (m_clientSettings.Binary.IsEmpty() || !::wxFileExists(m_clientSettings.Binary))
    {
        error=_("Client binary for game AssaultCube not found!\n");
        error<<_("Please check your settings.");
        return wxEmptyString;
    }
    if (m_clientSettings.GamePath.IsEmpty() || !::wxDirExists(m_clientSettings.GamePath))
    {
        error=_("Game path for game AssaultCube not found!\n");
        error<<_("Please check your settings.");
        return wxEmptyString;
    }
    if (m_clientSettings.ConfigPath.IsEmpty() || !::wxDirExists(m_clientSettings.ConfigPath))
    {
        error=_("Config path for game AssaultCube not found!\n");
        error<<_("Please check your settings.");
        return wxEmptyString;
    }

    path=configpath;
#ifdef __WXMSW__
    //binary must be surrounded by quotes if the path contains spaces
    bin=wxT("\"")+m_clientSettings.Binary+wxT("\"");
    // use Prepend() and do not use opts+= here, since --home=<path> must be before --init
    opts.Prepend(wxT("\"--home=")+path.RemoveLast()+wxT("\" "));
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
        address<<wxString::Format(wxT(" %d"), info->GamePort);

    configpath<<wxString(CSL_DEFAULT_INJECT_DIR_AC);

    if (!::wxDirExists(configpath))
        if (!wxFileName::Mkdir(configpath, 0700, wxPATH_MKDIR_FULL))
            return wxEmptyString;

    configpath<<wxString(CSL_DEFAULT_INJECT_FILE_AC);

    if (::wxFileExists(configpath))
    {
        if (!::wxCopyFile(configpath, configpath+wxT(".csl")))
            return wxEmptyString;
    }
    else
    {
        wxFile file;
        file.Create(configpath+wxT(".csl"), false, wxS_IRUSR|wxS_IWUSR);
        file.Close();
    }

    ProcessScript(*info, mode, preScript);
    ProcessScript(*info, mode, postScript);

    // use saycommand [/connect ...] on version >=1.0.0,
    // otherwise password bassed connect attemps don't work
    bool say=!(password.IsEmpty() || info->Protocol<=1126);

    wxString script=wxString::Format(wxT("\r\n%s\r\n%s%s %s \"%s\"%s\r\n%s\r\n"),
                                     preScript.IsEmpty() ? wxEmptyString : preScript.c_str(),
                                     say ? wxT("saycommand [/") : wxEmptyString,
                                     mode==CslServerInfo::CSL_CONNECT_ADMIN_PASS ?
                                     wxT("connectadmin"):wxT("connect"),
                                     address.c_str(), password.c_str(),
                                     say ? wxT("]") : wxEmptyString,
                                     postScript.IsEmpty() ? wxEmptyString : postScript.c_str());

    CSL_LOG_DEBUG("start client: %s\n", U2C(bin));

    return WriteTextFile(configpath, script, wxFile::write_append)==CSL_ERROR_NONE ? bin:wxString(wxEmptyString);
}

wxInt32 CslGameAssaultCube::GameEnd(wxString& error)
{
    wxString cfg, bak;
    cfg<<m_clientSettings.ConfigPath<<CSL_DEFAULT_INJECT_DIR_AC<<wxString(CSL_DEFAULT_INJECT_FILE_AC);
    bak<<cfg<<wxT(".csl");

    if (!::wxFileExists(bak))
        return CSL_ERROR_FILE_DONT_EXIST;
    if (!::wxRenameFile(bak, cfg))
        return CSL_ERROR_FILE_OPERATION;

    return CSL_ERROR_NONE;
}


CslGameAssaultCube *assaultcube = NULL;

bool plugin_init(CslPluginHost *host)
{
    CslEngine *engine = host->GetCslEngine();

    if (engine)
    {
        assaultcube = new CslGameAssaultCube;
        return engine->AddGame(assaultcube);
    }

    return true;
}

void plugin_deinit(CslPluginHost *host)
{
    if (assaultcube)
    {
        CslEngine *engine = host->GetCslEngine();

        if (engine)
            engine->RemoveGame(assaultcube);

        delete assaultcube;
        assaultcube = NULL;
    }
}

IMPLEMENT_PLUGIN(CSL_PLUGIN_VERSION_API, CSL_PLUGIN_TYPE_ENGINE, CSL_BUILD_FOURCC(CSL_FOURCC_AC),
                 CSL_DEFAULT_NAME_AC, CSL_VERSION_STR,
                 wxT("Glen Masgai"), wxT("mimosius@users.sourceforge.net"),
                 CSL_WEBADDR_STR, wxT("GPLv2"), wxT("Assaultcube CSL engine plugin"),
                 &plugin_init, &plugin_deinit, NULL)
