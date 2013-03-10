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
#include "CslGameCube.h"

#include "../img/cb_16_png.h"
#include "../img/cb_24_png.h"

CslGameCube::CslGameCube()
{
    m_name=CSL_DEFAULT_NAME_CB;
    m_fourcc=CSL_BUILD_FOURCC(CSL_FOURCC_CB);
    m_capabilities=CSL_CAPABILITY_CONNECT_PASS;
    m_defaultMasterConnection=CslMasterConnection(CSL_DEFAULT_MASTER_CB, CSL_DEFAULT_MASTER_PATH_CB);

    AddIcon(CSL_BITMAP_TYPE_PNG, 16, cb_16_png, sizeof(cb_16_png));
    AddIcon(CSL_BITMAP_TYPE_PNG, 24, cb_24_png, sizeof(cb_24_png));
}

CslGameCube::~CslGameCube()
{
}

inline const wxChar* CslGameCube::GetModeName(wxInt32 mode) const
{
    static const wxChar* modes[] =
    {
        wxT("ffa/default"), wxT("coopedit"), wxT("ffa/duel"), wxT("teamplay"),
        wxT("instagib"), wxT("instagib team"), wxT("efficiency"), wxT("efficiency team"),
        wxT("insta arena"), wxT("insta clan arena"), wxT("tactics arena")
    };

    return (mode>=0 && (size_t)mode<sizeof(modes)/sizeof(modes[0])) ?
           modes[mode] : T2C(_("unknown"));
}

bool CslGameCube::PingDefault(ucharbuf& buf, CslServerInfo& info) const
{
    putint(buf, m_fourcc);
    return true;
}

bool CslGameCube::ParseDefaultPong(ucharbuf& buf, CslServerInfo& info) const
{
    char text[MAXSTRLEN];
    wxInt32 i;

    if ((wxUint32)getint(buf)!=m_fourcc)
        return false;

    // "decrypt"
    for (i=0; i<buf.maxlength(); i++)
        *buf.at(i)^=0x61;

    i=getint(buf);
    if (i!=CSL_LAST_PROTOCOL_CB)
        return false;
    info.Protocol=i;
    info.Version=wxString::Format(wxT("%d"), i);
    info.MM=CSL_SERVER_OPEN;
    info.MMDescription=wxEmptyString;
    info.GameMode=GetModeName(getint(buf));
    i=getint(buf);
    if (info.HasRegisteredEvent(CslServerEvents::EVENT_EMPTY) && info.Players>0 && !i)
        info.SetEvents(CslServerEvents::EVENT_EMPTY);
    else if (info.HasRegisteredEvent(CslServerEvents::EVENT_NOT_EMPTY) && !info.Players && i>0)
        info.SetEvents(CslServerEvents::EVENT_NOT_EMPTY);
    info.Players=i;
    info.TimeRemain=max(0, getint(buf))*60;
    getstring(text, buf);
    info.Map=C2U(text);
    getstring(text, buf);
    FilterCubeString(text, 1);
    info.SetDescription(C2U(text));

    return !buf.overread();
}

CslGameClientSettings CslGameCube::GuessClientSettings(const wxString& path) const
{
    return CslGameClientSettings();
}

wxString CslGameCube::ValidateClientSettings(CslGameClientSettings& settings) const
{
    return wxEmptyString;
}

void CslGameCube::SetClientSettings(const CslGameClientSettings& settings)
{
    CslGameClientSettings set=settings;

    if (set.GamePath.IsEmpty() || !::wxDirExists(set.GamePath))
        return;
#ifdef __WXMAC__
    if (set.Binary.IsEmpty())
        set.Binary=set.GamePath+wxT("Cube.app/Contents/MacOS/Cube");
#endif
    if (set.Binary.IsEmpty() || !::wxFileExists(set.Binary))
        return;

    m_clientSettings=set;
}

