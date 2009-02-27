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

#include <wx/sysopt.h>
#include "CslApp.h"
#include "CslFrame.h"
#include "CslIPC.h"
IMPLEMENT_APP(CslApp)


BEGIN_EVENT_TABLE(CslApp,wxApp)
    EVT_END_SESSION(CslApp::OnEndSession)
END_EVENT_TABLE()


bool CslApp::OnInit()
{
    m_engine=NULL;
    m_single=NULL;
    m_shutdown=CSL_SHUTDOWN_NONE;

    ::wxSetWorkingDirectory(wxPathOnly(wxTheApp->argv[0]));
    g_basePath=::wxGetCwd();

    m_locale.Init(wxLANGUAGE_DEFAULT,wxLOCALE_CONV_ENCODING);
    m_locale.AddCatalogLookupPathPrefix(LOCALEPATH);
#ifdef __WXGTK__
    m_locale.AddCatalogLookupPathPrefix(g_basePath+wxString(wxT("/lang")));
#endif
    if (m_locale.AddCatalog(CSL_NAME_SHORT_STR))
        m_lang=m_locale.GetCanonicalName();

#ifdef __WXMAC__
    wxSystemOptions::SetOption(wxT("mac.listctrl.always_use_generic"),1);
    //enables Command-H, Command-M and Command-Q at least when not in fullscreen
    wxSetEnv(wxT("SDL_SINGLEDISPLAY"),wxT("1"));
    wxSetEnv(wxT("SDL_ENABLEAPPEVENTS"),wxT("1"));
    //TODO wxApp::SetExitOnFrameDelete(false);
#endif

    wxString uri;

    for (wxInt32 i=1;i<wxApp::argc;i++)
    {
        uri=wxApp::argv[i];

        if (!uri.StartsWith(CSL_URI_SCHEME_STR))
            uri.Empty();
    }

    wxString lock=wxString::Format(wxT(".%s-%s.lock"),CSL_NAME_SHORT_STR,wxGetUserId().c_str());
    m_single=new wxSingleInstanceChecker(lock);

    if (m_single->IsAnotherRunning())
    {
        if (uri.IsEmpty())
            uri=wxT("show");

        IpcCall(uri);
        return true;
    }

    m_engine=new CslEngine;

    wxInitAllImageHandlers();

    CslFrame* frame=new CslFrame(NULL,wxID_ANY,wxEmptyString,wxDefaultPosition);
    SetTopWindow(frame);
    frame->Show();

    if (!uri.IsEmpty())
        IpcCall(uri,frame);

    return true;
}

int CslApp::OnRun()
{
    if (GetTopWindow())
        wxApp::OnRun();

    return 0;
}

int CslApp::OnExit()
{
    if (m_engine)
        delete m_engine;

    if (m_single)
        delete m_single;

    return 0;
}

void CslApp::OnEndSession(wxCloseEvent& event)
{
    LOG_DEBUG("\n");

    m_shutdown=CSL_SHUTDOWN_FORCE;

    event.Skip();
}

void CslApp::IpcCall(const wxString& value,wxEvtHandler *evtHandler)
{
    if (evtHandler)
    {
        CslIpcEvent evt(CslIpcEvent::IPC_COMMAND,value);

        wxPostEvent(evtHandler,evt);
    }
    else
    {
        CslIpcClient client;

        if (client.Connect(CSL_IPC_HOST,CSL_IPC_SERV,CSL_IPC_TOPIC))
#if wxCHECK_VERSION(2,9,0)
            client.GetConnection()->Poke(CSL_NAME_SHORT_STR,value.c_str());
#else
            client.GetConnection()->Poke(CSL_NAME_SHORT_STR,(wxChar*)value.c_str());
#endif

    }
}
