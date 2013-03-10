/***************************************************************************
 *   Copyright (C) 2007-2013 by Glen Masgai                                *
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

#include <Csl.h>
#include <CslEngine.h>
#include "CslGameSauerbraten.h"

#include "../img/sb_16_png.h"
#include "../img/sb_24_png.h"


enum { MM_AUTH = -1, MM_OPEN, MM_VETO, MM_LOCKED, MM_PRIVATE, MM_PASSWORD };
enum { CS_ALIVE = 0, CS_DEAD, CS_SPAWNING, CS_LAGGED, CS_EDITING, CS_SPECTATOR };
enum { PRIV_NONE = 0, PRIV_MASTER, PRIV_AUTH, PRIV_ADMIN };

static const wxChar* CLIENT_BINARY[] =
{
#if defined(__WXMSW__)
    wxT("sauerbraten.bat"), wxT("sauerbraten.exe")
#elif defined(__WXMAC__)
    wxT("sauerbraten")
#elif defined(__WXGTK__) || defined(__WXX11__)
    wxT("native_client"), wxT("linux_client"), wxT("linux_64_client"), wxT("sauer_client")
#endif //__WXMSW__
};
#define CLIENT_BINARY_LEN ((wxInt32)(sizeof(CLIENT_BINARY)/sizeof(CLIENT_BINARY[0])))

static const wxChar* CLIENT_GAME_REQUIRES[] =
{
    wxT("data"), wxT("packages")
};
#define CLIENT_GAME_REQUIRES_LEN ((wxInt32)(sizeof(CLIENT_GAME_REQUIRES)/\
                                  sizeof(CLIENT_GAME_REQUIRES[0])))

#if defined(__WXMSW__)
const wxString CLIENT_CFG_DIR = wxStandardPaths().GetDocumentsDir() + wxT("My Games");
#elif defined(__WXMAC__)
const wxString CLIENT_CFG_DIR = ::wxGetHomeDir() + wxT("/Library/Application Support/sauerbraten")
#elif defined(__WXGTK__) || defined(__WXX11__)
const wxString CLIENT_CFG_DIR = ::wxGetHomeDir() + wxT("/.sauerbraten");
#endif //__WXMSW__


CslGameSauerbraten::CslGameSauerbraten()
{
    m_name = CSL_DEFAULT_NAME_SB;

    m_fourcc=CSL_BUILD_FOURCC(CSL_FOURCC_SB);

    m_capabilities = CSL_CAPABILITY_EXTINFO | CSL_CAPABILITY_CUSTOM_CONFIG |
                     CSL_CAPABILITY_CONNECT_PASS | CSL_CAPABILITY_CONNECT_ADMIN_PASS;

    m_defaultMasterConnection = CslMasterConnection(CSL_DEFAULT_MASTER_SB,
                                                    CSL_DEFAULT_MASTER_PORT_SB);

    AddIcon(CSL_BITMAP_TYPE_PNG, 16, sb_16_png, sizeof(sb_16_png));
    AddIcon(CSL_BITMAP_TYPE_PNG, 24, sb_24_png, sizeof(sb_24_png));
}

CslGameSauerbraten::~CslGameSauerbraten()
{
}

inline wxString CslGameSauerbraten::GetVersionName(wxInt32 prot) const
{
    static const wxChar* versions[] =
    {
        wxT("Collect"), wxT("Justice"), wxT("Trooper"), wxT("CTF"),
        wxT("Assassin"),wxT("Summer"), wxT("Spring"), wxT("Gui"),
        wxT("Water"), wxT("Normalmap"), wxT("Sp"), wxT("Occlusion"),
        wxT("Shader"), wxT("Physics"), wxT("Mp"), NULL, wxT("Agc"),
        wxT("Quakecon"), wxT("Independence")
    };

    static wxInt32 count=sizeof(versions)/sizeof(versions[0]);

    wxInt32 v=CSL_LAST_PROTOCOL_SB-prot;

    if (v<0 || v>=count || !versions[v])
        return wxString::Format(wxT("%d"), prot);
    else
        return versions[v];
}

inline const wxChar* CslGameSauerbraten::GetModeName(wxInt32 mode, wxInt32 prot) const
{
    if (prot<257)
    {
        static const wxChar* modes[] =
        {
            wxT("ffa/default"), wxT("coopedit"), wxT("ffa/duel"), wxT("teamplay"),
            wxT("instagib"), wxT("instagib team"), wxT("efficiency"), wxT("efficiency team"),
            wxT("insta arena"), wxT("insta clan arena"), wxT("tactics arena"), wxT("tactics clan arena"),
            wxT("capture"), wxT("insta capture"), wxT("regen capture"), wxT("assassin"),
            wxT("insta assassin"), wxT("ctf"), wxT("insta ctf")
        };
        return (mode>=0 && (size_t)mode<sizeof(modes)/sizeof(modes[0])) ?
               modes[mode] : T2C(_("unknown"));
    }
    else
    {
        static const wxChar* modes[] =
        {
            wxT("ffa"), wxT("coop edit"), wxT("teamplay"), wxT("instagib"), wxT("insta team"),
            wxT("efficiency"), wxT("effic team"), wxT("tactics"), wxT("tac team"),
            wxT("capture"), wxT("regen capture"), wxT("ctf"), wxT("insta ctf"),
            wxT("protect"), wxT("insta protect"), wxT("hold"), wxT("insta hold"),
            wxT("effic ctf"), wxT("effic protect"), wxT("effic hold"),
            wxT("collect"), wxT("insta collect"), wxT("effic collect")
        };
        return (mode>=0 && (size_t)mode<sizeof(modes)/sizeof(modes[0])) ?
               modes[mode] : T2C(_("unknown"));
    }

    return _("unknown");
}

inline const wxChar* CslGameSauerbraten::GetWeaponName(wxInt32 n, wxInt32 prot) const
{
    if (prot<257)
    {
        static const wxChar* weapons[] =
        {
            _("Fist"), _("Shotgun"), _("Chaingun"), _("Rocketlauncher"),
            _("Rifle"), _("Grenadelauncher"), _("Pistol"), _("Fireball"),
            _("Iceball"), _("Slimeball"), _("Bite")
        };
        return (n>=0 && (size_t)n<sizeof(weapons)/sizeof(weapons[0])) ?
               weapons[n] : T2C(_("unknown"));
    }
    else
    {
        static const wxChar* weapons[] =
        {
            _("Chainsaw"), _("Shotgun"), _("Chaingun"), _("Rocketlauncher"),
            _("Rifle"), _("Grenadelauncher"), _("Pistol"), _("Fireball"),
            _("Iceball"), _("Slimeball"), _("Bite")
        };
        return (n>=0 && (size_t)n<sizeof(weapons)/sizeof(weapons[0])) ?
               weapons[n] : T2C(_("unknown"));
    }

    return _("unknown");
}

inline wxInt32 CslGameSauerbraten::GetPrivileges(wxInt32 n, wxInt32 prot) const
{
    if (prot<259 && n>PRIV_MASTER)
        n++;

    return n>CSL_PLAYER_PRIV_NONE && n<CSL_PLAYER_PRIV_MAX ? n : CSL_PLAYER_PRIV_UNKNOWN;
}

inline bool CslGameSauerbraten::ModeHasFlags(wxInt32 mode, wxInt32 prot) const
{
    if (prot<257)
        return mode>16 && mode<19;
    else
        return mode>10 && mode<23;

    return false;
}

inline bool CslGameSauerbraten::ModeHasBases(wxInt32 mode, wxInt32 prot) const
{
    if (prot<257)
        return mode>11 && mode<15;
    else
        return mode>8 && mode<11;

    return false;
}

inline wxInt32 CslGameSauerbraten::ModeScoreLimit(wxInt32 mode, wxInt32 prot) const
{
    if (ModeHasBases(mode, prot))
        return 10000;
    if (ModeHasFlags(mode, prot))
        return 10;

    return -1;
}

inline wxInt32 CslGameSauerbraten::GetBestTeam(CslTeamStats& stats, wxInt32 prot) const
{
    wxInt32 i, best=-1;

    if (stats.TeamMode && stats.m_stats.size()>0 && stats.m_stats[0]->Ok)
    {
        best=0;

        for (i=1; i<(wxInt32)stats.m_stats.size() && stats.m_stats[i]->Ok; i++)
        {
            if (stats.m_stats[i]->Score>=stats.m_stats[best]->Score)
                best=i;
        }
    }

    return best;
}

bool CslGameSauerbraten::PingDefault(ucharbuf& buf, CslServerInfo& info) const
{
    putint(buf, m_fourcc);
    return true;
}

bool CslGameSauerbraten::ParseDefaultPong(ucharbuf& buf, CslServerInfo& info) const
{
    wxArrayInt attr;
    wxInt32 l, numattr;
    char text[MAXSTRLEN];
    bool wasfull=info.IsFull();

    if ((wxUint32)getint(buf)!=m_fourcc)
        return false;

    l=getint(buf);
    if (info.HasRegisteredEvent(CslServerEvents::EVENT_EMPTY) && info.Players>0 && !l)
        info.SetEvents(CslServerEvents::EVENT_EMPTY);
    else if (info.HasRegisteredEvent(CslServerEvents::EVENT_NOT_EMPTY) && !info.Players && l>0)
        info.SetEvents(CslServerEvents::EVENT_NOT_EMPTY);
    info.Players=l;

    numattr=getint(buf);
    loopj(numattr)
    {
        if (buf.overread())
            return false;

        attr.push_back(getint(buf));
    }

    if (numattr>=1)
    {
        info.Protocol=attr[0];
        info.Version=GetVersionName(info.Protocol);
    }
    if (numattr>=2)
        info.GameMode=GetModeName(attr[1], info.Protocol);
    if (numattr>=3)
    {
        info.TimeRemain=max(0, attr[2]);
        if (info.Protocol<254)
            info.TimeRemain++;
        if (info.Protocol<258)
            info.TimeRemain*=60;
    }
    if (numattr>=4)
    {
        info.PlayersMax=attr[3];

        if (info.HasRegisteredEvent(CslServerEvents::EVENT_FULL) && !wasfull && info.IsFull())
            info.SetEvents(CslServerEvents::EVENT_FULL);
        else if (info.HasRegisteredEvent(CslServerEvents::EVENT_NOT_FULL) && wasfull && !info.IsFull())
            info.SetEvents(CslServerEvents::EVENT_NOT_FULL);
    }

    l=info.MM;
    info.MM=CSL_SERVER_OPEN;
    info.MMDescription.Empty();

    if (numattr>=5)
    {
        if (attr[4]==MM_AUTH)
        {
            info.MMDescription<<wxT("0 (O/AUTH)");
            info.MM=MM_AUTH;
        }
        else if (attr[4]==MM_PASSWORD)
        {
            info.MMDescription<<wxT("PASS");
            CSL_FLAG_SET(info.MM, CSL_SERVER_PASSWORD);
        }
        else
        {
            info.MMDescription=wxString::Format(wxT("%d"), attr[4]);

            if (attr[4]==MM_OPEN)
                info.MMDescription<<wxT(" (O)");
            else if (attr[4]==MM_VETO)
            {
                info.MMDescription<<wxT(" (V)");
                info.MM=CSL_SERVER_VETO;
            }
            else if (attr[4]==MM_LOCKED)
            {
                if (info.HasRegisteredEvent(CslServerEvents::EVENT_LOCKED) &&
                    CSL_MM_IS_VALID(l) && !CSL_SERVER_IS_LOCKED(l))
                    info.SetEvents(CslServerEvents::EVENT_LOCKED);
                info.MMDescription<<wxT(" (L)");
                info.MM=CSL_SERVER_LOCKED;

            }
            else if (attr[4]==MM_PRIVATE)
            {
                if (info.HasRegisteredEvent(CslServerEvents::EVENT_PRIVATE) &&
                        CSL_MM_IS_VALID(l) && !CSL_SERVER_IS_PRIVATE(l))
                    info.SetEvents(CslServerEvents::EVENT_PRIVATE);
                info.MMDescription<<wxT(" (P)");
                info.MM=CSL_SERVER_PRIVATE;
            }
        }
    }

    getstring(text, buf);
    info.Map=CslCharEncoding::CubeMB2WX(FilterCubeString(text, 1));
    getstring(text, buf);
    info.SetDescription(CslCharEncoding::CubeMB2WX(FilterCubeString(text, 1)));

    return !buf.overread();
}

bool CslGameSauerbraten::ParsePlayerPong(wxInt32 protocol, ucharbuf& buf, CslPlayerStatsData& info) const
{
    char text[MAXSTRLEN];

    info.ID=getint(buf);
    if (protocol>=104)
        info.Ping=getint(buf);
    getstring(text, buf);
    info.Name=CslCharEncoding::CubeMB2WX(FilterCubeString(text, 1));
    getstring(text, buf);
    info.Team=CslCharEncoding::CubeMB2WX(FilterCubeString(text, 1));
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
        case CS_SPECTATOR: info.State=CSL_PLAYER_STATE_SPECTATOR; break;
        default:           info.State=CSL_PLAYER_STATE_UNKNOWN; break;
    }

    return !buf.overread();
}

bool CslGameSauerbraten::ParseTeamPong(wxInt32 protocol, ucharbuf& buf, CslTeamStatsData& info) const
{
    wxInt32 l;
    char text[MAXSTRLEN];

    getstring(text, buf);
    info.Name=CslCharEncoding::CubeMB2WX(FilterCubeString(text, 1));
    info.Score=getint(buf);
    l=getint(buf);
    while (l-->0)
        info.Bases.push_back(getint(buf));

    return !buf.overread();
}

bool GetClientPath(wxInt32 type, const wxString& path,
                   const wxArrayString& ressources,
                   wxArrayString& result, wxString& error)
{
    wxDir dir;

    if (!wxDirExists(path) || !dir.Open(path))
    {
        error = wxString::Format(_("Couldn't open directory '%s'."), path.c_str());
        return false;
    }

    if (type==CSL_GAME_GET_PATH_BINARY)
    {
        loopv(ressources)
            FindFiles(path, ressources[i], result);

        if (result.IsEmpty())
        {
            result = ressources;
            wxString::Format(_("Couldn't find any client binary."));
            return false;
        }
    }
    else if (type==CSL_GAME_GET_PATH_GAME)
    {
        loopv(ressources)
        {
            if (!dir.HasSubDirs(ressources[i]))
                result.Add(ressources[i]);
        }

        if (result.GetCount())
        {
            error = wxString::Format(_("Couldn't find all necessary game folders."));
            return false;
        }
    }
    else if (type==CSL_GAME_GET_PATH_CONFIG)
    {
        result.Add(CLIENT_CFG_DIR);

        loopv(ressources)
        {
            if (dir.HasSubDirs(ressources[i]))
                result.Add(ressources[i]);
        }

        if (result.IsEmpty())
        {
            error = wxString::Format(_("Couldn't find an game config path."));
            return false;
        }
    }
    else
    {
        error = _("Invalid type");
        return false;
    }

    return true;
}


CslGameClientSettings CslGameSauerbraten::GuessClientSettings(const wxString& path) const
{
    return CslGameClientSettings();
}

wxString CslGameSauerbraten::ValidateClientSettings(CslGameClientSettings& settings) const
{

    if (!::wxFileExists(settings.Binary))
    {
        return _("Client binary for game Sauerbraten not found!\n");
    }
    if (settings.GamePath.IsEmpty() || !::wxDirExists(settings.GamePath))
    {
        return _("Game path for game Sauerbraten not found!\n");
    }
    if (settings.ConfigPath.IsEmpty() || !::wxDirExists(settings.ConfigPath))
    {
        return _("Config path for game Sauerbraten not found!\n");
    }

    return wxEmptyString;
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
    if (set.Options.IsEmpty())
        set.Options=wxT("-r");
#endif
    if (set.Binary.IsEmpty() || !::wxFileExists(set.Binary))
        return;

    m_clientSettings=set;
}

wxString CslGameSauerbraten::GameStart(CslServerInfo *info, wxInt32 mode, wxString& error)
{
    if (!(error = ValidateClientSettings(m_clientSettings)).IsEmpty())
        return error;

    wxString address, password, path, script;
    wxString bin=m_clientSettings.Binary;
    wxString opts=m_clientSettings.Options;
    wxString preScript=m_clientSettings.PreScript;
    wxString postScript=m_clientSettings.PostScript;

    path=m_clientSettings.ConfigPath;
#ifdef __WXMSW__
    //binary must be surrounded by quotes if the path contains spaces
    bin=wxT("\"")+m_clientSettings.Binary+wxT("\"");
    // use Prepend() and do not use opts+= here, since -q<path> must be before -r
    opts.Prepend(wxT("\"-q")+path.RemoveLast()+wxT("\" "));
#else
    CmdlineEscapeSpaces(bin);
    CmdlineEscapeSpaces(path);
    // use Prepend() and do not use opts+= here, since -q<path> must be before -r
    opts.Prepend(wxT("-q")+path+wxT(" "));
#endif //__WXMSW__

    ProcessScript(*info, mode, preScript);
    ProcessScript(*info, mode, postScript);

    if (!preScript.IsEmpty())
    {
        preScript<<wxT(";");
        CmdlineEscapeQuotes(preScript);
    }
    if (!postScript.IsEmpty())
    {
        postScript.Prepend(wxT(";"));
        CmdlineEscapeQuotes(postScript);
    }

    address=info->Host;

    password<<wxT("\"");
    password<<(mode==CslServerInfo::CSL_CONNECT_PASS ? info->Password :
               mode==CslServerInfo::CSL_CONNECT_ADMIN_PASS ?
               info->PasswordAdmin : wxString(wxEmptyString));
    password<<wxT("\"");

    CmdlineEscapeQuotes(password);

    if (info->GamePort!=GetDefaultGamePort() || !password.IsEmpty())
        address<<wxString::Format(wxT(" %d"), info->GamePort);

    script=wxString::Format(wxT("%sconnect %s %s%s"),
                            preScript.c_str(),
                            address.c_str(), password.c_str(),
                            postScript.c_str());
#ifdef __WXMSW__
    opts<<wxT(" \"-x")<<script<<wxT("\"");
#else
    opts<<wxT(" -x")<<CmdlineEscapeSpaces(script);
#endif

    bin<<wxT(" ")<<opts;

    CSL_LOG_DEBUG("start client: %s\n", U2C(bin));

    return bin;
}

wxInt32 CslGameSauerbraten::GameEnd(wxString& error)
{
    return CSL_ERROR_NONE;
}


bool CslGameSauerbraten::GetMapImagePaths(wxArrayString& paths) const
{
    wxInt32 pos;
    wxString path;

    if (!m_clientSettings.GamePath.IsEmpty())
    {
        path<<m_clientSettings.GamePath<<wxT("packages")<<CSL_PATHDIV_WX<<wxT("base")<<CSL_PATHDIV_WX;
        paths.Add(path);
    }

    //TODO look for extra package directories
    if ((pos=m_clientSettings.Options.Find(wxT("-k")))!=wxNOT_FOUND)
    {
    }

    return !paths.IsEmpty();
}

CslGameSauerbraten *sauerbraten = NULL;

bool plugin_init(CslPluginHost *host)
{
    CslEngine *engine = host->GetCslEngine();

    if (engine)
    {
        sauerbraten = new CslGameSauerbraten;
        return engine->AddGame(sauerbraten);
    }

    return true;
}

void plugin_deinit(CslPluginHost *host)
{
    if (sauerbraten)
    {
        CslEngine *engine = host->GetCslEngine();

        if (engine)
            engine->RemoveGame(sauerbraten);

        delete sauerbraten;
        sauerbraten = NULL;
    }
}

IMPLEMENT_PLUGIN(CSL_PLUGIN_VERSION_API, CSL_PLUGIN_TYPE_ENGINE, CSL_BUILD_FOURCC(CSL_FOURCC_SB),
                 CSL_DEFAULT_NAME_SB, CSL_VERSION_STR,
                 wxT("Glen Masgai"), wxT("mimosius@users.sourceforge.net"),
                 CSL_WEBADDR_STR, wxT("GPLv2"), wxT("Sauerbraten CSL engine plugin"),
                 &plugin_init, &plugin_deinit, NULL)
