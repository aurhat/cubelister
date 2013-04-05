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

#include "Csl.h"
#include "CslEngine.h"
#include "CslGameRedEclipse.h"

#include "../img/re_16_png.h"
#include "../img/re_24_png.h"

enum { MM_OPEN, MM_VETO, MM_LOCKED, MM_PRIVATE, MM_PASSWORD };
enum { CS_ALIVE, CS_DEAD, CS_SPAWNING, CS_EDITING, CS_SPECTATOR, CS_WAITING };

enum
{
    G_DEMO        = 0,
    G_EDITMODE,
    G_CAMPAIGN,
    G_DEATHMATCH,
    G_CAPTURE,
    G_DEFEND,
    G_BOMBER,
    G_TRIAL,
    G_MAX,
    G_START       = G_EDITMODE,
    G_PLAY        = G_CAMPAIGN,
    G_FIGHT       = G_DEATHMATCH,
    G_RAND        = G_BOMBER-G_DEATHMATCH+1
};

enum
{
    G_M_NONE      = 0,
    G_M_TEAM      = 1<<0,
    G_M_INSTA     = 1<<1,
    G_M_MEDIEVAL  = 1<<2,
    G_M_BALLISTIC = 1<<3,
    G_M_DUEL      = 1<<4,
    G_M_SURVIVOR  = 1<<5,
    G_M_ARENA     = 1<<6,
    G_M_ONSLAUGHT = 1<<7,
    G_M_JETPACK   = 1<<8,
    G_M_VAMPIRE   = 1<<9,
    G_M_EXPERT    = 1<<10,
    G_M_RESIZE    = 1<<11,
    G_M_GSP1      = 1<<12,
    G_M_GSP2      = 1<<13,
    G_M_GSP3      = 1<<14,
    G_M_ALL       = G_M_TEAM|G_M_INSTA|G_M_MEDIEVAL|G_M_BALLISTIC|G_M_DUEL|G_M_SURVIVOR|G_M_ARENA|
                    G_M_ONSLAUGHT|G_M_JETPACK|G_M_VAMPIRE|G_M_EXPERT|G_M_RESIZE|G_M_GSP1|G_M_GSP2|G_M_GSP3,
    G_M_FILTER    = G_M_TEAM|G_M_INSTA|G_M_MEDIEVAL|G_M_BALLISTIC|G_M_DUEL|G_M_SURVIVOR|G_M_ARENA|
                    G_M_ONSLAUGHT|G_M_JETPACK|G_M_VAMPIRE|G_M_RESIZE|G_M_GSP1|G_M_GSP2|G_M_GSP3,
    G_M_GSN       = 3,
    G_M_GSP       = 12,
    G_M_NUM       = 15
};

