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
#include "CslSettings.h"
#include "CslApp.h"
#include "CslFrame.h"

CslListCtrlSettings& CslSettings::GetListSettings(const wxString& name)
{
    CslSettings& settings=GetInstance();

    loopv(settings.CslListSettings)
    {
        if (settings.CslListSettings[i]->Name==name)
            return *settings.CslListSettings[i];
    }

    settings.CslListSettings.push_back(new CslListCtrlSettings(name));

    return *settings.CslListSettings.Last();
}

wxFileConfig* CslSettings::OpenConfig(const wxString& name, bool read)
{
    wxLogNull nolog;

    if (read)
    {
        if (!::wxFileExists(name))
            return NULL;
    }
    else
    {
        wxFileName fn(name);

        if (!wxFileName::DirExists(fn.GetPath()) &&
            !wxFileName::Mkdir(fn.GetPath(), 0700, wxPATH_MKDIR_FULL))
            return NULL;
    }

    wxFileConfig *cfg = new wxFileConfig(wxT(""), wxT(""), name, wxT(""),
                                         wxCONFIG_USE_LOCAL_FILE, wxConvUTF8);

    if (!read)
        cfg->SetUmask(0077);
    else if (cfg->GetNumberOfGroups()==0)
    {
        delete cfg;

        wxFileConfig *cfg = new wxFileConfig(wxT(""), wxT(""), name, wxT(""),
                                             wxCONFIG_USE_LOCAL_FILE, wxConvLocal);

        if (cfg->GetNumberOfGroups()==0)
        {
            delete cfg;
            return NULL;
        }
    }

    return cfg;
}

