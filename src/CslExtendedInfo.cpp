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

#include "CslExtendedInfo.h"

CslPlayerStats::CslPlayerStats() : m_status(CSL_STATS_NEED_IDS),
        m_lastRefresh(0),m_lastResponse(0)
{
    m_critical=new wxCriticalSection();
}

CslPlayerStats::~CslPlayerStats()
{
    DeleteStats();
    delete m_critical;
}

CslPlayerStatsData* CslPlayerStats::GetNewStatsPtr()
{
    wxCriticalSectionLocker enter(*m_critical);

    loopv(m_stats)
    {
        if (m_stats[i]->m_ok) continue;
        return m_stats[i];
    }
    return new CslPlayerStatsData;
}

bool CslPlayerStats::AddStats(CslPlayerStatsData *data)
{
    wxCriticalSectionLocker enter(*m_critical);

    if (m_status!=CSL_STATS_NEED_STATS)
        return false;

    loopv(m_ids)
    {
        if (m_ids[i]!=data->m_id)
            continue;
        m_ids.remove(i);
        data->m_ok=true;
        if (m_ids.length()==0)
            m_status=CSL_STATS_NEED_IDS;
        loopv(m_stats) if (m_stats[i]==data) return true;
        m_stats.add(data);
        return true;
    }
    return false;
}

void CslPlayerStats::RemoveStats(CslPlayerStatsData *data)
{
    wxCriticalSectionLocker enter(*m_critical);

    loopv(m_stats)
    {
        if (m_stats[i]==data)
        {
            m_stats[i]->m_ok=false;
            return;
        }
    }
    delete data;
}

void CslPlayerStats::DeleteStats()
{
    wxCriticalSectionLocker enter(*m_critical);

    m_status=CSL_STATS_NEED_IDS;
    loopvrev(m_stats) delete m_stats[i];
    m_stats.setsize(0);
}

bool CslPlayerStats::AddId(wxInt32 id)
{
    wxCriticalSectionLocker enter(*m_critical);

    if (m_status!=CSL_STATS_NEED_IDS)
        return false;
    loopv(m_ids) if (m_ids[i]==id)
        return false;
    m_ids.add(id);
    return true;
}

bool CslPlayerStats::RemoveId(wxInt32 id)
{
    wxCriticalSectionLocker enter(*m_critical);

    loopv(m_ids)
    {
        if (m_ids[i]!=id)
            continue;
        m_ids.remove(i);
        if (m_ids.length()==0)
            m_status=CSL_STATS_NEED_IDS;
        return true;
    }
    return false;
}

void CslPlayerStats::SetWaitForStats()
{
    if (m_ids.length())
        m_status=CSL_STATS_NEED_STATS;
    else
        m_status=CSL_STATS_NEED_IDS;
}

void CslPlayerStats::Reset()
{
    wxCriticalSectionLocker enter(*m_critical);

    m_status=CSL_STATS_NEED_IDS;
    loopv(m_stats) m_stats[i]->m_ok=false;
    loopv(m_ids) m_ids.remove(0);
}



CslTeamStats::CslTeamStats() :
        m_teamplay(false),m_remain(-1),
        m_lastRefresh(0),m_lastResponse(0)
{
    m_critical=new wxCriticalSection();
}

CslTeamStats::~CslTeamStats()
{
    DeleteStats();
    delete m_critical;
}

CslTeamStatsData* CslTeamStats::GetNewStatsPtr()
{
    wxCriticalSectionLocker enter(*m_critical);

    loopv(m_stats)
    {
        if (m_stats[i]->m_ok) continue;
        return m_stats[i];
    }
    return new CslTeamStatsData;
}

void CslTeamStats::AddStats(CslTeamStatsData *data)
{
    wxCriticalSectionLocker enter(*m_critical);

    data->m_ok=true;
    loopv(m_stats) if (m_stats[i]==data) return;
    m_stats.add(data);
}

void CslTeamStats::DeleteStats()
{
    wxCriticalSectionLocker enter(*m_critical);

    loopvrev(m_stats) delete m_stats[i];
    m_stats.setsize(0);
}

void CslTeamStats::Reset()
{
    wxCriticalSectionLocker enter(*m_critical);

    loopv(m_stats) m_stats[i]->Reset();
}
