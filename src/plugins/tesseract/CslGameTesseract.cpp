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

#include <Csl.h>
#include <CslEngine.h>
#include "CslGameTesseract.h"

#include <wx/stdpaths.h>

#include "../img/tr_16_png.h"
#include "../img/tr_24_png.h"


enum { MM_AUTH = -1, MM_OPEN, MM_VETO, MM_LOCKED, MM_PRIVATE, MM_PASSWORD };
enum { CS_ALIVE = 0, CS_DEAD, CS_SPAWNING, CS_LAGGED, CS_EDITING, CS_SPECTATOR };
enum { PRIV_NONE = 0, PRIV_MASTER, PRIV_AUTH, PRIV_ADMIN };

#if 0
static const wxChar* CLIENT_BINARY[] =
{
#if defined(__WXMSW__)
    wxT("tesseract.bat"), wxT("tesseract.exe")
#elif defined(__WXMAC__)
    wxT("tesseract")
#elif defined(__WXGTK__) || defined(__WXX11__)
    wxT("native_client"), wxT("linux_client"), wxT("linux_64_client")
#endif //__WXMSW__
};
#define CLIENT_BINARY_LEN ((wxInt32)(sizeof(CLIENT_BINARY)/sizeof(CLIENT_BINARY[0])))

static const wxChar* CLIENT_GAME_REQUIRES[] =
{
    wxT("media")
};
#define CLIENT_GAME_REQUIRES_LEN ((wxInt32)(sizeof(CLIENT_GAME_REQUIRES)/\
                                  sizeof(CLIENT_GAME_REQUIRES[0])))
#endif

#if defined(__WXMSW__)
const wxString CLIENT_CFG_DIR = wxStandardPaths::Get().GetDocumentsDir() + wxT("My Games");
#elif defined(__WXMAC__)
const wxString CLIENT_CFG_DIR = ::wxGetHomeDir() + wxT("/Library/Application Support/tesseract")
#elif defined(__WXGTK__) || defined(__WXX11__)
const wxString CLIENT_CFG_DIR = ::wxGetHomeDir() + wxT("/.tesseract");
#endif //__WXMSW__


CslGameTesseract::CslGameTesseract()
{
    m_name = CSL_DEFAULT_NAME_TR;

    m_fourcc=CSL_BUILD_FOURCC(CSL_FOURCC_TR);

    m_capabilities = CSL_CAPABILITY_EXTINFO | CSL_CAPABILITY_CUSTOM_CONFIG |
                     CSL_CAPABILITY_CONNECT_PASS | CSL_CAPABILITY_CONNECT_ADMIN_PASS;

    m_defaultMasterURI = CSL_DEFAULT_MASTER_TR;

    AddIcon(CSL_BITMAP_TYPE_PNG, 16, tr_16_png, sizeof(tr_16_png));
    AddIcon(CSL_BITMAP_TYPE_PNG, 24, tr_24_png, sizeof(tr_24_png));
}

CslGameTesseract::~CslGameTesseract()
{
}

inline wxString CslGameTesseract::GetVersionName(wxInt32 prot) const
{
    static const wxChar* versions[] =
    {
        wxT("Dev")
    };

    static wxInt32 count=sizeof(versions)/sizeof(versions[0]);

    wxInt32 v=CSL_LAST_PROTOCOL_TR-prot;

    if (v<0 || v>=count || !versions[v])
        return wxString::Format(wxT("%d"), prot);
    else
        return versions[v];
}

inline const wxChar* CslGameTesseract::GetModeName(wxInt32 mode, wxInt32 prot) const
{
    static const wxChar* modes[] =
    {
        wxT("Edit"), wxT("rDM"), wxT("pDM"),
        wxT("rTDM"), wxT("pTDM"), wxT("rCTF"), wxT("pCTF")
    };

    return (mode>=0 && (size_t)mode<sizeof(modes)/sizeof(modes[0])) ?
            modes[mode] : T2C(_("unknown"));
}

const wxString& CslGameTesseract::GetWeaponName(wxInt32 n, wxInt32 prot) const
{
    static const wxString unknown(_("unknown"));

    static const wxString weapons[] =
    {
        _("railgun"), _("pulse rifle")
    };

    return (n>=0 && (size_t)n<sizeof(weapons)/sizeof(weapons[0])) ?
            weapons[n] : unknown;
}

inline wxInt32 CslGameTesseract::GetPrivileges(wxInt32 n, wxInt32 prot) const
{
    return n>CSL_PLAYER_PRIV_NONE && n<CSL_PLAYER_PRIV_MAX ? n : CSL_PLAYER_PRIV_UNKNOWN;
}

