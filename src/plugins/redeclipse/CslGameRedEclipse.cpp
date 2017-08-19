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
#include "CslGameRedEclipse.h"

#include "../img/re_16_png.h"
#include "../img/re_24_png.h"

enum { MM_OPEN, MM_VETO, MM_LOCKED, MM_PRIVATE, MM_PASSWORD };
enum { CS_ALIVE, CS_DEAD, CS_SPAWNING, CS_EDITING, CS_SPECTATOR, CS_WAITING };

enum
{
    G_DEMO = 0, G_EDITMODE, G_DEATHMATCH, G_CAPTURE, G_DEFEND, G_BOMBER, G_TRIAL, G_GAUNTLET, G_MAX,
    G_START = G_EDITMODE, G_PLAY = G_DEATHMATCH, G_FIGHT = G_DEATHMATCH,
    G_RAND = G_BOMBER-G_DEATHMATCH+1, G_COUNT = G_MAX-G_PLAY,
    G_NEVER = (1<<G_DEMO)|(1<<G_EDITMODE)|(1<<G_GAUNTLET),
    G_LIMIT = (1<<G_DEATHMATCH)|(1<<G_CAPTURE)|(1<<G_DEFEND)|(1<<G_BOMBER),
    G_ALL = (1<<G_DEMO)|(1<<G_EDITMODE)|(1<<G_DEATHMATCH)|(1<<G_CAPTURE)|(1<<G_DEFEND)|(1<<G_BOMBER)|(1<<G_TRIAL)|(1<<G_GAUNTLET),
    G_SW = (1<<G_TRIAL),
};

enum
{
    G_M_MULTI = 0, G_M_FFA, G_M_COOP, G_M_INSTA, G_M_MEDIEVAL, G_M_KABOOM, G_M_DUEL, G_M_SURVIVOR,
    G_M_CLASSIC, G_M_ONSLAUGHT, G_M_JETPACK, G_M_VAMPIRE, G_M_EXPERT, G_M_RESIZE,
    G_M_GSP, G_M_GSP1 = G_M_GSP, G_M_GSP2, G_M_GSP3, G_M_NUM,
    G_M_GSN = G_M_NUM-G_M_GSP,
    G_M_ALL = (1<<G_M_MULTI)|(1<<G_M_FFA)|(1<<G_M_COOP)|(1<<G_M_INSTA)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_DUEL)|(1<<G_M_SURVIVOR)|(1<<G_M_CLASSIC)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP1)|(1<<G_M_GSP2)|(1<<G_M_GSP3),
    G_M_FILTER = (1<<G_M_MULTI)|(1<<G_M_FFA)|(1<<G_M_COOP)|(1<<G_M_INSTA)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_DUEL)|(1<<G_M_SURVIVOR)|(1<<G_M_CLASSIC)|(1<<G_M_ONSLAUGHT)|(1<<G_M_VAMPIRE)|(1<<G_M_RESIZE)|(1<<G_M_GSP1)|(1<<G_M_GSP2)|(1<<G_M_GSP3),
    G_M_ROTATE = (1<<G_M_FFA),
    G_M_SW = (1<<G_M_INSTA)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM),
    G_M_DK = (1<<G_M_DUEL)|(1<<G_M_SURVIVOR),
    G_M_IM = (1<<G_M_INSTA)|(1<<G_M_MEDIEVAL),
};

enum { G_F_GSP = 0, G_F_NUM };

