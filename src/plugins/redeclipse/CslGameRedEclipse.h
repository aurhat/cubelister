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

#ifndef CSLREDECLIPSE_H
#define CSLREDECLIPSE_H

#define CSL_DEFAULT_NAME_RE        wxT("Red Eclipse")
#define CSL_LAST_PROTOCOL_RE       220

#define CSL_FOURCC_RE              "RDEE"

#define CSL_DEFAULT_PORT_RE        28801
#define CSL_DEFAULT_BCAST_PORT_RE  28799

#define CSL_DEFAULT_MASTER_RE      wxT("tcp://play.redeclipse.net:28800/list\n")


class CSL_DLL_PLUGINS CslGameRedEclipse : public CslGame
{
    public:
        CslGameRedEclipse();
        ~CslGameRedEclipse();

    private:
        wxString GetVersionName(wxInt32 prot) const;
        wxString GetModeName(wxInt32 mode, wxInt32 muts) const;

        //implementations for base class
        wxInt32 GetBestTeam(CslTeamStats& stats, wxInt32 prot) const;
        wxUint16 GetDefaultGamePort() const { return CSL_DEFAULT_PORT_RE; }
        wxUint16 GetInfoPort(wxUint16 port=0) const { return port ? port+1 : CSL_DEFAULT_PORT_RE+1; }
        wxUint16 GetBroadcastPort() { return CSL_DEFAULT_BCAST_PORT_RE; }
        bool PingDefault(ucharbuf& buf, CslServerInfo& info) const;
        bool ParseDefaultPong(ucharbuf& buf, CslServerInfo& info) const;
        CslGameClientSettings GuessClientSettings(const wxString& path) const;
        wxString ValidateClientSettings(CslGameClientSettings& settings) const;
        void SetClientSettings(const CslGameClientSettings& settings);
        wxString GameStart(CslServerInfo *info, wxInt32 mode, wxString& error);
        wxInt32 GameEnd(wxString& error);
};

#endif //CSLREDECLIPSE_H
