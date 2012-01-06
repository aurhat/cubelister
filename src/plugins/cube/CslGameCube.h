/***************************************************************************
 *   Copyright (C) 2007-2011 by Glen Masgai                                *
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

#ifndef CSLGAMECUBE_H
#define CSLGAMECUBE_H

/**
 @author Glen Masgai <mimosius@users.sourceforge.net>
*/

#define CSL_DEFAULT_NAME_CB           wxT("Cube")
#define CSL_LAST_PROTOCOL_CB          122

#define CSL_FOURCC_CB                 "CUBE"

#define CSL_DEFAULT_PORT_CB           28765

#define CSL_DEFAULT_MASTER_CB         wxT("wouter.fov120.com")
#define CSL_DEFAULT_MASTER_PATH_CB    wxT("/cube/masterserver/retrieve.do?item=list")

#define CSL_DEFAULT_INJECT_FILE_CB    wxT("packages/base/metl3.cfg")


class CslGameCube : public CslGame
{
    public:
        CslGameCube();
        virtual ~CslGameCube();

    private:
        const wxChar* GetModeName(wxInt32 mode) const;

        wxInt32 InjectConfig(const wxString& address, const wxString& password, wxString& error);

        //implementations for base class
        wxUint16 GetDefaultGamePort() const { return CSL_DEFAULT_PORT_CB; }
        wxUint16 GetInfoPort(wxUint16 port=0) const { return port ? port+1 : CSL_DEFAULT_PORT_CB+1; }
        inline bool PingDefault(ucharbuf& buf, CslServerInfo& info) const;
        bool ParseDefaultPong(ucharbuf& buf, CslServerInfo& info) const;
        void SetClientSettings(const CslGameClientSettings& settings);
        wxString GameStart(CslServerInfo *info, wxInt32 mode, wxString& error);
        wxInt32 GameEnd(wxString& error);
        void ProcessOutput(char *data) const;
        bool ReturnOk(wxInt32 code) const { return code==0 || code==1; }
};

class CslGameCubePlugin: public CslPlugin
{
    public:
        CslGameCubePlugin(CslPluginHostInfo *hostinfo, CslPluginInfo *plugininfo) :
                CslPlugin(hostinfo, plugininfo) { }
        virtual ~CslGameCubePlugin() { }

        virtual bool Create();
};

#endif //CSLGAMECUBE_H
