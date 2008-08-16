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

#include "CslEngine.h"
#include "CslGameBloodFrontier.h"
#include "CslTools.h"

#include "../img/bf_24.xpm"
#include "../img/bf_16.xpm"

CslBloodFrontier::CslBloodFrontier()
{
    m_name=CSL_DEFAULT_NAME_BF;
    m_defaultMasterConnection=CslMasterConnection(CSL_DEFAULT_MASTER_BF,CSL_DEFAULT_MASTER_PORT_BF);
    m_capabilities=CSL_CAPABILITY_EXTINFO | CSL_CAPABILITY_CUSTOM_CONFIG;
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
    G_M_NONE    = 0,
    G_M_TEAM    = 1<<0,
    G_M_INSTA   = 1<<1,
    G_M_DUEL    = 1<<2,
    G_M_PROG    = 1<<3,
    G_M_MULTI   = 1<<4,
    G_M_DLMS    = 1<<5,
    G_M_MAYHEM  = 1<<6,
    G_M_NOITEMS = 1<<7,
    G_M_ALL     = G_M_TEAM|G_M_INSTA|G_M_DUEL|G_M_PROG|G_M_MULTI|G_M_DLMS|G_M_MAYHEM|G_M_NOITEMS,
    G_M_FIGHT   = G_M_TEAM|G_M_INSTA|G_M_DUEL|G_M_MULTI|G_M_DLMS|G_M_NOITEMS,
    G_M_DUKE    = G_M_INSTA|G_M_DUEL|G_M_DLMS|G_M_NOITEMS,
    G_M_STF     = G_M_TEAM|G_M_INSTA|G_M_PROG|G_M_MULTI|G_M_MAYHEM|G_M_NOITEMS,
    G_M_CTF     = G_M_TEAM|G_M_INSTA|G_M_PROG|G_M_MULTI|G_M_MAYHEM|G_M_NOITEMS,
};

#define G_M_NUM     8


static struct
{
    int type,mutators,implied; const wxChar *name;
}
gametype[] =
{
    { G_DEMO,       G_M_NONE,  G_M_NONE,    wxT("Demo")    },
    { G_LOBBY,      G_M_NONE,  G_M_NOITEMS, wxT("Lobby")   },
    { G_EDITMODE,   G_M_NONE,  G_M_NONE,    wxT("Editing") },
    { G_MISSION,    G_M_NONE,  G_M_NONE,    wxT("Mission") },
    { G_DEATHMATCH, G_M_FIGHT, G_M_NONE,    wxT("DM")      },
    { G_STF,        G_M_STF,   G_M_TEAM,    wxT("STF")     },
    { G_CTF,        G_M_CTF,   G_M_TEAM,    wxT("CTF")     },
},
mutstype[] =
{
    { G_M_TEAM,    G_M_ALL,  G_M_NONE,    wxT("Team")   },
    { G_M_INSTA,   G_M_ALL,  G_M_NOITEMS, wxT("Insta")  },
    { G_M_DUEL,    G_M_DUKE, G_M_NONE,    wxT("Duel")   },
    { G_M_PROG,    G_M_ALL,  G_M_NONE,    wxT("PG")     },
    { G_M_MULTI,   G_M_ALL,  G_M_TEAM,    wxT("MS")     },
    { G_M_DLMS,    G_M_DUKE, G_M_DUEL,    wxT("LMS")    },
    { G_M_MAYHEM,  G_M_ALL,  G_M_NONE,    wxT("Mayhem") },
    { G_M_NOITEMS, G_M_ALL,  G_M_NONE,    wxT("NI")     },
};

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

wxString CslBloodFrontier::GetVersionName(wxInt32 n) const
{
    static const wxChar* versions[] =
    {
        wxT("Alpha 2")
    };
    wxUint32 v=CSL_LAST_PROTOCOL_BF-n;
    return (v>=0 && v<sizeof(versions)/sizeof(versions[0])) ?
           wxString(versions[v]) : wxString::Format(wxT("%d"),n);
}

