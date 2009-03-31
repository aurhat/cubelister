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
#include "CslGameBloodFrontier.h"

#include "../img/bf_24.xpm"
#include "../img/bf_16.xpm"

enum { MM_OPEN, MM_VETO, MM_LOCKED, MM_PRIVATE, MM_PASSWORD };

enum
{
    G_DEMO = 0,
    G_LOBBY,
    G_EDITMODE,
    G_MISSION,
    G_DEATHMATCH,
    G_STF,
    G_CTF,
    G_MAX
};

enum
{
    G_M_NONE  = 0,
    G_M_MULTI = 1<<0,
    G_M_TEAM  = 1<<1,
    G_M_INSTA = 1<<2,
    G_M_DUEL  = 1<<3,
    G_M_LMS   = 1<<4,
    G_M_PAINT = 1<<5,
    G_M_DM    = G_M_INSTA|G_M_PAINT,
    G_M_TEAMS = G_M_MULTI|G_M_TEAM|G_M_INSTA|G_M_PAINT,
    G_M_ALL   = G_M_MULTI|G_M_TEAM|G_M_INSTA|G_M_PAINT|G_M_DUEL|G_M_LMS,
};

#define G_M_NUM  6


static struct
{
    int type,mutators,implied; const wxChar *name;
}
gametype[] =
{
    { G_DEMO,       G_M_NONE,  G_M_NONE,    wxT("Demo")    },
    { G_LOBBY,      G_M_NONE,  G_M_NONE,    wxT("Lobby")   },
    { G_EDITMODE,   G_M_NONE,  G_M_NONE,    wxT("Editing") },
    { G_MISSION,    G_M_NONE,  G_M_NONE,    wxT("Mission") },
    { G_DEATHMATCH, G_M_ALL,   G_M_NONE,    wxT("DM")      },
    { G_STF,        G_M_TEAMS, G_M_TEAM,    wxT("STF")     },
    { G_CTF,        G_M_TEAMS, G_M_TEAM,    wxT("CTF")     },
},
mutstype[] =
{
    { G_M_MULTI, G_M_ALL,         G_M_TEAM|G_M_MULTI, wxT("MS")        },
    { G_M_TEAM,  G_M_TEAMS,       G_M_TEAM,           wxT("Team")      },
    { G_M_INSTA, G_M_ALL,         G_M_INSTA,          wxT("Instagib")  },
    { G_M_DUEL,  G_M_DM|G_M_DUEL, G_M_DUEL,           wxT("Duel")      },
    { G_M_LMS,   G_M_DM|G_M_LMS,  G_M_LMS,            wxT("LMS")       },
    { G_M_PAINT, G_M_ALL,         G_M_PAINT,          wxT("Paintball") },
};


CslBloodFrontier::CslBloodFrontier()
{
    m_name=CSL_DEFAULT_NAME_BF;
    m_defaultMasterConnection=CslMasterConnection(CSL_DEFAULT_MASTER_BF,CSL_DEFAULT_MASTER_PORT_BF);
    m_capabilities=CSL_CAPABILITY_EXTINFO | CSL_CAPABILITY_CUSTOM_CONFIG | CSL_CAPABILITY_CONNECT_PASS;
#ifdef __WXMAC__
    m_clientSettings.ConfigPath=::wxGetHomeDir();
    m_clientSettings.ConfigPath+=wxT("/Library/Application Support/bloodfrontier");
#elif __WXGTK__
    m_clientSettings.ConfigPath=::wxGetHomeDir()+wxT("/.bloodfrontier");
#endif
}

CslBloodFrontier::~CslBloodFrontier()
{
}

wxString CslBloodFrontier::GetModeName(wxInt32 n,wxInt32 m) const
{
    if (n<0 || m<0 || (size_t)n>=sizeof(gametype)/sizeof(gametype[0]))
        return wxString(wxT("unknown"));

    wxString mode=gametype[n].name;

    if (gametype[n].mutators && m)
    {
        mode+=wxT(" (");
        wxString addition;
        loopi(G_M_NUM)
        {
            if ((gametype[n].mutators & mutstype[i].type) &&
                (m & mutstype[i].type) &&
                !(gametype[n].implied & mutstype[i].type))
            {
                if (!addition.IsEmpty())
                    addition+=wxT("/");
                addition+=wxString(mutstype[i].name);
            }
        }
        mode+=addition+wxT(")");
    }

    return mode;
}

const wxChar* CslBloodFrontier::GetVersionName(wxInt32 prot) const
{
    static const wxChar* versions[] =
    {
        wxT("0.80")
    };

    wxInt32 v=CSL_LAST_PROTOCOL_BF-prot;

    if (v<0 || (size_t)v>=sizeof(versions)/sizeof(versions[0]))
    {
        static wxString version=wxString::Format(wxT("%d"),prot);
        return version.c_str();
    }
    else
        return versions[v];
}