inline bool CslGameTesseract::ModeHasFlags(wxInt32 mode, wxInt32 prot) const
{
    return mode>4 && mode<7;
}

inline wxInt32 CslGameTesseract::ModeScoreLimit(wxInt32 mode, wxInt32 prot) const
{
    if (ModeHasFlags(mode, prot))
        return 10;

    return -1;
}

inline wxInt32 CslGameTesseract::GetBestTeam(CslTeamStats& stats, wxInt32 prot) const
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

bool CslGameTesseract::PingDefault(ucharbuf& buf, CslServerInfo& info) const
{
    buf.put(0xff); buf.put(0xff);
    putint(buf, m_fourcc);
    return true;
}

bool CslGameTesseract::PingEx(ucharbuf& buf, CslServerInfo& info) const
{
    buf.put(0xff); buf.put(0xff);
    putint(buf, 0);
    return true;
}

bool CslGameTesseract::ParseDefaultPong(ucharbuf& buf, CslServerInfo& info) const
{
    wxArrayInt attr;
    wxInt32 l, numattr;
    char text[MAXSTRLEN];
    bool wasfull=info.IsFull();

    if ((wxUint32)getint(buf)!=m_fourcc)
        return false;

    info.Protocol = getint(buf);
    info.Version = GetVersionName(info.Protocol);

    l = getint(buf);
    if (info.HasRegisteredEvent(CslServerEvents::EVENT_EMPTY) && info.Players>0 && !l)
        info.SetEvents(CslServerEvents::EVENT_EMPTY);
    else if (info.HasRegisteredEvent(CslServerEvents::EVENT_NOT_EMPTY) && !info.Players && l>0)
        info.SetEvents(CslServerEvents::EVENT_NOT_EMPTY);

    info.Players = l;
    info.PlayersMax = getint(buf);

    if (info.HasRegisteredEvent(CslServerEvents::EVENT_FULL) && !wasfull && info.IsFull())
        info.SetEvents(CslServerEvents::EVENT_FULL);
    else if (info.HasRegisteredEvent(CslServerEvents::EVENT_NOT_FULL) && wasfull && !info.IsFull())
        info.SetEvents(CslServerEvents::EVENT_NOT_FULL);

    numattr = getint(buf);
    loopj(numattr)
    {
        if (buf.overread())
            return false;

        attr.push_back(getint(buf));
    }

    if (numattr>0)
        info.GameMode = GetModeName(attr[0], info.Protocol);
    if (numattr>1)
        info.TimeRemain = max(0, attr[1]);

    l = info.MM;
    info.MM = CSL_SERVER_OPEN;
    info.MMDescription.Empty();

    if (numattr>2)
    {
        if (attr[2]==MM_AUTH)
        {
            info.MMDescription << wxT("0 (O/AUTH)");
            info.MM = MM_AUTH;
        }
        else if (attr[2]==MM_PASSWORD)
        {
            info.MMDescription << wxT("PASS");
            CSL_FLAG_SET(info.MM, CSL_SERVER_PASSWORD);
        }
        else
        {
            info.MMDescription=wxString::Format(wxT("%d"), attr[2]);

            if (attr[2]==MM_OPEN)
                info.MMDescription<<wxT(" (O)");
            else if (attr[2]==MM_VETO)
            {
                info.MMDescription<<wxT(" (V)");
                info.MM = CSL_SERVER_VETO;
            }
            else if (attr[2]==MM_LOCKED)
            {
                if (info.HasRegisteredEvent(CslServerEvents::EVENT_LOCKED) &&
                    CSL_MM_IS_VALID(l) && !CSL_SERVER_IS_LOCKED(l))
                    info.SetEvents(CslServerEvents::EVENT_LOCKED);
                info.MMDescription << wxT(" (L)");
                info.MM = CSL_SERVER_LOCKED;

            }
            else if (attr[2]==MM_PRIVATE)
            {
                if (info.HasRegisteredEvent(CslServerEvents::EVENT_PRIVATE) &&
                    CSL_MM_IS_VALID(l) && !CSL_SERVER_IS_PRIVATE(l))
                    info.SetEvents(CslServerEvents::EVENT_PRIVATE);
                info.MMDescription << wxT(" (P)");
                info.MM = CSL_SERVER_PRIVATE;
            }
        }
    }

    getstring(text, buf);
    info.Map = CslCharEncoding::CubeMB2WX(FilterCubeString(text, 1));
    getstring(text, buf);
    info.SetDescription(CslCharEncoding::CubeMB2WX(FilterCubeString(text, 1)));

    return !buf.overread();
}

