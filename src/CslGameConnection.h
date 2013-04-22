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

#ifndef CSLGAMECONNECTION_H
#define CSLGAMECONNECTION_H

#include "CslGameProcess.h"

class CslGameConnection
{
    private:
        CslGameConnection() :
                m_info(NULL), m_process(NULL),
                m_locked(false), m_playing(false), m_detached(false) { }
        ~CslGameConnection() { Reset(); }

        static CslGameConnection& GetInstance();

    public:
        enum { NO_PASS, USE_PASS, ASK_PASS };

        static void Reset(CslServerInfo *info=NULL);
        static bool CountDown();
        static bool Prepare(CslServerInfo *info,wxInt32 pass=NO_PASS);
        static bool Connect();
        static void Detach();
        static bool IsPlaying() { return GetInstance().m_playing; }
        static bool IsWaiting()
        {
            return GetInstance().m_info ?
                   GetInstance().m_info->ConnectWait>0 : false;
        }
        static CslServerInfo* GetInfo() { return GetInstance().m_info; }

    private:
        wxString m_cmd;
        CslServerInfo *m_info;
        CslGameProcess *m_process;
        bool m_locked, m_playing, m_detached;
};

#endif //CSLGAMECONNECTION_H