void CslSettings::LoadSettings()
{
    wxFileConfig *cfg = OpenConfig(::wxGetApp().GetHomeDir() + CSL_SETTINGS_FILE, true);

    if (!cfg)
        return;

    wxString s;
    long int val;

    CslSettings& settings = GetInstance();
    CslEngine *engine = ::wxGetApp().GetCslEngine();

    cfg->SetPath(wxT("/Version"));
    if (cfg->Read(wxT("Version"), &val)) settings.Version = val;

    cfg->SetPath(wxT("/Gui"));
    if (cfg->Read(wxT("SizeX"), &val)) settings.FrameSize.SetWidth(val);
    if (cfg->Read(wxT("SizeY"), &val)) settings.FrameSize.SetHeight(val);
    if (cfg->Read(wxT("SizeMaxX"), &val)) settings.FrameSizeMax.SetWidth(val);
    if (cfg->Read(wxT("SizeMaxY"), &val)) settings.FrameSizeMax.SetHeight(val);
    if (cfg->Read(wxT("Layout"), &s)) settings.Layout=s;
    if (cfg->Read(wxT("Systray"), &val)) settings.Systray=val;
    if (cfg->Read(wxT("TTS"), &val)) settings.TTS=val!=0;
    if (cfg->Read(wxT("TTSVolume"), &val))
    {
        if (val>0 && val<=100)
            settings.TTSVolume=val;
    }
    if (cfg->Read(wxT("UpdateInterval"), &val))
    {
        if (val>=CSL_UPDATE_INTERVAL_MIN && val<=CSL_UPDATE_INTERVAL_MAX)
            settings.UpdateInterval=val;
    }
    if (cfg->Read(wxT("NoUpdatePlaying"), &val)) settings.DontUpdatePlaying=val!=0;
    if (cfg->Read(wxT("SearchLAN"), &val)) settings.SearchLAN=val!=0;
    if (cfg->Read(wxT("FilterMaster"), &val)) settings.FilterMaster=val;
    if (cfg->Read(wxT("FilterFavourites"), &val)) settings.FilterFavourites=val;
    if (cfg->Read(wxT("WaitOnFullServer"), &val))
    {
        if (val>=CSL_WAIT_SERVER_FULL_MIN && val<=CSL_WAIT_SERVER_FULL_MAX)
            settings.WaitServerFull=val;
    }
    if (cfg->Read(wxT("CleanupServers"), &val))
    {
        if ((!val || (val && val>=86400)) && val<=CSL_CLEANUP_SERVERS_MAX*86400)
            settings.CleanupServers=val;
    }
    if (cfg->Read(wxT("CleanupServersKeepFavourites"), &val))
        settings.CleanupServersKeepFav=val!=0;
    if (cfg->Read(wxT("CleanupServersKeepStats"), &val))
        settings.CleanupServersKeepStats=val!=0;
    if (cfg->Read(wxT("TooltipDelay"), &val))
    {
        if (val>=CSL_TOOLTIP_DELAY_MIN && val<=CSL_TOOLTIP_DELAY_MAX)
            settings.TooltipDelay=val/CSL_TOOLTIP_DELAY_STEP*CSL_TOOLTIP_DELAY_STEP;
    }
    if (cfg->Read(wxT("PingGood"), &val))
    {
        if (val<=9999)
            settings.PingGood=val;
    }
    if (cfg->Read(wxT("PingBad"), &val))
    {
        if (val<settings.PingGood)
            settings.PingBad=settings.PingGood;
        else
            settings.PingBad=val;
    }
    if (cfg->Read(wxT("ScreenOutputPath"), &s)) settings.ScreenOutputPath=s;
    if (settings.ScreenOutputPath.IsEmpty())
        settings.ScreenOutputPath = ::wxGetApp().GetHomeDir();
    if (cfg->Read(wxT("GameOutputPath"), &s)) settings.GameOutputPath=s;
    if (settings.GameOutputPath.IsEmpty())
        settings.GameOutputPath = ::wxGetApp().GetHomeDir();
    if (cfg->Read(wxT("AutoSaveOutput"), &val)) settings.AutoSaveOutput=val!=0;
    if (cfg->Read(wxT("LastSelectedGame"), &s)) settings.LastGame=s;
    if (cfg->Read(wxT("MinPlaytime"), &val))
    {
        if (val>=CSL_MIN_PLAYTIME_MIN && val<=CSL_MIN_PLAYTIME_MAX)
            settings.MinPlaytime=val;
    }

    /* Checking for program updates */
    if (cfg->Read(wxT("CheckReleaseVersion"), &val)) settings.CheckReleaseVersion = val!=0;
    if (cfg->Read(wxT("CheckTestingVersion"), &val)) settings.CheckTestingVersion = val!=0;

    /* GeoIP */
    cfg->SetPath(wxT("/GeoIP"));
    if (cfg->Read(wxT("GeoIPType"), &val)) settings.GeoIPType = clamp((int)val, 0, 1);
    if (cfg->Read(wxT("GeoIPAutoUpdate"), &val)) settings.GeoIPAutoUpdate = val!=0;
    if (cfg->Read(wxT("GeoIPCountryLastCheck"), &val)) settings.GeoIPCountryLastCheck = val;
    if (cfg->Read(wxT("GeoIPCountryLastDate"), &val)) settings.GeoIPCountryLastDate = val;
    if (cfg->Read(wxT("GeoIPCityLastCheck"), &val)) settings.GeoIPCityLastCheck = val;
    if (cfg->Read(wxT("GeoIPCityLastDate"), &val)) settings.GeoIPCityLastDate = val;

    /* Lists */
    cfg->SetPath(wxT("/List"));
    if (cfg->Read(wxT("AutoSort"), &val)) settings.AutoSortColumns=val!=0;
    /* Colours */
    /* Server lists */
    if (cfg->Read(wxT("ColourEmpty"), &val)) settings.ColServerEmpty=INT2COLOUR(val);
    if (cfg->Read(wxT("ColourOffline"), &val)) settings.ColServerOff=INT2COLOUR(val);
    if (cfg->Read(wxT("ColourFull"), &val)) settings.ColServerFull=INT2COLOUR(val);
    if (cfg->Read(wxT("ColourMM1"), &val)) settings.ColServerMM1=INT2COLOUR(val);
    if (cfg->Read(wxT("ColourMM2"), &val)) settings.ColServerMM2=INT2COLOUR(val);
    if (cfg->Read(wxT("ColourMM3"), &val)) settings.ColServerMM3=INT2COLOUR(val);
    /* Player lists */
    if (cfg->Read(wxT("PlayerMaster"), &val)) settings.ColPlayerMaster=INT2COLOUR(val);
    if (cfg->Read(wxT("PlayerAuth"), &val)) settings.ColPlayerAuth=INT2COLOUR(val);
    if (cfg->Read(wxT("PlayerAdmin"), &val)) settings.ColPlayerAdmin=INT2COLOUR(val);
    /* other */
    if (cfg->Read(wxT("ColourSearch"), &val)) settings.ColServerHigh=INT2COLOUR(val);
    if (cfg->Read(wxT("ColourPlaying"), &val)) settings.ColServerPlay=INT2COLOUR(val);
    if (cfg->Read(wxT("ColourStripes"), &val)) settings.ColInfoStripe=INT2COLOUR(val);

    wxInt32 i, pos;
    wxString name, value;

    for (i=0; cfg->Read(wxString::Format(wxT("ColumnsMask%d"), i), &name); i++)
    {
        unsigned long int columnmask=0;

        if ((pos=name.Find(wxT(':')))==wxNOT_FOUND)
            continue;
        if (!name.Mid(0, pos).ToULong(&columnmask))
            continue;
        if ((name=name.Mid(pos+1)).IsEmpty())
            continue;

        CslListCtrlSettings& ls = GetListSettings(name);
        ls.ColumnMask = columnmask;
    }

    /* Client */
    CslArrayCslGame& games=engine->GetGames();
    loopv(games)
    {
#ifdef __WXMAC__
        val=true;
#else
        val=false;
#endif //__WXMAC__
        CslGameClientSettings settings;

        cfg->SetPath(wxT("/")+games[i]->GetName());
        if (cfg->Read(wxT("Binary"), &s)) settings.Binary=s;
        if (cfg->Read(wxT("GamePath"), &s)) settings.GamePath=s;
        if (cfg->Read(wxT("ConfigPath"), &s)) settings.ConfigPath=s;
        if (cfg->Read(wxT("Options"), &s)) settings.Options=s;
        if (cfg->Read(wxT("PreScript"), &s)) settings.PreScript=s;
        if (cfg->Read(wxT("PostScript"), &s)) settings.PostScript=s;

        games[i]->SetClientSettings(settings);
    }

    delete cfg;
}