struct gametypes
{
    int type, flags, implied, mutators[G_M_GSN+1];
    const wxChar *name, *sname, *gsp[G_M_GSN];
}
gametype[] = {
    {
        G_DEMO, 0, 0, { 0, 0, 0, 0 },
        wxT("demo"), wxT("demo"), { wxT(""), wxT(""), wxT("") },
    },
    {
        G_EDITMODE, 0, (1<<G_M_FFA)|(1<<G_M_CLASSIC),
        {
            (1<<G_M_FFA)|(1<<G_M_CLASSIC)|(1<<G_M_JETPACK),
            0, 0, 0
        },
        wxT("editing"), wxT("editing"), { wxT(""), wxT(""), wxT("") },
    },
    {
        G_DEATHMATCH, 0, 0,
        {
            (1<<G_M_MULTI)|(1<<G_M_FFA)|(1<<G_M_COOP)|(1<<G_M_INSTA)|(1<<G_M_DUEL)|(1<<G_M_SURVIVOR)|(1<<G_M_CLASSIC)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE),
            0, 0, 0
        },
        wxT("deathmatch"), wxT("dm"), { wxT(""), wxT(""), wxT("") }
    },
    {
        G_CAPTURE, 0, 0,
        {
            (1<<G_M_MULTI)|(1<<G_M_COOP)|(1<<G_M_INSTA)|(1<<G_M_DUEL)|(1<<G_M_SURVIVOR)|(1<<G_M_CLASSIC)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP1)|(1<<G_M_GSP2)|(1<<G_M_GSP3),
            (1<<G_M_MULTI)|(1<<G_M_COOP)|(1<<G_M_INSTA)|(1<<G_M_DUEL)|(1<<G_M_SURVIVOR)|(1<<G_M_CLASSIC)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP1),
            (1<<G_M_MULTI)|(1<<G_M_COOP)|(1<<G_M_INSTA)|(1<<G_M_DUEL)|(1<<G_M_SURVIVOR)|(1<<G_M_CLASSIC)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP2),
            (1<<G_M_MULTI)|(1<<G_M_COOP)|(1<<G_M_INSTA)|(1<<G_M_DUEL)|(1<<G_M_SURVIVOR)|(1<<G_M_CLASSIC)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP3)
        },
        wxT("capture-the-flag"), wxT("capture"), { wxT("quick"), wxT("defend"), wxT("protect") },
    },
    {
        G_DEFEND, 0, 0,
        {
            (1<<G_M_MULTI)|(1<<G_M_COOP)|(1<<G_M_INSTA)|(1<<G_M_CLASSIC)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP1)|(1<<G_M_GSP2),
            (1<<G_M_MULTI)|(1<<G_M_COOP)|(1<<G_M_INSTA)|(1<<G_M_CLASSIC)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP1)|(1<<G_M_GSP2),
            (1<<G_M_MULTI)|(1<<G_M_COOP)|(1<<G_M_INSTA)|(1<<G_M_CLASSIC)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP1)|(1<<G_M_GSP2),
            0
        },
        wxT("defend-and-control"), wxT("defend"), { wxT("quick"), wxT("king"), wxT("") },
    },
    {
        G_BOMBER, (1<<G_F_GSP), 0,
        {
            (1<<G_M_MULTI)|(1<<G_M_COOP)|(1<<G_M_INSTA)|(1<<G_M_DUEL)|(1<<G_M_SURVIVOR)|(1<<G_M_CLASSIC)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP1)|(1<<G_M_GSP2),
            (1<<G_M_MULTI)|(1<<G_M_FFA)|(1<<G_M_COOP)|(1<<G_M_INSTA)|(1<<G_M_CLASSIC)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP1),
            (1<<G_M_MULTI)|(1<<G_M_COOP)|(1<<G_M_INSTA)|(1<<G_M_DUEL)|(1<<G_M_SURVIVOR)|(1<<G_M_CLASSIC)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP2),
            0
        },
        wxT("bomber-ball"), wxT("bomber"), { wxT("hold"), wxT("touchdown"), wxT("") },
    },
    {
        G_TRIAL, 0, 0,
        {
            (1<<G_M_FFA)|(1<<G_M_INSTA)|(1<<G_M_CLASSIC)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE),
            0, 0, 0
        },
        wxT("time-trial"), wxT("trial"), { wxT(""), wxT(""), wxT("") },
    },
    {
        G_GAUNTLET, 0, 0,
        {
            (1<<G_M_INSTA)|(1<<G_M_CLASSIC)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP1)|(1<<G_M_GSP2),
            (1<<G_M_INSTA)|(1<<G_M_CLASSIC)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP1)|(1<<G_M_GSP2),
            (1<<G_M_INSTA)|(1<<G_M_CLASSIC)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP1)|(1<<G_M_GSP2),
            0
        },
        wxT("gauntlet"), wxT("gauntlet"), { wxT("timed"), wxT("hard"), wxT("") },
    },
};

