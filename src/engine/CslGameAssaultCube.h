/***************************************************************************
 *   Copyright (C) 2007 -2009 by Glen Masgai                               *
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

#ifndef CSLGAMEASSAULTCUBE_H
#define CSLGAMEASSAULTCUBE_H

/**
 @author Glen Masgai <mimosius@users.sourceforge.net>
*/

#define CSL_DEFAULT_NAME_AC           wxT("AssaultCube")
#define CSL_LAST_PROTOCOL_AC          1128

#define CSL_DEFAULT_PORT_AC           28763

#define CSL_DEFAULT_MASTER_AC         wxT("masterserver.cubers.net")
#define CSL_DEFAULT_MASTER_PATH_AC    wxT("/cgi-bin/actioncube.pl/retrieve.do?item=list")

#define CSL_DEFAULT_INJECT_DIR_AC     wxT("config/")
#define CSL_DEFAULT_INJECT_FILE_AC    wxT("autoexec.cfg")


class CslGameAssaultCube : public CslGame
{
    public:
        CslGameAssaultCube();
        ~CslGameAssaultCube();

    private:
        const wxChar* GetVersionName(wxInt32 prot) const;
        const wxChar* GetModeName(wxInt32 mode) const;

        //implementations for base class
        const wxChar* GetWeaponName(wxInt32 n) const;
        bool ModeHasFlags(wxInt32 mode,wxInt32 prot) const { return mode==5 || (mode>=13 && mode<=15); }
        wxInt32 GetBestTeam(CslTeamStats& stats,wxInt32 prot) const;
        wxUint16 GetDefaultGamePort() const { return CSL_DEFAULT_PORT_AC; }
        wxUint16 GetInfoPort(wxUint16 port=0) const { return port ? port+1:CSL_DEFAULT_PORT_AC+1; }
        bool ParseDefaultPong(ucharbuf& buf,CslServerInfo& info) const;
        bool ParsePlayerPong(wxUint32 protocol,ucharbuf& buf,CslPlayerStatsData& info) const;
        bool ParseTeamPong(wxUint32 protocol,ucharbuf& buf,CslTeamStatsData& info) const;
        void SetClientSettings(const CslGameClientSettings& settings);
        wxString GameStart(CslServerInfo *info,wxUint32 mode,wxString *error);
        wxInt32 GameEnd(wxString *error=NULL);
        const char** GetIcon(wxInt32 size) const;
};

#endif //CSLGAMEASSAULTCUBE_H