void CslSettings::SaveSettings()
{
    wxFileConfig *cfg = OpenConfig(::wxGetApp().GetHomeDir() + CSL_SETTINGS_FILE, false);

    if (!cfg)
        return;

    CslSettings& settings = GetInstance();
    CslEngine *engine = ::wxGetApp().GetCslEngine();
    wxAuiManager& AuiMgr = ((CslFrame*)wxGetApp().GetTopWindow())->GetAuiManager();

    cfg->DeleteAll();

    cfg->SetPath(wxT("/Version"));
    cfg->Write(wxT("Version"), CSL_CONFIG_VERSION);

    /* GUI */
    wxSize size=settings.FrameSize;
    cfg->SetPath(wxT("/Gui"));
    cfg->Write(wxT("SizeX"), (long int)size.GetWidth());
    cfg->Write(wxT("SizeY"), (long int)size.GetHeight());
    size=settings.FrameSizeMax;
    cfg->Write(wxT("SizeMaxX"), (long int)size.GetWidth());
    cfg->Write(wxT("SizeMaxY"), (long int)size.GetHeight());
    cfg->Write(wxT("Layout"), AuiMgr.SavePerspective());
    cfg->Write(wxT("Systray"), settings.Systray);
    cfg->Write(wxT("TTS"), settings.TTS);
    cfg->Write(wxT("TTSVolume"), settings.TTSVolume);
    cfg->Write(wxT("UpdateInterval"), (long int)settings.UpdateInterval);
    cfg->Write(wxT("NoUpdatePlaying"), settings.DontUpdatePlaying);
    cfg->Write(wxT("SearchLAN"), settings.SearchLAN);
    cfg->Write(wxT("FilterMaster"), (long int)settings.FilterMaster);
    cfg->Write(wxT("FilterFavourites"), (long int)settings.FilterFavourites);
    cfg->Write(wxT("WaitOnFullServer"), (long int)settings.WaitServerFull);
    cfg->Write(wxT("CleanupServers"), (long int)settings.CleanupServers);
    cfg->Write(wxT("CleanupServersKeepFavourites"), (long int)settings.CleanupServersKeepFav);
    cfg->Write(wxT("CleanupServersKeepStats"), (long int)settings.CleanupServersKeepStats);
    cfg->Write(wxT("TooltipDelay"), (long int)settings.TooltipDelay);
    cfg->Write(wxT("PingGood"), (long int)settings.PingGood);
    cfg->Write(wxT("PingBad"), (long int)settings.PingBad);
    cfg->Write(wxT("ScreenOutputPath"), settings.ScreenOutputPath);
    cfg->Write(wxT("GameOutputPath"), settings.GameOutputPath);
    cfg->Write(wxT("AutoSaveOutput"), settings.AutoSaveOutput);
    cfg->Write(wxT("LastSelectedGame"), settings.LastGame);
    cfg->Write(wxT("MinPlaytime"), (long int)settings.MinPlaytime);

    /* Checking for program updates */
    cfg->Write(wxT("CheckReleaseVersion"), (long int)settings.CheckReleaseVersion);
    cfg->Write(wxT("CheckTestingVersion"), (long int)settings.CheckTestingVersion);

    /* GeoIP */
    cfg->SetPath(wxT("/GeoIP"));
    cfg->Write(wxT("GeoIPType") ,settings.GeoIPType);
    cfg->Write(wxT("GeoIPAutoUpdate"), (long int)settings.GeoIPAutoUpdate);
    cfg->Write(wxT("GeoIPCountryLastCheck"), (long int)settings.GeoIPCountryLastCheck);
    cfg->Write(wxT("GeoIPCountryLastDate"), (long int)settings.GeoIPCountryLastDate);
    cfg->Write(wxT("GeoIPCityLastCheck"), (long int)settings.GeoIPCityLastCheck);
    cfg->Write(wxT("GeoIPCityLastDate"), (long int)settings.GeoIPCityLastDate);

    /* ListCtrl */
    cfg->SetPath(wxT("/List"));
    cfg->Write(wxT("ColourEmpty"), COLOUR2INT(settings.ColServerEmpty));
    cfg->Write(wxT("ColourOffline"), COLOUR2INT(settings.ColServerOff));
    cfg->Write(wxT("ColourFull"), COLOUR2INT(settings.ColServerFull));
    cfg->Write(wxT("ColourMM1"), COLOUR2INT(settings.ColServerMM1));
    cfg->Write(wxT("ColourMM2"), COLOUR2INT(settings.ColServerMM2));
    cfg->Write(wxT("ColourMM3"), COLOUR2INT(settings.ColServerMM3));
    cfg->Write(wxT("ColourSearch"), COLOUR2INT(settings.ColServerHigh));
    cfg->Write(wxT("ColourPlaying"), COLOUR2INT(settings.ColServerPlay));
    cfg->Write(wxT("ColourStripes"), COLOUR2INT(settings.ColInfoStripe));

    wxInt32 id=0;
    wxString n, v;
    CslArraywxWindow lists;
    GetChildWindowsByClassInfo(NULL, CLASSINFO(CslListCtrl), lists);

    loopv(lists)
    {
        CslListCtrl *list = (CslListCtrl*)lists[i];
        const wxString& name = list->GetName();

        if (!name.empty() && name!=wxListCtrlNameStr)
        {
            wxUint32 m = list->ListGetColumnMask();

            if (m)
            {
                n = wxString::Format(wxT("ColumnsMask%d"), id++);
                v = wxString::Format(wxT("%u:"), m)+name;
                cfg->Write(n, v);
            }
        }
    }

    /* Client */
    CslArrayCslGame& games=engine->GetGames();
    loopv(games)
    {
        CslGame& game=*games[i];
        const CslGameClientSettings& settings=game.GetClientSettings();

        cfg->SetPath(wxT("/")+game.GetName());
        cfg->Write(wxT("Binary"), settings.Binary);
        cfg->Write(wxT("GamePath"), settings.GamePath);
        cfg->Write(wxT("ConfigPath"), settings.ConfigPath);
        cfg->Write(wxT("Options"), settings.Options);
        cfg->Write(wxT("PreScript"), settings.PreScript);
        cfg->Write(wxT("PostScript"), settings.PostScript);
    }

    delete cfg;
}