static const struct
{
    int type, implied, mutators[G_M_GSN+1];
    const wxChar *name, *gsp[G_M_GSN];
}
gametype[] =
{
    {
        G_DEMO, G_M_NONE,
        {
            G_M_NONE, G_M_NONE, G_M_NONE, G_M_NONE
        },
        wxT("demo"), { wxT(""), wxT(""), wxT("") },
    },
    {
        G_EDITMODE, G_M_NONE,
        {
            G_M_TEAM|G_M_INSTA|G_M_ARENA|G_M_MEDIEVAL|G_M_BALLISTIC|G_M_JETPACK|G_M_VAMPIRE|G_M_EXPERT|G_M_RESIZE,
            G_M_NONE, G_M_NONE, G_M_NONE
        },
        wxT("editing"), { wxT(""), wxT(""), wxT("") },
    },
    {
        G_CAMPAIGN, G_M_TEAM,
        {
            G_M_TEAM|G_M_INSTA|G_M_ARENA|G_M_MEDIEVAL|G_M_BALLISTIC|G_M_JETPACK|G_M_VAMPIRE|G_M_EXPERT|G_M_RESIZE,
            G_M_NONE, G_M_NONE, G_M_NONE
        },
        wxT("campaign"), { wxT(""), wxT(""), wxT("") },
    },
    {
        G_DEATHMATCH, G_M_NONE,
        {
            G_M_TEAM|G_M_INSTA|G_M_DUEL|G_M_SURVIVOR|G_M_ARENA|G_M_MEDIEVAL|G_M_BALLISTIC|G_M_ONSLAUGHT|G_M_JETPACK|G_M_VAMPIRE|G_M_EXPERT|G_M_RESIZE,
            G_M_NONE, G_M_NONE, G_M_NONE
        },
        wxT("deathmatch"), { wxT(""), wxT(""), wxT("") }
    },
    {
        G_CAPTURE, G_M_TEAM,
        {
            G_M_TEAM|G_M_INSTA|G_M_DUEL|G_M_SURVIVOR|G_M_ARENA|G_M_MEDIEVAL|G_M_BALLISTIC|G_M_ONSLAUGHT|G_M_JETPACK|G_M_VAMPIRE|G_M_EXPERT|G_M_RESIZE|G_M_GSP1|G_M_GSP2|G_M_GSP3,
            G_M_TEAM|G_M_INSTA|G_M_DUEL|G_M_SURVIVOR|G_M_ARENA|G_M_MEDIEVAL|G_M_BALLISTIC|G_M_ONSLAUGHT|G_M_JETPACK|G_M_VAMPIRE|G_M_EXPERT|G_M_RESIZE|G_M_GSP1,
            G_M_TEAM|G_M_INSTA|G_M_DUEL|G_M_SURVIVOR|G_M_ARENA|G_M_MEDIEVAL|G_M_BALLISTIC|G_M_ONSLAUGHT|G_M_JETPACK|G_M_VAMPIRE|G_M_EXPERT|G_M_RESIZE|G_M_GSP2,
            G_M_TEAM|G_M_INSTA|G_M_DUEL|G_M_SURVIVOR|G_M_ARENA|G_M_MEDIEVAL|G_M_BALLISTIC|G_M_ONSLAUGHT|G_M_JETPACK|G_M_VAMPIRE|G_M_EXPERT|G_M_RESIZE|G_M_GSP3
        },
        wxT("ctf"), { wxT("return"), wxT("defend"), wxT("protect") }
    },
    {
        G_DEFEND, G_M_TEAM,
        {
            G_M_TEAM|G_M_INSTA|G_M_ARENA|G_M_MEDIEVAL|G_M_BALLISTIC|G_M_ONSLAUGHT|G_M_JETPACK|G_M_VAMPIRE|G_M_EXPERT|G_M_RESIZE|G_M_GSP1|G_M_GSP2,
            G_M_TEAM|G_M_INSTA|G_M_ARENA|G_M_MEDIEVAL|G_M_BALLISTIC|G_M_ONSLAUGHT|G_M_JETPACK|G_M_VAMPIRE|G_M_EXPERT|G_M_RESIZE|G_M_GSP1,
            G_M_TEAM|G_M_INSTA|G_M_ARENA|G_M_MEDIEVAL|G_M_BALLISTIC|G_M_ONSLAUGHT|G_M_JETPACK|G_M_VAMPIRE|G_M_EXPERT|G_M_RESIZE|G_M_GSP2,
            G_M_NONE
        },
        wxT("dtf"), { wxT("quick"), wxT("conquer"), wxT("") }
    },
    {
        G_BOMBER, G_M_NONE,
        {
            G_M_TEAM|G_M_INSTA|G_M_DUEL|G_M_SURVIVOR|G_M_ARENA|G_M_MEDIEVAL|G_M_BALLISTIC|G_M_ONSLAUGHT|G_M_JETPACK|G_M_VAMPIRE|G_M_EXPERT|G_M_RESIZE|G_M_GSP1|G_M_GSP2,
            G_M_TEAM|G_M_INSTA|G_M_DUEL|G_M_SURVIVOR|G_M_ARENA|G_M_MEDIEVAL|G_M_BALLISTIC|G_M_ONSLAUGHT|G_M_JETPACK|G_M_VAMPIRE|G_M_EXPERT|G_M_RESIZE|G_M_GSP1|G_M_GSP2,
            G_M_TEAM|G_M_INSTA|G_M_DUEL|G_M_SURVIVOR|G_M_ARENA|G_M_MEDIEVAL|G_M_BALLISTIC|G_M_ONSLAUGHT|G_M_JETPACK|G_M_VAMPIRE|G_M_EXPERT|G_M_RESIZE|G_M_GSP1|G_M_GSP2,
            G_M_NONE
        },
        wxT("bomber-ball"), { wxT("multi"), wxT("hold"), wxT("") },
    },
    {
        G_TRIAL, G_M_NONE,
        {
            G_M_TEAM|G_M_INSTA|G_M_ARENA|G_M_MEDIEVAL|G_M_BALLISTIC|G_M_ONSLAUGHT|G_M_JETPACK|G_M_VAMPIRE|G_M_EXPERT|G_M_RESIZE,
            G_M_NONE, G_M_NONE, G_M_NONE
        },
        wxT("time-trial"), { wxT(""), wxT(""), wxT("") },
    },
};