wxString CslGameCube::GameStart(CslServerInfo *info, wxInt32 mode, wxString& error)
{
    wxString address, password;
    wxString bin=m_clientSettings.Binary;
    wxString opts=m_clientSettings.Options;

    if (m_clientSettings.Binary.IsEmpty() || !::wxFileExists(m_clientSettings.Binary))
    {
        error=_("Client binary for game Cube not found!");
        error<<_("Please check your settings.");
        return wxEmptyString;
    }
    if (m_clientSettings.GamePath.IsEmpty() || !::wxDirExists(m_clientSettings.GamePath))
    {
        error=_("Game path for game Cube not found!");
        error<<_("Please check your settings.");
        return wxEmptyString;
    }

    address=info->Host;
    if (GetDefaultGamePort()!=info->GamePort)
        address<<wxString::Format(wxT(" %d"), info->GamePort);

#ifdef __WXMSW__
    //binary must be surrounded by quotes if the path contains spaces
    bin=wxT("\"")+m_clientSettings.Binary+wxT("\"");
#else
    CmdlineEscapeSpaces(bin);
#endif
    bin<<wxT(" ")<<opts;

    if (mode==CslServerInfo::CSL_CONNECT_ADMIN_PASS)
        password=info->Password;

    CSL_LOG_DEBUG("start client: %s\n", U2C(bin));

    return InjectConfig(address, password, error)==CSL_ERROR_NONE ? bin : wxString(wxEmptyString);
}

wxInt32 CslGameCube::GameEnd(wxString& error)
{
    wxString cfg=m_clientSettings.GamePath+CSL_DEFAULT_INJECT_FILE_CB;
    wxString bak=cfg+wxT(".csl");

    if (!::wxFileExists(bak))
        return CSL_ERROR_FILE_DONT_EXIST;
    if (!::wxRenameFile(bak, cfg))
        return CSL_ERROR_FILE_OPERATION;

    return CSL_ERROR_NONE;
}

wxInt32 CslGameCube::InjectConfig(const wxString& address, const wxString& password, wxString& error)
{
    char *buf;
    wxFile file;
    bool autoexec=true;
    wxString cfg, script;

    cfg=m_clientSettings.GamePath+wxString(wxT("autoexec.cfg"));

    // check autoexec.cfg
    if (!::wxFileExists(cfg))
    {
        if (!file.Create(cfg, false, wxS_IRUSR|wxS_IWUSR))
            return CSL_ERROR_FILE_OPERATION;
        file.Close();
    }
    if (!file.Open(cfg, wxFile::read_write))
        return CSL_ERROR_FILE_OPERATION;

    wxUint32 size=file.Length();

    if (size)
    {
        buf=new char[size+1];
        buf[size]=0;

        if (file.Read((void*)buf, size)!=(wxInt32)size)
        {
            delete[] buf;
            file.Close();
            return CSL_ERROR_FILE_OPERATION;
        }

        if (strstr(buf, "alias csl_connect 1"))
            autoexec=false;

        delete[] buf;
    }

    if (autoexec)
    {
        if (!file.Write(wxT("\r\nalias csl_connect 1\r\n")))
        {
            file.Close();
            return CSL_ERROR_FILE_OPERATION;
        }
    }
    file.Close();

    // make a backup of the map config
    cfg=m_clientSettings.GamePath+CSL_DEFAULT_INJECT_FILE_CB;
    if (::wxFileExists(cfg))
    {
        if (!::wxCopyFile(cfg, cfg+wxT(".csl")))
            return CSL_ERROR_FILE_OPERATION;
    }
    else
    {
        file.Create(cfg+wxT(".csl"), false, wxS_IRUSR|wxS_IWUSR);
        file.Close();
    }

    if (!password.IsEmpty())
        script=wxT("password ")+password;

    script<<wxString::Format(wxT("\r\nif (= $csl_connect 1) [ sleep 1000 [ connect %s ] ]\r\n%s\r\n"),
                             address.c_str(), wxT("alias csl_connect 0"));

    return WriteTextFile(cfg, script, wxFile::write_append);
}

void CslGameCube::ProcessOutput(char *data) const
{
    FilterCubeString(data, 0, true, true);
}


CslGameCube *cube = NULL;

bool plugin_init(CslPluginHost *host)
{
    CslEngine *engine = host->GetCslEngine();

    if (engine)
    {
        cube = new CslGameCube;
        return engine->AddGame(cube);
    }

    return true;
}

void plugin_deinit(CslPluginHost *host)
{
    if (cube)
    {
        CslEngine *engine = host->GetCslEngine();

        if (engine)
            engine->RemoveGame(cube);

        delete cube;
        cube = NULL;
    }
}

IMPLEMENT_PLUGIN(CSL_PLUGIN_VERSION_API, CSL_PLUGIN_TYPE_ENGINE, CSL_BUILD_FOURCC(CSL_FOURCC_CB),
                 CSL_DEFAULT_NAME_CB, CSL_VERSION_STR,
                 wxT("Glen Masgai"), wxT("mimosius@users.sourceforge.net"),
                 CSL_WEBADDR_STR, wxT("GPLv2"), wxT("Cube CSL engine plugin"),
                 &plugin_init, &plugin_deinit, NULL)