bool CslGameTesseract::ParsePlayerPong(wxInt32 protocol, ucharbuf& buf, CslPlayerStatsData& info) const
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

bool CslGameTesseract::ParseTeamPong(wxInt32 protocol, ucharbuf& buf, CslTeamStatsData& info) const
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


CslGameClientSettings CslGameTesseract::GuessClientSettings(const wxString& path) const
{
    return CslGameClientSettings();
}

wxString CslGameTesseract::ValidateClientSettings(CslGameClientSettings& settings) const
{

    if (!::wxFileExists(settings.Binary))
    {
        return _("Client binary for game Tesseract not found!\n");
    }
    if (settings.GamePath.IsEmpty() || !::wxDirExists(settings.GamePath))
    {
        return _("Game path for game Tesseract not found!\n");
    }
    if (settings.ConfigPath.IsEmpty() || !::wxDirExists(settings.ConfigPath))
    {
        return _("Config path for game Tesseract not found!\n");
    }

    return wxEmptyString;
}

void CslGameTesseract::SetClientSettings(const CslGameClientSettings& settings)
{
    CslGameClientSettings set=settings;

    if (set.GamePath.IsEmpty() || !::wxDirExists(set.GamePath))
        return;
    if (set.ConfigPath.IsEmpty() || !::wxDirExists(set.ConfigPath))
        set.ConfigPath=set.GamePath;
#ifdef __WXMAC__
    if (set.Binary.IsEmpty())
        set.Binary=set.GamePath+wxT("tesseract.app/Contents/MacOS/tesseract");
    if (set.Options.IsEmpty())
        set.Options=wxT("-r");
#endif
    if (set.Binary.IsEmpty() || !::wxFileExists(set.Binary))
        return;

    m_clientSettings=set;
}

wxString CslGameTesseract::GameStart(CslServerInfo *info, wxInt32 mode, wxString& error)
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
    opts.Prepend(wxT("\"-u")+path.RemoveLast()+wxT("\" "));
#else
    CmdlineEscapeSpaces(bin);
    CmdlineEscapeSpaces(path);
    // use Prepend() and do not use opts+= here, since -q<path> must be before -r
    opts.Prepend(wxT("-u")+path+wxT(" "));
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

wxInt32 CslGameTesseract::GameEnd(wxString& error)
{
    return CSL_ERROR_NONE;
}


bool CslGameTesseract::GetMapImagePaths(wxArrayString& paths) const
{
    wxInt32 pos;
    wxString path, custom_path;

    if (!m_clientSettings.ConfigPath.IsEmpty())
    {
        custom_path<<m_clientSettings.ConfigPath<<wxT("media")<<CSL_PATHDIV_WX<<wxT("map")<<CSL_PATHDIV_WX;
        paths.Add(custom_path);
    }

    if (!m_clientSettings.GamePath.IsEmpty())
    {
        path<<m_clientSettings.GamePath<<wxT("media")<<CSL_PATHDIV_WX<<wxT("map")<<CSL_PATHDIV_WX;
        paths.Add(path);
    }

    //TODO look for extra package directories
    if ((pos=m_clientSettings.Options.Find(wxT("-k")))!=wxNOT_FOUND)
    {
    }

    return !paths.IsEmpty();
}

CslGameTesseract *tesseract = NULL;

bool plugin_init(CslPluginHost *host)
{
    CslEngine *engine = host->GetCslEngine();

    if (engine)
    {
        tesseract = new CslGameTesseract;
        return engine->AddGame(tesseract);
    }

    return true;
}

void plugin_deinit(CslPluginHost *host)
{
    if (tesseract)
    {
        CslEngine *engine = host->GetCslEngine();

        if (engine)
            engine->RemoveGame(tesseract);

        delete tesseract;
        tesseract = NULL;
    }
}

IMPLEMENT_PLUGIN(CSL_PLUGIN_VERSION_API, CSL_PLUGIN_TYPE_ENGINE, CSL_BUILD_FOURCC(CSL_FOURCC_TR),
                 CSL_DEFAULT_NAME_TR, CSL_VERSION_STR,
                 wxT("Glen Masgai"), wxT("mimosius@users.sourceforge.net"),
                 CSL_WEBADDR_STR, wxT("GPLv2"), wxT("Tesseract CSL engine plugin"),
                 &plugin_init, &plugin_deinit, NULL)