static const struct
{
    int type, implied, mutators;
    const wxChar *name;
}
mutstype[] =
{
    {
        G_M_TEAM, G_M_TEAM, G_M_TEAM|G_M_INSTA|G_M_DUEL|G_M_SURVIVOR|G_M_ARENA|G_M_MEDIEVAL|G_M_BALLISTIC|G_M_ONSLAUGHT|G_M_JETPACK|G_M_VAMPIRE|G_M_EXPERT|G_M_RESIZE|G_M_GSP1|G_M_GSP2|G_M_GSP3,
        wxT("team")
    },
    {
        G_M_INSTA, G_M_INSTA, G_M_TEAM|G_M_INSTA|G_M_DUEL|G_M_SURVIVOR|G_M_ARENA|G_M_MEDIEVAL|G_M_BALLISTIC|G_M_ONSLAUGHT|G_M_JETPACK|G_M_VAMPIRE|G_M_EXPERT|G_M_RESIZE|G_M_GSP1|G_M_GSP2|G_M_GSP3,
        wxT("instagib")
    },
    {
        G_M_MEDIEVAL, G_M_MEDIEVAL, G_M_TEAM|G_M_INSTA|G_M_DUEL|G_M_SURVIVOR|G_M_MEDIEVAL|G_M_ONSLAUGHT|G_M_JETPACK|G_M_VAMPIRE|G_M_EXPERT|G_M_RESIZE|G_M_GSP1|G_M_GSP2|G_M_GSP3,
        wxT("medieval")
    },
    {
        G_M_BALLISTIC, G_M_BALLISTIC, G_M_TEAM|G_M_INSTA|G_M_DUEL|G_M_SURVIVOR|G_M_BALLISTIC|G_M_ONSLAUGHT|G_M_JETPACK|G_M_VAMPIRE|G_M_EXPERT|G_M_RESIZE|G_M_GSP1|G_M_GSP2|G_M_GSP3,
        wxT("ballistic")
    },
    {
        G_M_DUEL, G_M_DUEL, G_M_TEAM|G_M_INSTA|G_M_DUEL|G_M_ARENA|G_M_MEDIEVAL|G_M_BALLISTIC|G_M_ONSLAUGHT|G_M_JETPACK|G_M_VAMPIRE|G_M_EXPERT|G_M_RESIZE|G_M_GSP1|G_M_GSP2|G_M_GSP3,
        wxT("duel")
    },
    {
        G_M_SURVIVOR, G_M_SURVIVOR, G_M_TEAM|G_M_INSTA|G_M_SURVIVOR|G_M_ARENA|G_M_MEDIEVAL|G_M_BALLISTIC|G_M_ONSLAUGHT|G_M_JETPACK|G_M_VAMPIRE|G_M_EXPERT|G_M_RESIZE|G_M_GSP1|G_M_GSP2|G_M_GSP3,
        wxT("survivor")
    },
    {
        G_M_ARENA, G_M_ARENA, G_M_TEAM|G_M_INSTA|G_M_DUEL|G_M_SURVIVOR|G_M_ARENA|G_M_ONSLAUGHT|G_M_JETPACK|G_M_VAMPIRE|G_M_EXPERT|G_M_RESIZE|G_M_GSP1|G_M_GSP2|G_M_GSP3,
        wxT("arena")
    },
    {
        G_M_ONSLAUGHT, G_M_ONSLAUGHT, G_M_TEAM|G_M_INSTA|G_M_MEDIEVAL|G_M_BALLISTIC|G_M_DUEL|G_M_SURVIVOR|G_M_ARENA|G_M_ONSLAUGHT|G_M_JETPACK|G_M_VAMPIRE|G_M_EXPERT|G_M_RESIZE|G_M_GSP1|G_M_GSP2|G_M_GSP3,
        wxT("onslaught")
    },
    {
        G_M_JETPACK, G_M_JETPACK, G_M_TEAM|G_M_INSTA|G_M_MEDIEVAL|G_M_BALLISTIC|G_M_DUEL|G_M_SURVIVOR|G_M_ARENA|G_M_ONSLAUGHT|G_M_JETPACK|G_M_VAMPIRE|G_M_EXPERT|G_M_RESIZE|G_M_GSP1|G_M_GSP2|G_M_GSP3,
        wxT("jetpack")
    },
    {
        G_M_VAMPIRE, G_M_VAMPIRE, G_M_TEAM|G_M_INSTA|G_M_MEDIEVAL|G_M_BALLISTIC|G_M_DUEL|G_M_SURVIVOR|G_M_ARENA|G_M_ONSLAUGHT|G_M_JETPACK|G_M_VAMPIRE|G_M_EXPERT|G_M_RESIZE|G_M_GSP1|G_M_GSP2|G_M_GSP3,
        wxT("vampire")
    },
    {
        G_M_EXPERT, G_M_EXPERT, G_M_TEAM|G_M_INSTA|G_M_MEDIEVAL|G_M_BALLISTIC|G_M_DUEL|G_M_SURVIVOR|G_M_ARENA|G_M_ONSLAUGHT|G_M_JETPACK|G_M_VAMPIRE|G_M_EXPERT|G_M_RESIZE|G_M_GSP1|G_M_GSP2|G_M_GSP3,
        wxT("expert")
    },
    {
        G_M_RESIZE, G_M_RESIZE, G_M_TEAM|G_M_INSTA|G_M_MEDIEVAL|G_M_BALLISTIC|G_M_DUEL|G_M_SURVIVOR|G_M_ARENA|G_M_ONSLAUGHT|G_M_JETPACK|G_M_VAMPIRE|G_M_EXPERT|G_M_RESIZE|G_M_GSP1|G_M_GSP2|G_M_GSP3,
        wxT("resize")
    },
    {
        G_M_GSP1, G_M_GSP1, G_M_TEAM|G_M_INSTA|G_M_MEDIEVAL|G_M_BALLISTIC|G_M_DUEL|G_M_SURVIVOR|G_M_ARENA|G_M_ONSLAUGHT|G_M_JETPACK|G_M_VAMPIRE|G_M_EXPERT|G_M_RESIZE|G_M_GSP1|G_M_GSP2|G_M_GSP3,
        wxT("")
    },
    {
        G_M_GSP2, G_M_GSP2, G_M_TEAM|G_M_INSTA|G_M_MEDIEVAL|G_M_BALLISTIC|G_M_DUEL|G_M_SURVIVOR|G_M_ARENA|G_M_ONSLAUGHT|G_M_JETPACK|G_M_VAMPIRE|G_M_EXPERT|G_M_RESIZE|G_M_GSP1|G_M_GSP2|G_M_GSP3,
        wxT("")
    },
    {
        G_M_GSP3, G_M_GSP3, G_M_TEAM|G_M_INSTA|G_M_MEDIEVAL|G_M_BALLISTIC|G_M_DUEL|G_M_SURVIVOR|G_M_ARENA|G_M_ONSLAUGHT|G_M_JETPACK|G_M_VAMPIRE|G_M_EXPERT|G_M_RESIZE|G_M_GSP1|G_M_GSP2|G_M_GSP3,
        wxT("")
    },
};