const wxChar* CslBloodFrontier::GetWeaponName(wxInt32 n) const
{
    static const wxChar* weapons[] =
    {
        _("Plasma"),_("Shotgun"),_("Chaingun"),_("Flamer"),
        _("Carbine"),_("Rifle"),_("Grenade"),_("Paintgun")
    };
    return (n>=0 && (size_t)n<sizeof(weapons)/sizeof(weapons[0])) ?
           weapons[n] : T2C(_("unknown"));
}

void CslBloodFrontier::GetPlayerstatsDescriptions(vector<wxString>& desc) const
{
    desc.add(_("Player"));
    desc.add(_("Team"));
    desc.add(_("Frags"));
    desc.add(_("Deaths"));
    desc.add(_("Teamkills"));
    desc.add(_("Ping"));
    desc.add(_("Accuracy"));
    desc.add(_("Health"));
    desc.add(_("Spree"));
    desc.add(_("Weapon"));
}

wxInt32 CslBloodFrontier::GetBestTeam(CslTeamStats& stats,wxInt32 prot) const
{
    wxInt32 i,best=-1;

    if (stats.TeamMode && stats.m_stats.length()>0 && stats.m_stats[0]->Ok)
    {
        best=0;

        for (i=1;i<stats.m_stats.length() && stats.m_stats[i]->Ok;i++)
        {
            if (stats.m_stats[i]->Score>=stats.m_stats[best]->Score)
                best=i;
        }
    }

    return best;
}

bool CslBloodFrontier::ParseDefaultPong(ucharbuf& buf,CslServerInfo& info) const
{
    vector<int>attr;
    wxUint32 l,numattr;
    char text[_MAXDEFSTR];
    bool wasfull=info.IsFull();

    l=getint(buf);
    if (info.HasRegisteredEvent(CslServerEvents::EVENT_EMPTY) && info.Players>0 && !l)
        info.SetEvents(CslServerEvents::EVENT_EMPTY);
    else if (info.HasRegisteredEvent(CslServerEvents::EVENT_NOT_EMPTY) && !info.Players && l>0)
        info.SetEvents(CslServerEvents::EVENT_NOT_EMPTY);
    info.Players=l;

    numattr=getint(buf);
    loopj(numattr) attr.add(getint(buf));
    if (numattr>=1)
    {
        info.Protocol=attr[0];
        info.Version=GetVersionName(info.Protocol);
    }
    if (numattr>=3)
        info.GameMode=GetModeName(attr[1],attr[2]);
    if (numattr>=4)
        info.TimeRemain=attr[3];
    if (numattr>=5)
    {
        info.PlayersMax=attr[4];

        if (info.HasRegisteredEvent(CslServerEvents::EVENT_FULL) && !wasfull && info.IsFull())
            info.SetEvents(CslServerEvents::EVENT_FULL);
        else if (info.HasRegisteredEvent(CslServerEvents::EVENT_NOT_FULL) && wasfull && !info.IsFull())
            info.SetEvents(CslServerEvents::EVENT_NOT_FULL);
    }

    l=info.MM;
    info.MM=CSL_SERVER_OPEN;
    info.MMDescription.Empty();

    if (numattr>=6)
    {
        if (attr[4]==MM_PASSWORD)
        {
            info.MMDescription+=wxT("PASS");
            info.MM|=CSL_SERVER_PASSWORD;
        }
        else
        {
            info.MMDescription=wxString::Format(wxT("%d"),attr[5]);

            if (attr[5]==MM_OPEN)
                info.MMDescription+=wxT(" (O)");
            else if (attr[5]==MM_VETO)
            {
                info.MMDescription+=wxT(" (V)");
                info.MM=CSL_SERVER_VETO;
            }
            else if (attr[5]==MM_LOCKED)
            {
                if (info.HasRegisteredEvent(CslServerEvents::EVENT_LOCKED) &&
                    CSL_MM_IS_VALID(l) && !CSL_SERVER_IS_LOCKED(l))
                    info.SetEvents(CslServerEvents::EVENT_LOCKED);
                info.MMDescription+=wxT(" (L)");
                info.MM=CSL_SERVER_LOCKED;
            }
            else if (attr[5]==MM_PRIVATE)
            {
                if (info.HasRegisteredEvent(CslServerEvents::EVENT_PRIVATE) &&
                    CSL_MM_IS_VALID(l) && !CSL_SERVER_IS_PRIVATE(l))
                    info.SetEvents(CslServerEvents::EVENT_PRIVATE);
                info.MMDescription+=wxT(" (P)");
                info.MM=CSL_SERVER_PRIVATE;
            }
        }
    }

    getstring(text,buf);
    info.Map=A2U(text);
    getstring(text,buf);
    l=(wxInt32)strlen(text);
    FixString(text,&l,1);
    info.SetDescription(A2U(text));

    return !buf.overread();
}