class CslIDMapping
{
    public:
        CslIDMapping(wxInt32 oldId, wxInt32 newId) :
                m_oldId(oldId), m_newId(newId) { }

        wxInt32 m_oldId, m_newId;
};
WX_DEFINE_ARRAY(CslIDMapping*, CslArrayCslIDMapping);

bool CslSettings::LoadServers(wxUint32 *numm, wxUint32 *nums)
{
    wxFileConfig *cfg = OpenConfig(::wxGetApp().GetHomeDir() + CSL_SERVERS_FILE, true);

    if (!cfg)
        return false;

    long int val;
    bool read_server;
    wxUint32 mc=0, sc=0, tmc=0, tsc=0;
    CslMaster *master;
    CslArrayCslIDMapping mappings;
    wxArrayInt ids;

    wxString addr, path, pass1, pass2, description;
    wxUint16 port=0, iport=0;
    wxUint32 view=0;
    wxUint32 events=0;
    wxUint32 lastSeen=0;
    wxUint32 playLast=0;
    wxUint32 playTimeLastGame=0;
    wxUint32 playTimeTotal=0;
    wxUint32 connectedTimes=0;
    wxUint32 version = 0;
    wxInt32 gt;

    CslEngine *engine = ::wxGetApp().GetCslEngine();
    CslArrayCslGame& games = engine->GetGames();
    CslFrame *frame = (CslFrame*)::wxGetApp().GetTopWindow();

    cfg->SetPath(wxT("/Version"));
    if (cfg->Read(wxT("Version"), &val)) version = val;

    for (gt=0; gt<(wxInt32)games.GetCount(); gt++)
    {
        wxString s=wxT("/")+games[gt]->GetName();
        cfg->SetPath(s);

        mc=0;
        while (1)
        {
            long int id;
            master = NULL;

            cfg->SetPath(s+wxString::Format(wxT("/Master/%d"), mc++));

            if (cfg->Read(wxT("Address"), &addr) &&
                cfg->Read(wxT("ID"), &id))
            {
                if (version<2)
                {
                    val = 0;
                    cfg->Read(wxT("Port"), &val, 0); port = val;
                    cfg->Read(wxT("Type"), &val, 0);
                    cfg->Read(wxT("Path"), &path);

                    if (path.IsEmpty() && val)
                        path = wxT("/list\n");

                    master = CslMaster::Create(CslMaster::CreateURI(val ? wxT("tcp") : wxT("http"),
                                                                    addr, port, path));
                }
                else
                    master = CslMaster::Create(addr);
            }

            if (!master)
                break;

            if (games[gt]->AddMaster(master)<0)
            {
                CslMaster::Destroy(master);
                continue;
            }
            mappings.Add(new CslIDMapping(id, master->GetId()));
            tmc++;
        }

        sc=0;
        read_server=true;
        while (read_server)
        {
            wxInt32 id=0;
            ids.Empty();

            cfg->SetPath(s+wxString::Format(wxT("/Server/%d"), sc++));

            read_server=false;
            if (cfg->Read(wxT("Address"), &addr))
            {
                cfg->Read(wxT("Port"), &val, games[gt]->GetDefaultGamePort()); port=val;
                // use 0 as default infoport to import older servers.ini and
                // set the appropriate infoport when creating the CslServerInfo
                cfg->Read(wxT("InfoPort"), &val, 0); iport=val;
                cfg->Read(wxT("Password"), &pass1, wxEmptyString);
                cfg->Read(wxT("AdminPassword"), &pass2, wxEmptyString);
                cfg->Read(wxT("Description"), &description, wxEmptyString);

                while (cfg->Read(wxString::Format(wxT("Master%d"), id++), &val))
                    ids.Add(val);
                if (ids.GetCount()==0)
                    ids.Add(-1);

                if (cfg->Read(wxT("View"), &val))
                {
                    view=val;
                    cfg->Read(wxT("Events"), &val, 0);
                    events=(wxUint32)val;
                    cfg->Read(wxT("LastSeen"), &val, 0);
                    lastSeen=(wxUint32)val;
                    cfg->Read(wxT("PlayLast"), &val, 0);
                    playLast=(wxUint32)val;
                    cfg->Read(wxT("PlayTimeLastGame"), &val, 0);
                    playTimeLastGame=(wxUint32)val;
                    cfg->Read(wxT("PlayTimeTotal"), &val, 0);
                    playTimeTotal=(wxUint32)val;
                    cfg->Read(wxT("ConnectedTimes"), &val, 0);
                    connectedTimes=(wxUint32)val;
                    read_server=true;
                }
            }

            if (!read_server)
                break;

            loopv(ids)
            {
                CslServerInfo *info = CslServerInfo::Create(games[gt], addr, port, iport ? iport:port+1,
                                                            view, lastSeen, playLast, playTimeLastGame,
                                                            playTimeTotal, connectedTimes,
                                                            description, pass1, pass2);

                if (ids[i]==-1)
                    id=-1;
                else
                {
                    loopvj(mappings)
                    {
                        if (mappings[j]->m_oldId==ids[i])
                        {
                            id=mappings[j]->m_newId;
                            break;
                        }
                    }
                }

                if (engine->AddServer(games[gt], info, id))
                {
                    long type;
                    wxInt32 pos, j=0;
                    wxString guiview;

                    info->RegisterEvents(events);

                    while (cfg->Read(wxString::Format(wxT("GuiView%d"), j++), &guiview))
                    {
                        if ((pos=guiview.Find(wxT(':')))==wxNOT_FOUND)
                            continue;
                        if (!guiview.Mid(0, pos).ToLong(&type))
                            continue;
                        if ((guiview=guiview.Mid(pos+1)).IsEmpty())
                            continue;
                        frame->CreateServerView(info, type, guiview);
                    }

                    tsc++;
                }
                else
                    CslServerInfo::Destroy(info);
            }
        }
        //engine->ResetPingSends(games[gt], NULL);
    }

    delete cfg;

    WX_CLEAR_ARRAY(mappings);

    if (numm)
        *nums=tmc;
    if (nums)
        *nums=tsc;

    return tmc>0||tsc>0;
}

