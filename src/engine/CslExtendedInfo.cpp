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
#include "CslExtendedInfo.h"

CslPlayerStats::CslPlayerStats() : m_status(CSL_STATS_NEED_IDS),
        m_lastPing(0),m_lastPong(0)
{
}

CslPlayerStats::~CslPlayerStats()
{
    DeleteStats();
}

CslPlayerStatsData* CslPlayerStats::GetNewStats()
{
    loopv(m_stats)
    {
        if (m_stats[i]->Ok)
            continue;
        return m_stats[i];
    }
    return new CslPlayerStatsData;
}

bool CslPlayerStats::AddStats(CslPlayerStatsData *data)
{
    if (m_status==CSL_STATS_NEED_STATS)
    {
        loopv(m_ids)
        {
            if (m_ids[i]!=data->ID)
                continue;
            m_ids.erase(m_ids.begin()+i);
            data->Ok=true;
            if (m_ids.size()==0)
                m_status=CSL_STATS_NEED_IDS;
            loopvj(m_stats) if (m_stats[j]==data) return true;
            m_stats.push_back(data);
            return true;
        }
    }

    return false;
}

void CslPlayerStats::RemoveStats(CslPlayerStatsData *data)
{
    loopv(m_stats)
    {
        if (m_stats[i]==data)
        {
            m_stats[i]->Ok=false;
            return;
        }
    }
    delete data;
}

void CslPlayerStats::DeleteStats()
{
    m_status=CSL_STATS_NEED_IDS;
    WX_CLEAR_ARRAY(m_stats);
}

bool CslPlayerStats::AddId(wxInt32 id)
{
    if (m_status!=CSL_STATS_NEED_IDS)
        return false;
    loopv(m_ids) if (m_ids[i]==id) return false;
    m_ids.push_back(id);
    return true;
}

bool CslPlayerStats::RemoveId(wxInt32 id)
{
    loopv(m_ids)
    {
        if (m_ids[i]!=id)
            continue;
        m_ids.erase(m_ids.begin()+i);
        if (m_ids.size()==0)
            m_status=CSL_STATS_NEED_IDS;
        return true;
    }
    return false;
}

void CslPlayerStats::SetWaitForStats()
{
    if (m_ids.size())
        m_status=CSL_STATS_NEED_STATS;
    else
        m_status=CSL_STATS_NEED_IDS;
}

void CslPlayerStats::Reset()
{
    m_status=CSL_STATS_NEED_IDS;
    loopv(m_stats) m_stats[i]->Ok=false;
    m_ids.Empty();
}


CslTeamStats::CslTeamStats() :
        TeamMode(false),TimeRemain(-1),
        LastPing(0),LastPong(0)
{
}

CslTeamStats::~CslTeamStats()
{
    DeleteStats();
}

CslTeamStatsData* CslTeamStats::GetNewStats()
{
    loopv(m_stats)
    {
        if (m_stats[i]->Ok)
            continue;
        return m_stats[i];
    }
    return new CslTeamStatsData;
}

void CslTeamStats::AddStats(CslTeamStatsData *data)
{
    data->Ok=true;
    loopv(m_stats) if (m_stats[i]==data) return;
    m_stats.push_back(data);
}

void CslTeamStats::RemoveStats(CslTeamStatsData *data)
{
    loopv(m_stats)
    {
        if (m_stats[i]==data)
        {
            m_stats[i]->Ok=false;
            return;
        }
    }
    delete data;
}

void CslTeamStats::DeleteStats()
{
    WX_CLEAR_ARRAY(m_stats);
}

void CslTeamStats::Reset()
{
    loopv(m_stats) m_stats[i]->Reset();
}