bool CslBloodFrontier::ParsePlayerPong(wxUint32 protocol,ucharbuf& buf,CslPlayerStatsData& info) const
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
    info.State=getint(buf);

    return !buf.overread();
}

bool CslBloodFrontier::ParseTeamPong(wxUint32 protocol,ucharbuf& buf,CslTeamStatsData& info) const
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

    return !buf.overread();
}

void CslBloodFrontier::SetClientSettings(const CslGameClientSettings& settings)
{
    CslGameClientSettings set=settings;

#ifdef __WXMAC__
    bool isApp=set.Binary.EndsWith(wxT(".app"));

    if (set.Binary.IsEmpty() || isApp ? !::wxDirExists(set.Binary) : !::wxFileExists(set.Binary))
        return;
    if (set.GamePath.IsEmpty() || !::wxDirExists(set.GamePath))
    {
        if (!isApp)
            return;
        set.GamePath=set.Binary+wxT("/Contents/gamedata/");
        if (!::wxDirExists(set.GamePath))
            return;
    }
    if (set.ConfigPath.IsEmpty() || !::wxDirExists(set.ConfigPath))
        set.ConfigPath=set.GamePath;
    if (isApp)
        set.Binary+=wxT("/Contents/gamedata/bloodfrontier.app/Contents/MacOS/bloodfrontier");
    if (set.Options.IsEmpty())
        set.Options=wxT("-rinit.cfg");
#else
    if (set.GamePath.IsEmpty() || !::wxDirExists(set.GamePath))
        return;
    if (set.ConfigPath.IsEmpty() || !::wxDirExists(set.ConfigPath))
        set.ConfigPath=set.GamePath;
#endif
    if (set.Binary.IsEmpty() || !::wxFileExists(set.Binary))
        return;

    m_clientSettings=set;
}

wxString CslBloodFrontier::GameStart(CslServerInfo *info,wxUint32 mode,wxString& error)
{
    wxString address,path;
    wxString bin=m_clientSettings.Binary;
    wxString opts=m_clientSettings.Options;

    if (m_clientSettings.Binary.IsEmpty() || !::wxFileExists(m_clientSettings.Binary))
    {
        error=_("Client binary for game Blood Frontier not found!\nCheck your settings.");
        return wxEmptyString;
    }
    if (m_clientSettings.GamePath.IsEmpty() || !::wxDirExists(m_clientSettings.GamePath))
    {
        error=_("Game path for game Blood Frontier not found!\nCheck your settings.");
        return wxEmptyString;
    }
    if (m_clientSettings.ConfigPath.IsEmpty() || !::wxDirExists(m_clientSettings.ConfigPath))
    {
        error=_("Config path for game Blood Frontier not found!\nCheck your settings.");
        return wxEmptyString;
    }

    path=m_clientSettings.ConfigPath;
#ifdef __WXMSW__
    //binary must be surrounded by quotes if the path contains spaces
    bin=wxT("\"")+m_clientSettings.Binary+wxT("\"");
    opts.Prepend(wxT("-h\"")+path.RemoveLast()+wxT("\" "));
#else
    bin.Replace(wxT(" "),wxT("\\ "));
    path.Replace(wxT(" "),wxT("\\ "));
    // use Prepend() and do not use opts+= here, since -h<path> must be before -r
    opts.Prepend(wxT("-h")+path+wxT(" "));
#endif //__WXMSW__

    address=info->Host;
    if (GetDefaultGamePort()!=info->GamePort)
        address+=wxString::Format(wxT(" %d"),info->GamePort);

#ifdef __WXMSW__
    opts+=wxT(" -x\"connect ")+address;
    if (mode==CslServerInfo::CSL_CONNECT_PASS)
        opts+=wxT(" ")+info->Password;
    opts+=wxT("\"");
#else
    address.Replace(wxT(" "),wxT("\\ "));
    opts+=wxT(" -xconnect\\ ")+address;
    if (mode==CslServerInfo::CSL_CONNECT_PASS)
        opts+=wxT("\\ ")+info->Password;
#endif

    bin+=wxT(" ")+opts;

    return bin;
}

wxInt32 CslBloodFrontier::GameEnd(wxString& error)
{
    return CSL_ERROR_NONE;
}

const char** CslBloodFrontier::GetIcon(wxInt32 size) const
{
    switch (size)
    {
        case 16:
            return bf_16_xpm;
        case 24:
            return bf_24_xpm;
        default:
            break;
    }
    return NULL;
}
