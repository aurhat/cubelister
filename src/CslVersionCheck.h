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

#ifndef CSLVERSIONCHECK_H
#define CSLVERSIONCHECK_H

/**
    @author Glen Masgai <mimosius@users.sourceforge.net>
*/


BEGIN_DECLARE_EVENT_TYPES()
DECLARE_LOCAL_EVENT_TYPE(wxCSL_EVT_VERSIONCHECK, wxID_ANY)
END_DECLARE_EVENT_TYPES()

#define CSL_EVT_VERSIONCHECK(id,fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
                               wxCSL_EVT_VERSIONCHECK,id,wxID_ANY, \
                               (wxObjectEventFunction)(wxEventFunction) \
                               wxStaticCastEvent(wxCommandEventFunction,&fn), \
                               (wxObject*)NULL \
                             ),


class CslVersionCheckThread : public wxThread
{
    public:
        CslVersionCheckThread(wxEvtHandler *evtHandler);
        ~CslVersionCheckThread();

        bool IsOk() { return m_ok; }

    protected:
        bool m_ok;
        bool m_terminate;
        wxMutex m_mutex;
        wxEvtHandler *m_evtHandler;

        virtual ExitCode Entry();
};

#endif //CSLVERSIONCHECK_H
