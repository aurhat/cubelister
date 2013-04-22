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

#ifndef CSLLISTCTRLINFO_H
#define CSLLISTCTRLINFO_H

#include "CslListCtrl.h"

class CslListCtrlInfo : public CslListCtrl
{
    public:
        CslListCtrlInfo(wxWindow* parent,wxWindowID id,const wxPoint& pos=wxDefaultPosition,
                        const wxSize& size=wxDefaultSize,long style=wxLC_ICON,
                        const wxValidator& validator=wxDefaultValidator,
                        const wxString& name=wxListCtrlNameStr);
        virtual ~CslListCtrlInfo();

        // FIXME let the engine send a delete event and adjust stuff
        CslServerInfo* GetServerInfo() { return m_info; }

        void UpdateServer(CslServerInfo *info) { UpdateServer(info, false); }

    protected:
        void UpdateServer(CslServerInfo *info, bool noresolve);
        // FIXME let the engine send a delete event and adjust stuff
        void Reset();
        void Resolve();
        virtual void GetToolTipText(wxInt32 row, CslToolTipEvent& event);

        wxSize DoGetBestSize() const;
#if wxCHECK_VERSION(2, 9, 0)
        wxSize DoGetBestClientSize() const { return DoGetBestSize(); }
#endif
        CslServerInfo *m_info;
        bool m_resolving;

    private:
        void OnSize(wxSizeEvent& event);
        void OnPong(CslPongEvent& event);
        void OnResolve(CslDNSResolveEvent& event);

        DECLARE_EVENT_TABLE()
};

#endif // CSLLISTCTRLINFO_H
