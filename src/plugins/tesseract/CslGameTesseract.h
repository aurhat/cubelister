/***************************************************************************
 *   Copyright (C) 2007-2014 by Glen Masgai                                *
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

#ifndef CSLGAMETESSERACT_H
#define CSLGAMETESSERACT_H

#define CSL_DEFAULT_NAME_TR        wxT("Tesseract")
#define CSL_LAST_PROTOCOL_TR       1

#define CSL_FOURCC_TR              "TESS"

#define CSL_DEFAULT_PORT_TR        42000
#define CSL_DEFAULT_BCAST_PORT_TR  41998

#define CSL_DEFAULT_MASTER_TR      wxT("tcp://tesseract.gg:41999/list\n")


class CSL_DLL_PLUGINS CslGameTesseract : public CslGame
{
    public:
        CslGameTesseract();
        ~CslGameTesseract();

    private:
        wxString GetVersionName(wxInt32 prot) const;
        const wxChar* GetModeName(wxInt32 mode, wxInt32 prot) const;

        //implementations for base class
        const wxString& GetWeaponName(wxInt32 n, wxInt32 prot) const;
        wxInt32 GetPrivileges(wxInt32 n, wxInt32 prot) const;
        bool ModeHasFlags(wxInt32 mode, wxInt32 prot) const;
        wxInt32 ModeScoreLimit(wxInt32 mode, wxInt32 prot) const;
        wxInt32 GetBestTeam(CslTeamStats& stats, wxInt32 prot) const;
        wxUint16 GetDefaultGamePort() const { return CSL_DEFAULT_PORT_TR; }
        wxUint16 GetInfoPort(wxUint16 port=0) const { return port ? port : CSL_DEFAULT_PORT_TR; }
        wxUint16 GetBroadcastPort() { return CSL_DEFAULT_BCAST_PORT_TR; }
        bool PingDefault(ucharbuf& buf, CslServerInfo& info) const;
        bool PingEx(ucharbuf& buf, CslServerInfo& info) const;
        bool ParseDefaultPong(ucharbuf& buf, CslServerInfo& info) const;
        bool ParsePlayerPong(wxInt32 protocol, ucharbuf& buf, CslPlayerStatsData& info) const;
        bool ParseTeamPong(wxInt32 protocol, ucharbuf& buf, CslTeamStatsData& info) const;
        CslGameClientSettings GuessClientSettings(const wxString& path) const;
        wxString ValidateClientSettings(CslGameClientSettings& settings) const;
        void SetClientSettings(const CslGameClientSettings& settings);
        wxString GameStart(CslServerInfo *info, wxInt32 mode, wxString& error);
        wxInt32 GameEnd(wxString& error);
        bool GetMapImagePaths(wxArrayString& paths) const;
};

#endif //CSLGAMETESSERACT_H