wxString CslBloodFrontier::GetWeaponName(wxInt32 n) const
{
    static const wxChar* weapons[] =
    {
        _("Pistol"),_("Shotgun"),_("Chaingun"),
        _("Grenades"),_("Flamer"),_("Rifle")
    };
    return (n>=0 && (size_t)n<sizeof(weapons)/sizeof(weapons[0])) ?
           wxString(weapons[n]) : wxString(_("unknown"));
}

void CslBloodFrontier::GetPlayerstatsDescriptions(vector<wxString>& desc) const
{
    desc.add(_("Player"));
    desc.add(_("Team"));
    desc.add(_("Frags"));
    desc.add(_("Deaths"));
    desc.add(_("Teamkills"));
    desc.add(_("Accuracy"));
    desc.add(_("Health"));
    desc.add(_("Spree"));
    desc.add(_("Weapon"));
}

bool CslBloodFrontier::ParseDefaultPong(ucharbuf& buf,CslServerInfo& info) const
{
    char text[_MAXDEFSTR];
    wxUint32 l;
    vector<int>attr;
    attr.setsize(0);

    info.Players=getint(buf);
    int numattr=getint(buf);
    loopj(numattr) attr.add(getint(buf));
    info.Protocol=attr[0];
    info.Version=GetVersionName(info.Protocol);
    info.GameMode=GetModeName(attr[1],attr[2]);
    info.TimeRemain=attr[3];
    info.PlayersMax=attr[4];
    info.MM=attr[5];
    getstring(text,buf);
    info.Map=A2U(text);
    getstring(text,buf);
    l=(wxUint32)strlen(text);
    StripColours(text,&l,2);
    info.SetDescription(A2U(text));

    return true;
}

bool CslBloodFrontier::ParsePlayerPong(wxUint32 protocol,ucharbuf& buf,CslPlayerStatsData& info) const
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

    return true;
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

wxString CslBloodFrontier::GameStart(CslServerInfo *info,wxUint32 mode,wxString *error)
{
    wxString address,path;
    wxString bin=m_clientSettings.Binary;
    wxString opts=m_clientSettings.Options;

    if (m_clientSettings.Binary.IsEmpty() || !::wxFileExists(m_clientSettings.Binary))
    {
        *error=_("Client binary for game Blood Frontier not found!\nCheck your settings.");
        return wxEmptyString;
    }
    if (m_clientSettings.GamePath.IsEmpty() || !::wxDirExists(m_clientSettings.GamePath))
    {
        *error=_("Game path for game Blood Frontier not found!\nCheck check your settings.");
        return wxEmptyString;
    }
    if (m_clientSettings.ConfigPath.IsEmpty() || !::wxDirExists(m_clientSettings.ConfigPath))
    {
        *error=_("Config path for game Blood Frontier not found!\nCheck check your settings.");
        return wxEmptyString;
    }

    // use Prepend() and do not use opts+= here, since -q<path> must be before -r
    path=m_clientSettings.ConfigPath;
#ifdef __WXMSW__
    opts.Prepend(wxString(wxT("-h\""))+path.RemoveLast()+wxString(wxT("\" ")));
#else
    path.Replace(wxT(" "),wxT("\\ "));
    opts.Prepend(wxString(wxT("-h"))+path+wxString(wxT(" ")));
#endif //__WXMSW__

    address=info->Host;
    if (GetDefaultPort()!=info->Port)
        address+=m_portDelimiter+wxString::Format(wxT("%d"),info->Port);

#ifdef __WXMSW__
        opts+=wxString(wxT(" -x\"sleep 1000 [connect "))+address+wxString(wxT("]\""));
#else
        address.Replace(wxT(" "),wxT("\\ "));
        opts+=wxString(wxT(" -xsleep\\ 1000\\ [connect\\ "))+address+wxString(wxT("]"));
#endif

    bin.Replace(wxT(" "),wxT("\\ "));
    bin+=wxString(wxT(" "))+opts;

    return bin;
}

wxInt32 CslBloodFrontier::GameEnd(wxString *error)
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
