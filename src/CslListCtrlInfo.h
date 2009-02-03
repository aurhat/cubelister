/***************************************************************************
 *   Copyright (C) 2007 -2009 by Glen Masgai                               *
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

#ifndef CSLLISTCTRLINFO_H
#define CSLLISTCTRLINFO_H

/**
 @author Glen Masgai <mimosius@gmx.de>
*/


#include "CslListCtrl.h"
#include "engine/CslGame.h"


class CslListCtrlInfo : public CslListCtrl
{
    public:
        CslListCtrlInfo(wxWindow* parent,wxWindowID id,const wxPoint& pos=wxDefaultPosition,
                        const wxSize& size=wxDefaultSize,long style=wxLC_ICON,
                        const wxValidator& validator=wxDefaultValidator,
                        const wxString& name=wxListCtrlNameStr);

        void UpdateInfo(CslServerInfo *info);

    private:
        wxImageList m_imgList;

        void OnSize(wxSizeEvent& event);

        DECLARE_EVENT_TABLE()

    protected:
        void AdjustSize(wxSize size);
        void GetToolTipText(wxInt32 row,CslToolTipEvent& event);
};

#endif // CSLLISTCTRLINFO_H
