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

#ifndef CSLGAMECONNECTION_H
#define CSLGAMECONNECTION_H

/**
 @author Glen Masgai <mimosius@users.sourceforge.net>
*/

#include <wx/wxprec.h>
#ifdef __BORLANDC__
#pragma hdrstop
#endif
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include "engine/CslGame.h"


class CslGameConnection
{
    public:
        enum { NO_PASS, USE_PASS, ASK_PASS };

        static void Reset();
        static void CountDown();
        static bool Prepare(CslServerInfo *info,wxInt32 pass=NO_PASS);
        static bool Connect();
        static bool IsPlaying() { return m_playing; }
        static bool IsWaiting() { return m_waitTime>0; }
        static wxInt32 GetWaitTime() { return m_waitTime; }
        static CslServerInfo* GetInfo() { return m_info; }

    private:
        static bool m_playing;
        static wxInt32 m_waitTime,m_connectMode;
        static CslServerInfo *m_info;
};

#endif //CSLGAMECONNECTION_H