#define m_game(a)      (a > -1 && a < G_MAX)
#define m_implied(a,b) (gametype[a].implied|(a == G_BOMBER && !((b|gametype[a].implied) & G_M_GSP2) ? G_M_TEAM : G_M_NONE))


CslGameRedEclipse::CslGameRedEclipse()
{
    m_name=CSL_DEFAULT_NAME_RE;
    m_fourcc=CSL_BUILD_FOURCC(CSL_FOURCC_RE);
    m_capabilities=CSL_CAPABILITY_EXTINFO | CSL_CAPABILITY_CUSTOM_CONFIG |
                   CSL_CAPABILITY_CONNECT_PASS | CSL_CAPABILITY_CONNECT_ADMIN_PASS;
    m_defaultMasterConnection=CslMasterConnection(CSL_DEFAULT_MASTER_RE, CSL_DEFAULT_MASTER_PORT_RE);
#ifdef __WXMAC__
    m_clientSettings.ConfigPath=::wxGetHomeDir();
    m_clientSettings.ConfigPath<<wxT("/Library/Application Support/redeclipse");
#elif defined(__WXGTK__) || defined(__WXX11__)
    m_clientSettings.ConfigPath=::wxGetHomeDir()+wxT("/.redeclipse");
#endif

    AddIcon(CSL_BITMAP_TYPE_PNG, 16, re_16_png, sizeof(re_16_png));
    AddIcon(CSL_BITMAP_TYPE_PNG, 24, re_24_png, sizeof(re_24_png));
}

CslGameRedEclipse::~CslGameRedEclipse()
{
}