void CslSettings::SaveServers()
{
    CslEngine *engine=::wxGetApp().GetCslEngine();
    CslArrayCslGame& games=engine->GetGames();

    if (!games.GetCount())
        return;

    wxFileConfig *cfg = OpenConfig(::wxGetApp().GetHomeDir() + CSL_SERVERS_FILE, false);

    if (!cfg)
        return;

    wxString s;
    wxInt32 mc, sc;
    CslGame *game;
    CslServerInfo *info;

    cfg->DeleteAll();

    cfg->SetPath(wxT("/Version"));
    cfg->Write(wxT("Version"), CSL_SERVERCONFIG_VERSION);

    loopv(games)
    {
        game=games[i];
        s=wxT("/")+game->GetName();
        cfg->SetPath(s);

        CslArrayCslMaster& masters=game->GetMasters();
        mc=0;

        loopvj(masters)
        {
            wxString uri = masters[j]->GetURI().BuildURI();

            cfg->SetPath(s+s.Format(wxT("/Master/%d"), mc++));
            cfg->Write(wxT("Address"), uri);
            cfg->Write(wxT("ID"), masters[j]->GetId());
        }

        CslArrayCslServerInfo& servers=game->GetServers();
        if (servers.GetCount()==0)
            continue;

        sc=0;
        loopvj(servers)
        {
            info=servers[j];
            cfg->SetPath(s+s.Format(wxT("/Server/%d"), sc++));
            cfg->Write(wxT("Address"), info->Host);
            cfg->Write(wxT("Port"), info->GamePort);
            cfg->Write(wxT("InfoPort"), info->Address().GetPort());
            cfg->Write(wxT("Password"), info->Password);
            cfg->Write(wxT("AdminPassword"), info->PasswordAdmin);
            cfg->Write(wxT("Description"), info->DescriptionOld);
            wxArrayInt masters=info->GetMasterIDs();
            loopvk(masters) cfg->Write(wxString::Format(wxT("Master%d"), k), masters[k]);
            cfg->Write(wxT("View"), (int)info->View);
            cfg->Write(wxT("Events"), (int)info->GetRegisteredEvents());
            cfg->Write(wxT("LastSeen"), (int)info->LastSeen);
            cfg->Write(wxT("PlayLast"), (int)info->PlayedLast);
            cfg->Write(wxT("PlayTimeLastGame"), (int)info->PlayTimeLast);
            cfg->Write(wxT("PlayTimeTotal"), (int)info->PlayTimeTotal);
            cfg->Write(wxT("ConnectedTimes"), (int)info->ConnectedTimes);

            wxUint32 l=0;
            wxAuiManager& AuiMgr=((CslFrame*)wxGetApp().GetTopWindow())->GetAuiManager();
            CslArrayCslServerView& serverviews = ((CslFrame*)wxGetApp().GetTopWindow())->GetServerViews();

            loopvk(serverviews)
            {
                if (k==0)
                    continue;
                CslPanelServerView *list=serverviews[k];
                if (list->GetServerInfo()==info)
                {
                    wxAuiPaneInfo& pane=AuiMgr.GetPane(list);
                    if (!pane.IsOk())
                        continue;

                    wxString view=wxString::Format(wxT("%d:"), list->ListCtrl()->View())+pane.name;
                    cfg->Write(wxString::Format(wxT("GuiView%d"), l++), view);
                }
            }
        }
    }

    delete cfg;
}