static const struct
{
    int type, implied, mutators;
    const wxChar *name;
}
mutstype[] = {
    {
        G_M_MULTI, (1<<G_M_MULTI),
        (1<<G_M_MULTI)|(1<<G_M_COOP)|(1<<G_M_INSTA)|(1<<G_M_DUEL)|(1<<G_M_SURVIVOR)|(1<<G_M_CLASSIC)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP1)|(1<<G_M_GSP2)|(1<<G_M_GSP3),
        wxT("multi")
    },
    {
        G_M_FFA, (1<<G_M_FFA),
        (1<<G_M_FFA)|(1<<G_M_INSTA)|(1<<G_M_DUEL)|(1<<G_M_SURVIVOR)|(1<<G_M_CLASSIC)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP1)|(1<<G_M_GSP2)|(1<<G_M_GSP3),
        wxT("ffa")
    },
    {
        G_M_COOP, (1<<G_M_COOP),
        (1<<G_M_MULTI)|(1<<G_M_COOP)|(1<<G_M_INSTA)|(1<<G_M_DUEL)|(1<<G_M_SURVIVOR)|(1<<G_M_CLASSIC)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP1)|(1<<G_M_GSP2)|(1<<G_M_GSP3),
        wxT("coop")
    },
    {
        G_M_INSTA, (1<<G_M_INSTA),
        (1<<G_M_MULTI)|(1<<G_M_FFA)|(1<<G_M_COOP)|(1<<G_M_INSTA)|(1<<G_M_DUEL)|(1<<G_M_SURVIVOR)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP1)|(1<<G_M_GSP2)|(1<<G_M_GSP3),
        wxT("instagib")
    },
    {
        G_M_MEDIEVAL, (1<<G_M_MEDIEVAL),
        (1<<G_M_MULTI)|(1<<G_M_FFA)|(1<<G_M_COOP)|(1<<G_M_INSTA)|(1<<G_M_DUEL)|(1<<G_M_SURVIVOR)|(1<<G_M_MEDIEVAL)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP1)|(1<<G_M_GSP2)|(1<<G_M_GSP3),
        wxT("medieval")
    },
    {
        G_M_KABOOM,  (1<<G_M_KABOOM),
        (1<<G_M_MULTI)|(1<<G_M_FFA)|(1<<G_M_COOP)|(1<<G_M_INSTA)|(1<<G_M_DUEL)|(1<<G_M_SURVIVOR)|(1<<G_M_KABOOM)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP1)|(1<<G_M_GSP2)|(1<<G_M_GSP3),
        wxT("kaboom")
    },
    {
        G_M_DUEL, (1<<G_M_DUEL),
        (1<<G_M_MULTI)|(1<<G_M_FFA)|(1<<G_M_COOP)|(1<<G_M_INSTA)|(1<<G_M_DUEL)|(1<<G_M_CLASSIC)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP1)|(1<<G_M_GSP2)|(1<<G_M_GSP3),
        wxT("duel")
    },
    {
        G_M_SURVIVOR, (1<<G_M_SURVIVOR),
        (1<<G_M_MULTI)|(1<<G_M_FFA)|(1<<G_M_COOP)|(1<<G_M_INSTA)|(1<<G_M_SURVIVOR)|(1<<G_M_CLASSIC)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP1)|(1<<G_M_GSP2)|(1<<G_M_GSP3),
        wxT("survivor")
    },
    {
        G_M_CLASSIC, (1<<G_M_CLASSIC),
        (1<<G_M_MULTI)|(1<<G_M_FFA)|(1<<G_M_COOP)|(1<<G_M_DUEL)|(1<<G_M_SURVIVOR)|(1<<G_M_CLASSIC)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP1)|(1<<G_M_GSP2)|(1<<G_M_GSP3),
        wxT("classic")
    },
    {
        G_M_ONSLAUGHT, (1<<G_M_ONSLAUGHT),
        (1<<G_M_MULTI)|(1<<G_M_FFA)|(1<<G_M_COOP)|(1<<G_M_INSTA)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_DUEL)|(1<<G_M_SURVIVOR)|(1<<G_M_CLASSIC)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP1)|(1<<G_M_GSP2)|(1<<G_M_GSP3),
        wxT("onslaught")
    },
    {
        G_M_JETPACK, (1<<G_M_JETPACK),
        (1<<G_M_MULTI)|(1<<G_M_FFA)|(1<<G_M_COOP)|(1<<G_M_INSTA)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_DUEL)|(1<<G_M_SURVIVOR)|(1<<G_M_CLASSIC)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP1)|(1<<G_M_GSP2)|(1<<G_M_GSP3),
        wxT("jetpack")
    },
    {
        G_M_VAMPIRE, (1<<G_M_VAMPIRE),
        (1<<G_M_MULTI)|(1<<G_M_FFA)|(1<<G_M_COOP)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_DUEL)|(1<<G_M_SURVIVOR)|(1<<G_M_CLASSIC)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP1)|(1<<G_M_GSP2)|(1<<G_M_GSP3),
        wxT("vampire")
    },
    {
        G_M_EXPERT, (1<<G_M_EXPERT),
        (1<<G_M_MULTI)|(1<<G_M_FFA)|(1<<G_M_COOP)|(1<<G_M_INSTA)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_DUEL)|(1<<G_M_SURVIVOR)|(1<<G_M_CLASSIC)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP1)|(1<<G_M_GSP2)|(1<<G_M_GSP3),
        wxT("expert")
    },
    {
        G_M_RESIZE, (1<<G_M_RESIZE),
        (1<<G_M_MULTI)|(1<<G_M_FFA)|(1<<G_M_COOP)|(1<<G_M_INSTA)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_DUEL)|(1<<G_M_SURVIVOR)|(1<<G_M_CLASSIC)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP1)|(1<<G_M_GSP2)|(1<<G_M_GSP3),
        wxT("resize")
    },
    {
        G_M_GSP1, (1<<G_M_GSP1),
        (1<<G_M_MULTI)|(1<<G_M_FFA)|(1<<G_M_COOP)|(1<<G_M_INSTA)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_DUEL)|(1<<G_M_SURVIVOR)|(1<<G_M_CLASSIC)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP1)|(1<<G_M_GSP2)|(1<<G_M_GSP3),
        wxT("gsp1")
    },
    {
        G_M_GSP2, (1<<G_M_GSP2),
        (1<<G_M_MULTI)|(1<<G_M_FFA)|(1<<G_M_COOP)|(1<<G_M_INSTA)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_DUEL)|(1<<G_M_SURVIVOR)|(1<<G_M_CLASSIC)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP1)|(1<<G_M_GSP2)|(1<<G_M_GSP3),
        wxT("gsp2")
    },
    {
        G_M_GSP3, (1<<G_M_GSP3),
        (1<<G_M_MULTI)|(1<<G_M_FFA)|(1<<G_M_COOP)|(1<<G_M_INSTA)|(1<<G_M_MEDIEVAL)|(1<<G_M_KABOOM)|(1<<G_M_DUEL)|(1<<G_M_SURVIVOR)|(1<<G_M_CLASSIC)|(1<<G_M_ONSLAUGHT)|(1<<G_M_JETPACK)|(1<<G_M_VAMPIRE)|(1<<G_M_EXPERT)|(1<<G_M_RESIZE)|(1<<G_M_GSP1)|(1<<G_M_GSP2)|(1<<G_M_GSP3),
        wxT("gsp3")
    },
};

