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
#include "CslFrame.h"
#include "CslIPC.h"
#include "CslApp.h"
#ifndef _DEBUG
#include <wx/debugrpt.h>
#endif //_DEBUG
#ifdef __WXMAC__
#include <Carbon/Carbon.h>
#endif //__WXMAC__

IMPLEMENT_APP(CslApp)

BEGIN_EVENT_TABLE(CslApp,wxApp)
    EVT_END_SESSION(CslApp::OnEndSession)
END_EVENT_TABLE()

#ifdef __WXMAC__
static pascal OSErr MacCallbackGetUrl(const AppleEvent *in,AppleEvent *out,long ptr)
{
    Size l=0;
    OSErr err=noErr;
    DescType type=typeChar;

    if ((err=AESizeOfParam(in,keyDirectObject,&type,&l))==noErr && l)
    {
        char buf[l+1];

        if ((err=AEGetParamPtr(in,keyDirectObject,type,0,&buf,l,&l))==noErr && l)
        {
            buf[l]=0;
            const CslApp& app=::wxGetApp();
            app.IpcCall(C2U(buf),app.GetTopWindow());
        }
    }

    return noErr;
}
#endif //__WXMAC__

bool CslApp::OnInit()
{
#ifdef _DEBUG
    #ifdef _MSC_VER
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    #endif
#else
    ::wxHandleFatalExceptions(true);
#endif

    m_engine = NULL;
    m_single = NULL;
    m_shutdown = CSL_SHUTDOWN_NONE;

    wxString cwd = DirName(wxPathOnly(wxTheApp->argv[0]));

#ifdef PROJECTDIR
    AddDataDir(wxT(PROJECTDIR), true);
#endif //PROJECTDIR
    AddDataDir(CSL_USER_DIR, true);
#ifndef DATADIR
    #ifdef __WXMAC__
    AddDataDir(cwd+wxT("../Resources/"), true);
    #else
    AddDataDir(cwd, true);
    #endif
#else
    AddDataDir(wxT(DATADIR));
#endif //DATADIR

#ifdef BUILDDIR
    AddPluginDir(wxT(BUILDDIR), true);
#endif //BUILDDIR
    AddPluginDir(CSL_USER_DIR, true);
#ifndef PKGLIBDIR
    #ifdef __WXMAC__
    AddPluginDir(cwd+wxT("../"), true);
    #endif
    #ifdef __WXMSW__
    AddPluginDir(cwd, true);
    #endif
#else
    AddPluginDir(wxT(PKGLIBDIR));
#endif //PKGLIBDIR

    m_locale.Init(wxLANGUAGE_DEFAULT,
#if wxCHECK_VERSION(2, 9, 0)
                  wxLOCALE_LOAD_DEFAULT
#else
                  wxLOCALE_CONV_ENCODING
#endif //wxCHECK_VERSION
                 );

#ifndef LOCALEDIR
#ifdef __WXMSW__
    m_locale.AddCatalogLookupPathPrefix(cwd+wxT("\\locale"));
#else
#ifdef __WXMAC__
    m_locale.AddCatalogLookupPathPrefix(cwd+wxT("../Resources"));
#endif //__WXMAC__
#endif //__WXMSW__
#else
    m_locale.AddCatalogLookupPathPrefix(wxT(LOCALEDIR));
#endif //LOCALEDIR

    if (m_locale.AddCatalog(wxString(CSL_NAME_SHORT_STR).Lower()))
        m_lang=m_locale.GetCanonicalName();

#ifdef __WXMAC__
    wxSystemOptions::SetOption(wxT("mac.listctrl.always_use_generic"),1);
    //enables Command-H, Command-M and Command-Q at least when not in fullscreen
    wxSetEnv(wxT("SDL_SINGLEDISPLAY"),wxT("1"));
    wxSetEnv(wxT("SDL_ENABLEAPPEVENTS"),wxT("1"));
    //TODO wxApp::SetExitOnFrameDelete(false);
    //register event handler for URI schemes
    AEInstallEventHandler(kInternetEventClass,kAEGetURL,
                          NewAEEventHandlerUPP((AEEventHandlerProcPtr)MacCallbackGetUrl),0,false);
#endif //__WXMAC__

    wxString cmdline;

    for (wxInt32 i=1;i<wxApp::argc;i++)
        cmdline << argv[i] << wxT(" ");

    wxString lock = wxString::Format(wxT(".%s-%s.lock"),CSL_NAME_SHORT_STR,wxGetUserId().c_str());
    m_single = new wxSingleInstanceChecker(lock);

    if (m_single->IsAnotherRunning())
    {
        IpcCall(cmdline.IsEmpty() ? wxT("show") : cmdline);
        return true;
    }

    m_engine=new CslEngine;

    wxInitAllImageHandlers();

    CslFrame* frame = new CslFrame(NULL,wxID_ANY,wxEmptyString,wxDefaultPosition);

    if (m_shutdown!=CSL_SHUTDOWN_NONE)
        return true;

    SetTopWindow(frame);
    frame->Show();

    if (cmdline.Find(CSL_URI_SCHEME_STR)!=wxNOT_FOUND)
        IpcCall(cmdline, frame);

    return true;
}

int CslApp::OnRun()
{
    if (GetTopWindow())
        wxApp::OnRun();

    return m_shutdown>CSL_SHUTDOWN_NORMAL ? 1 : 0;
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
    CSL_LOG_DEBUG("\n");

    m_shutdown=CSL_SHUTDOWN_FORCE;

    event.Skip();
}

int CslApp::FilterEvent(wxEvent& event)
{
    if (event.GetEventType()==wxEVT_KEY_DOWN)
        CslToolTip::Reset();

    return -1;
}

#ifndef _DEBUG
void CslApp::OnFatalException()
{
    wxDebugReport report;
    wxDebugReportPreviewStd preview;

    report.AddAll();

    if (preview.Show(report))
        report.Process();
}
#endif //_DEBUG

void CslApp::IpcCall(const wxString& value,wxEvtHandler *evtHandler) const
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
#endif //wxCHECK_VERSION
    }
}
