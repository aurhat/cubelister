/***************************************************************************
 *   Copyright (C) 2007 by Glen Masgai                                     *
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

#ifndef CSLCONNECTIONSTATE_H
#define CSLCONNECTIONSTATE_H

/**
 @author Glen Masgai <mimosius@gmx.de>
*/

#include <wx/wxprec.h>
#ifdef __BORLANDC__
#pragma hdrstop
#endif
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/imaglist.h>
#include <wx/listctrl.h>
#include "engine/CslEngine.h"
#include "CslStatusBar.h"


class CslConnectionState
{
    public:
        static void Reset()
        {
            if (m_activeInfo)
				m_activeInfo->SetWaiting(false);
            m_playing=false;
            m_waitTime=0;
			m_connectMode=CslGame::CSL_CONNECT_DEFAULT;
            m_activeInfo=NULL;
            CslStatusBar::SetText(1,wxEmptyString);
        }

        static void CreateWaitingState(CslServerInfo *info,wxInt32 mode,wxInt32 time)
        {
            m_activeInfo=info;
            m_waitTime=time;
			m_connectMode=mode;
            info->SetWaiting(true);
        }

        static bool CountDown()
        {
            if (--m_waitTime==0)
            {
                Reset();
                return false;
            }
            return true;
        }

        static void CreatePlayingState(CslServerInfo *info)
        {
            m_activeInfo=info;
            m_playing=true;
        }

		static void CreateConnectState(CslServerInfo *info,wxInt32 mode=CslGame::CSL_CONNECT_DEFAULT)
		{
			m_activeInfo=info;
			m_connectMode=mode;
		}

        static bool IsPlaying() { return m_playing; }
        static bool IsWaiting() { return m_waitTime>0; }
        static wxInt32 GetWaitTime() { return m_waitTime; }
        static wxInt32 GetConnectMode() { return m_connectMode; }
        static CslServerInfo* GetInfo() { return m_activeInfo; }

    private:
        static bool m_playing;
        static wxInt32 m_waitTime,m_connectMode;
        static CslServerInfo *m_activeInfo;
};

#endif //CSLCONNECTIONSTATE_H
