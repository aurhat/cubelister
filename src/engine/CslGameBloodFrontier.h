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

#ifndef CSLBLOODFRONTIER_H
#define CSLBLOODFRONTIER_H

/**
 @author Glen Masgai <mimosius@users.sourceforge.net>
*/

#define CSL_DEFAULT_NAME_BF           wxT("Blood Frontier")
#define CSL_LAST_PROTOCOL_BF          157

#define CSL_DEFAULT_PORT_BF           28795

#define CSL_DEFAULT_MASTER_BF         wxT("bloodfrontier.com")
#define CSL_DEFAULT_MASTER_PORT_BF    28800


class CslBloodFrontier : public CslGame
{
    public:
        CslBloodFrontier();
        ~CslBloodFrontier();

    private:
        const wxChar* GetVersionName(wxInt32 prot) const;
        wxString GetModeName(wxInt32 n,wxInt32 m) const;

        //implementations for base class
        void GetPlayerstatsDescriptions(vector<wxString>& desc) const;
        const wxChar* GetWeaponName(wxInt32 n) const;
        wxInt32 GetBestTeam(CslTeamStats& stats,wxInt32 prot) const;
        wxUint16 GetDefaultGamePort() const { return CSL_DEFAULT_PORT_BF; }
        wxUint16 GetInfoPort(wxUint16 port=0) const { return port ? port+1:CSL_DEFAULT_PORT_BF+1; }
        bool ParseDefaultPong(ucharbuf& buf,CslServerInfo& info) const;
        bool ParsePlayerPong(wxUint32 protocol,ucharbuf& buf,CslPlayerStatsData& info) const ;
        bool ParseTeamPong(wxUint32 protocol,ucharbuf& buf,CslTeamStatsData& info) const;
        void SetClientSettings(const CslGameClientSettings& settings);
        wxString GameStart(CslServerInfo *info,wxUint32 mode,wxString& error);
        wxInt32 GameEnd(wxString& error);
};

#endif //CSLBLOODFRONTIER_H
