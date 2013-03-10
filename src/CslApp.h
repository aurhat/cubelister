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

/**
    @author Glen Masgai <mimosius@users.sourceforge.net>
*/

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

        const wxString& GetLanguage() const { return m_lang; }

        CslEngine* GetCslEngine() { return m_engine; }

        void Shutdown(wxInt32 val) { m_shutdown=val; }
        wxInt32 Shutdown() const { return m_shutdown; }

        void IpcCall(const wxString& value,wxEvtHandler *evtHandler=NULL) const;

    private:
        wxString m_lang;
        wxLocale m_locale;
        wxString m_appPath;
        wxInt32 m_shutdown;
        wxSingleInstanceChecker *m_single;

        CslEngine *m_engine;

        bool OnInit();
        int OnRun();
        int OnExit();
#ifndef _DEBUG
        void OnFatalException();
#endif //_DEBUG

        int FilterEvent(wxEvent& event);
        void OnEndSession(wxCloseEvent& event);

        DECLARE_EVENT_TABLE()
};

DECLARE_APP(CslApp);

#endif //CSLAPP_H
