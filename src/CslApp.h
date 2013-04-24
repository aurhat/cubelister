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

#ifndef CSLAPP_H
#define CSLAPP_H

#include <wx/cmdline.h>
#include <wx/snglinst.h>

class CslApp: public wxApp
{
    public:
        enum
        {
            CSL_SHUTDOWN_NONE = 0,
            CSL_SHUTDOWN_FORCE,
            CSL_SHUTDOWN_NORMAL,
            CSL_SHUTDOWN_ERROR
        };

        CslApp() :
                m_shutdown(CSL_SHUTDOWN_NONE),
                m_single(NULL),
                m_engine(NULL)
            { }

        wxString GetHomeDir(wxPathFormat format = wxPATH_NATIVE) const;
        const wxString& GetLanguage() const { return m_lang; }
        wxInt32 GetShutdown() const { return m_shutdown; }
        CslEngine* GetCslEngine() { return m_engine; }

        void SetShutdown(wxInt32 val) { m_shutdown = val; }

    private:
        bool OnInit();
        int OnRun();
        int OnExit();
        void OnFatalException();
        void OnEndSession(wxCloseEvent& event);
        int FilterEvent(wxEvent& event);

        void IpcCall(const wxString& value, wxEvtHandler *evtHandler = NULL) const;

        DECLARE_EVENT_TABLE()

        wxString m_home;
        wxString m_lang;
        wxLocale m_locale;
        wxString m_appPath;
        wxInt32 m_shutdown;
        wxSingleInstanceChecker *m_single;

        CslEngine *m_engine;
};

DECLARE_APP(CslApp);

#endif //CSLAPP_H