wxString CslGameRedEclipse::GetModeName(wxInt32 mode, wxInt32 muts) const
{
    if (!m_game(mode))
        return _("unknown");

    wxString name=gametype[mode].name;

    if (gametype[mode].mutators[0] && muts)
    {
        const wxChar *mut;
        wxString addition;

        loopi(G_M_NUM)
        {
            wxInt32 implied=m_implied(mode, muts);

            if ((gametype[mode].mutators[0]&mutstype[i].type) &&
                (muts&mutstype[i].type) &&
                (!implied || !(implied&mutstype[i].type)))
            {
                mut=i<G_M_GSP ? mutstype[i].name : gametype[mode].gsp[i-G_M_GSP];

                if (mut[0]=='\0')
                    continue;

                if (!addition.IsEmpty())
                    addition<<wxT("/");

                addition<<mut;
            }
        }

        if (!addition.IsEmpty())
            name<<wxT(" (")<<addition<<wxT(")");

    }

    return name;
}

inline wxString CslGameRedEclipse::GetVersionName(wxInt32 prot) const
{
    static const wxChar* versions[] =
    {
        wxT("Cosmic"), NULL, NULL, wxT("Supernova"), wxT("Ides")
    };

    static wxInt32 count=sizeof(versions)/sizeof(versions[0]);

    wxInt32 v=CSL_LAST_PROTOCOL_RE-prot;

    if (v<0 || v>=count || !versions[v])
        return wxString::Format(wxT("%d"), prot);
    else
        return versions[v];
}

inline const wxString& CslGameRedEclipse::GetWeaponName(wxInt32 n, wxInt32 prot) const
{
    static const wxString unknown(_("unknown"));

    static const wxString weapons[] =
    {
        _("Melee"), _("Pistol"), _("Sword"), _("Shotgun"),
        _("SMG"), _("Flamer"), _("Plasma"), _("Rifle"),
        _("Grenade"), _("Rocket")
    };
    return (n>=0 && (size_t)n<sizeof(weapons)/sizeof(weapons[0])) ? weapons[n] : unknown;
}

wxInt32 CslGameRedEclipse::GetPlayerstatsDescriptions(const wxString **desc) const
{
    static const wxString descriptions[] =
    {
        _("Player"), _("Team"), _("Frags"), _("Deaths"), _("Teamkills"),
        _("Ping"), _("KpD"), _("Accuracy"), _("Health"), _("Spree"), _("Weapon")
    };

    *desc = descriptions;

    return sizeof(descriptions)/sizeof(descriptions[0]);
}

inline wxInt32 CslGameRedEclipse::GetBestTeam(CslTeamStats& stats, wxInt32 prot) const
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

bool CslGameRedEclipse::PingDefault(ucharbuf& buf, CslServerInfo& info) const
{
    putint(buf, m_fourcc);
    return true;
}

bool CslGameRedEclipse::ParseDefaultPong(ucharbuf& buf, CslServerInfo& info) const
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
    if (numattr>=3)
        info.GameMode=GetModeName(attr[1], attr[2]);
    if (numattr>=4)
        info.TimeRemain=max(0, attr[3]);
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
        if (attr[5]==MM_PASSWORD)
        {
            info.MMDescription<<wxT("PASS");
            CSL_FLAG_SET(info.MM, CSL_SERVER_PASSWORD);
        }
        else
        {
            info.MMDescription=wxString::Format(wxT("%d"), attr[5]);

            if (attr[5]==MM_OPEN)
                info.MMDescription<<wxT(" (O)");
            else if (attr[5]==MM_VETO)
            {
                info.MMDescription<<wxT(" (V)");
                info.MM=CSL_SERVER_VETO;
            }
            else if (attr[5]==MM_LOCKED)
            {
                if (info.HasRegisteredEvent(CslServerEvents::EVENT_LOCKED) &&
                    CSL_MM_IS_VALID(l) && !CSL_SERVER_IS_LOCKED(l))
                    info.SetEvents(CslServerEvents::EVENT_LOCKED);
                info.MMDescription<<wxT(" (L)");
                info.MM=CSL_SERVER_LOCKED;
            }
            else if (attr[5]==MM_PRIVATE)
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
    info.Map = CslCharEncoding::CubeMB2WX(FilterCubeString(text, 1));
    getstring(text, buf);
    info.SetDescription(CslCharEncoding::CubeMB2WX(FilterCubeString(text, 1)));

    return !buf.overread();
}