#define m_game(a)      (a > -1 && a < G_MAX)


CslGameRedEclipse::CslGameRedEclipse()
{
    m_name=CSL_DEFAULT_NAME_RE;
    m_fourcc=CSL_BUILD_FOURCC(CSL_FOURCC_RE);
    m_capabilities=CSL_CAPABILITY_CUSTOM_CONFIG |
                   CSL_CAPABILITY_CONNECT_PASS |
                   CSL_CAPABILITY_CONNECT_ADMIN_PASS;
    m_defaultMasterURI = CSL_DEFAULT_MASTER_RE;
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
        mode = G_DEATHMATCH;

    if (gametype[mode].implied)
        muts |= gametype[mode].implied;

    wxString name = gametype[mode].sname;

    if (gametype[mode].mutators[0] && muts)
    {
        wxString addition;
        wxInt32 implied = gametype[mode].implied;

        loopi(G_M_NUM) if(muts&(1<<mutstype[i].type)) implied |= mutstype[i].implied&~(1<<mutstype[i].type);
        loopi(G_M_NUM) if(muts&(1<<mutstype[i].type) && (!implied || !(implied&(1<<mutstype[i].type))))
        {
            const wxChar *mut = i < G_M_GSP ? mutstype[i].name : gametype[mode].gsp[i-G_M_GSP];

            if (!mut || !mut[0])
                continue;

            if (!addition.IsEmpty())
                addition << wxT("/");

            addition << mut;
        }

        if (!addition.IsEmpty())
            name<<wxT(" (")<<addition<<wxT(")");
    }

    return name;
}

