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

#ifndef CSLGAMEASSAULTCUBE_H
#define CSLGAMEASSAULTCUBE_H

#define CSL_DEFAULT_NAME_AC         wxT("AssaultCube")
#define CSL_LAST_PROTOCOL_AC        1132

#define CSL_FOURCC_AC               "ATCE"

#define CSL_DEFAULT_PORT_AC         28763
#define CSL_DEFAULT_BCAST_PORT_AC   28762

#define CSL_DEFAULT_MASTER_AC       wxT("tcp://assault.cubers.net:28760/list\n")

#define CSL_DEFAULT_INJECT_DIR_AC   wxT("config/")
#define CSL_DEFAULT_INJECT_FILE_AC  wxT("autoexec.cfg")


class CSL_DLL_PLUGINS CslGameAssaultCube : public CslGame
{
    public:
        CslGameAssaultCube();
        ~CslGameAssaultCube();

    private:
        wxString GetVersionName(wxInt32 prot) const;
        const wxChar* GetModeName(wxInt32 mode) const;

        //implementations for base class
        const wxString& GetWeaponName(wxInt32 n, wxInt32 prot) const;
        bool ModeHasFlags(wxInt32 mode, wxInt32 prot) const { return mode==5 || (mode>=13 && mode<=15); }
        wxInt32 GetBestTeam(CslTeamStats& stats, wxInt32 prot) const;
        wxUint16 GetDefaultGamePort() const { return CSL_DEFAULT_PORT_AC; }
        wxUint16 GetInfoPort(wxUint16 port=0) const { return port ? port+1 : CSL_DEFAULT_PORT_AC+1; }
        wxUint16 GetBroadcastPort() { return CSL_DEFAULT_BCAST_PORT_AC; }
        bool PingDefault(ucharbuf& buf, CslServerInfo& info) const;
        bool ParseDefaultPong(ucharbuf& buf, CslServerInfo& info) const;
        bool ParsePlayerPong(wxInt32 protocol, ucharbuf& buf, CslPlayerStatsData& info) const;
        bool ParseTeamPong(wxInt32 protocol, ucharbuf& buf, CslTeamStatsData& info) const;
        CslGameClientSettings GuessClientSettings(const wxString& path) const;
        wxString ValidateClientSettings(CslGameClientSettings& settings) const;
        void SetClientSettings(const CslGameClientSettings& settings);
        wxString GameStart(CslServerInfo *info, wxInt32 mode, wxString& error);
        wxInt32 GameEnd(wxString& error);
};

#endif //CSLGAMEASSAULTCUBE_H