bool CslGameRedEclipse::ParsePlayerPong(wxInt32 protocol, ucharbuf& buf, CslPlayerStatsData& info) const
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
        case CS_EDITING:   info.State=CSL_PLAYER_STATE_EDITING; break;
        case CS_SPECTATOR: info.State=CSL_PLAYER_STATE_SPECTATOR; break;
        case CS_WAITING:   info.State=CSL_PLAYER_STATE_WAITING; break;
        default:           info.State=CSL_PLAYER_STATE_UNKNOWN; break;
    }

    return !buf.overread();
}

bool CslGameRedEclipse::ParseTeamPong(wxInt32 protocol, ucharbuf& buf, CslTeamStatsData& info) const
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

CslGameClientSettings CslGameRedEclipse::GuessClientSettings(const wxString& path) const
{
    return CslGameClientSettings();
}

wxString CslGameRedEclipse::ValidateClientSettings(CslGameClientSettings& settings) const
{
    return wxEmptyString;
}

void CslGameRedEclipse::SetClientSettings(const CslGameClientSettings& settings)
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
        set.Binary<<wxT("/Contents/gamedata/redeclipse.app/Contents/MacOS/redeclipse");
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

wxString CslGameRedEclipse::GameStart(CslServerInfo *info, wxInt32 mode, wxString& error)
{
    wxString address, password, path, script;
    wxString bin=m_clientSettings.Binary;
    wxString opts=m_clientSettings.Options;
    wxString preScript=m_clientSettings.PreScript;
    wxString postScript=m_clientSettings.PostScript;

    if (m_clientSettings.Binary.IsEmpty() || !::wxFileExists(m_clientSettings.Binary))
    {
        error=_("Client binary for game Red Eclipse not found!");
        error<<_("Please check your settings.");
        return wxEmptyString;
    }
    if (m_clientSettings.GamePath.IsEmpty() || !::wxDirExists(m_clientSettings.GamePath))
    {
        error=_("Game path for game Red Eclipse not found!");
        error<<_("Please check your settings.");
        return wxEmptyString;
    }
    if (m_clientSettings.ConfigPath.IsEmpty() || !::wxDirExists(m_clientSettings.ConfigPath))
    {
        error=_("Config path for game Red Eclipse not found!");
        error<<_("Please check your settings.");
        return wxEmptyString;
    }

    path=m_clientSettings.ConfigPath;
#ifdef __WXMSW__
    //binary must be surrounded by quotes if the path contains spaces
    bin=wxT("\"")+m_clientSettings.Binary+wxT("\"");
    // use Prepend() and do not use opts+= here, since -h<path> must be before -r
    opts.Prepend(wxT("\"-h")+path.RemoveLast()+wxT("\" "));
#else
    CmdlineEscapeSpaces(bin);
    CmdlineEscapeSpaces(path);
    // use Prepend() and do not use opts+= here, since -h<path> must be before -r
    opts.Prepend(wxT("-h")+path+wxT(" "));
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

    if (GetDefaultGamePort()!=info->GamePort || mode==CslServerInfo::CSL_CONNECT_PASS)
        address<<wxString::Format(wxT(" %d %d"), info->GamePort, info->Address().GetPort());

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

wxInt32 CslGameRedEclipse::GameEnd(wxString& error)
{
    return CSL_ERROR_NONE;
}


CslGameRedEclipse *redeclipse = NULL;

bool plugin_init(CslPluginHost *host)
{
    CslEngine *engine = host->GetCslEngine();

    if (engine)
    {
        redeclipse = new CslGameRedEclipse;
        return engine->AddGame(redeclipse);
    }

    return true;
}

void plugin_deinit(CslPluginHost *host)
{
    if (redeclipse)
    {
        CslEngine *engine = host->GetCslEngine();

        if (engine)
            engine->RemoveGame(redeclipse);

        delete redeclipse;
        redeclipse = NULL;
    }
}

IMPLEMENT_PLUGIN(CSL_PLUGIN_VERSION_API, CSL_PLUGIN_TYPE_ENGINE, CSL_BUILD_FOURCC(CSL_FOURCC_RE),
                 CSL_DEFAULT_NAME_RE, CSL_VERSION_STR,
                 wxT("Glen Masgai"), wxT("mimosius@users.sourceforge.net"),
                 CSL_WEBADDR_STR, wxT("GPLv2"), wxT("Red Eclipse CSL engine plugin"),
                 &plugin_init, &plugin_deinit, NULL)