inline wxString CslGameRedEclipse::GetVersionName(wxInt32 prot) const
{
    switch (prot)
    {
        case 230:
        case 229: return wxT("1.5.x Elysium");
        case 226: return wxT("1.5.x Aurora");
        case 220: return wxT("1.4 Elara");
        case 217: return wxT("1.3 Galactic");
        case 214: return wxT("1.2 Cosmic");
        case 211: return wxT("1.1 Supernova");
        case 210: return wxT("1.0 Ides");
    }

    return wxString::Format(wxT("%d"), prot);
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

CslGameClientSettings CslGameRedEclipse::GuessClientSettings(const wxString& path) const
{
    return CslGameClientSettings();
}

wxString CslGameRedEclipse::ValidateClientSettings(CslGameClientSettings& settings) const
{

    if (!settings.Binary.IsEmpty())
    {
        if (!::wxFileExists(settings.Binary))
        {
            return _("Client binary for game Red Eclipse not found!\n");
        }
        if (settings.GamePath.IsEmpty() || !::wxDirExists(settings.GamePath))
        {
            return _("Game path for game Red Eclipse not found!\n");
        }
        if (settings.ConfigPath.IsEmpty() || !::wxDirExists(settings.ConfigPath))
        {
            return _("Config path for game Red Eclipse not found!\n");
        }
    }

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

bool CslGameRedEclipse::GetMapImagePaths(wxArrayString& paths) const
{
    wxInt32 pos;
    wxString path, custom_path;

    if (!m_clientSettings.ConfigPath.IsEmpty())
    {
        custom_path<<m_clientSettings.ConfigPath<<wxT("maps")<<CSL_PATHDIV_WX;
        paths.Add(custom_path);
    }

    if (!m_clientSettings.GamePath.IsEmpty())
    {
        path<<m_clientSettings.GamePath<<wxT("data")<<CSL_PATHDIV_WX<<wxT("maps")<<CSL_PATHDIV_WX;
        paths.Add(path);
    }

    //TODO look for extra package directories
    if ((pos=m_clientSettings.Options.Find(wxT("-p")))!=wxNOT_FOUND)
    {
    }

    return !paths.IsEmpty();
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
