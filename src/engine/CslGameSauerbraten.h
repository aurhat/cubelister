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

#ifndef CSLGAMESAUERBRATEN_H
#define CSLGAMESAUERBRATEN_H

/**
 @author Glen Masgai <mimosius@gmx.de>
*/

#define CSL_DEFAULT_NAME_SB           wxT("Sauerbraten")
#define CSL_LAST_PROTOCOL_SB          256

#define CSL_DEFAULT_PORT_SB           28785

#define CSL_DEFAULT_MASTER_SB         wxT("sauerbraten.org")
#define CSL_DEFAULT_MASTER_PATH_SB    wxT("/masterserver/retrieve.do?item=list")

#define CSL_DEFAULT_INJECT_DIR_SB     wxT("packages/base/")
#define CSL_DEFAULT_INJECT_FIL_SB     wxT("csl_start_sb")


class CslGameSauerbraten : public CslGame
{
    public:
        CslGameSauerbraten();
        ~CslGameSauerbraten();

    private:
        bool m_injected;

        wxString GetVersionName(wxInt32 n) const;
        wxString GetModeName(wxInt32 n) const;

        wxInt32 InjectConfig(const wxString& address,wxString *error);

        //implementations for base class
        wxString GetWeaponName(wxInt32 n) const;
        wxUint16 GetDefaultPort() const { return CSL_DEFAULT_PORT_SB; }
        bool ParseDefaultPong(ucharbuf& buf,CslServerInfo& info) const;
        bool ParsePlayerPong(wxUint32 protocol,ucharbuf& buf,CslPlayerStatsData& info) const;
        bool ParseTeamPong(wxUint32 protocol,ucharbuf& buf,CslTeamStatsData& info) const;
        void SetClientSettings(const CslGameClientSettings& settings);
        wxString GameStart(CslServerInfo *info,wxUint32 mode,wxString *error);
        wxInt32 GameEnd(wxString *error=NULL);
        const char** GetIcon(wxInt32 size) const;
};

#endif //CSLGAMESAUERBRATEN_H
