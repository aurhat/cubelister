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

#ifndef CSLPLUGIN_H
#define CSLPLUGIN_H

#define CSL_PLUGIN_VERSION_API  3

#define CSL_PLUGIN_EVENT_ID_MIN_GUI     (wxID_HIGHEST + 20000)
#define CSL_PLUGIN_EVENT_ID_MIN_ENGINE  (wxID_HIGHEST + 40000)

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EXPORTED_EVENT_TYPE(CSL_DLL_PLUGIN, wxCSL_EVT_PLUGIN, wxID_ANY)
END_DECLARE_EVENT_TYPES()

class CslPluginEvent;

typedef void (wxEvtHandler::*CslPluginEventFunction)(CslPluginEvent&);

#define CslPluginEventHandler(fn) \
    (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(CslPluginEventFunction, &fn)

#define CSL_EVT_PLUGIN(id, fn)                                 \
    DECLARE_EVENT_TABLE_ENTRY(                                 \
                               wxCSL_EVT_PLUGIN, id, wxID_ANY, \
                               CslPluginEventHandler(fn),      \
                               (wxObject*)NULL                 \
                             ),


class CSL_DLL_PLUGIN CslPluginEvent : public wxEvent
{
    public:
        enum { EVT_SERVER_MENU = 0, EVT_PLAYER_MENU };

        CslPluginEvent(wxInt32 id = wxID_ANY) :
                wxEvent(id, wxCSL_EVT_PLUGIN),
                m_menu(NULL),
                m_serverInfo(NULL),
                m_playerStatsData(NULL)
            { }

        virtual wxEvent* Clone() const
            { return new CslPluginEvent(*this); }

        wxMenu* GetMenu() const { return m_menu; }

        CslServerInfo* GetServerInfo() const
            { return m_serverInfo; }
        CslPlayerStatsData* GetPlayerStatsData() const
            { return m_playerStatsData; }

        void SetMenu(wxMenu *menu) { m_menu = menu; }

        void SetServerInfo(CslServerInfo *info)
            { m_serverInfo = info; }
        void SetPlayerStatsData(CslPlayerStatsData *data)
            { m_playerStatsData = data; }

    private:
        wxMenu *m_menu;

        CslServerInfo *m_serverInfo;
        CslPlayerStatsData *m_playerStatsData;

        DECLARE_DYNAMIC_CLASS_NO_ASSIGN(CslPluginEvent)
};


enum { CSL_PLUGIN_TYPE_ENGINE, CSL_PLUGIN_TYPE_GUI };

#define IMPLEMENT_PLUGIN(api, type, fcc, name, version,                \
                         author, email, website, license, description, \
                         init, deinit, config)                         \
    extern "C" \
    { \
        WXEXPORT CslPluginInfo csl_plugin_info = \
        { \
            api,         \
            type,        \
            fcc,         \
            name,        \
            version,     \
            author,      \
            email,       \
            website,     \
            license,     \
            description, \
            init,        \
            deinit,      \
            config,      \
        }; \
    }

class CslEngine;
class CslGame;

class CSL_DLL_PLUGIN CslPluginHost
{
    public:
        virtual wxWindow* GetMainWindow() { return NULL; }
        virtual wxEvtHandler* GetEvtHandler() { return NULL; }
        virtual wxString GetHomeDir(wxPathFormat format = wxPATH_NATIVE)
            { return wxEmptyString; }

        virtual wxInt32 GetFreeId() { return -1; }
        virtual wxInt32 GetFreeIds(wxInt32 count, wxInt32 ids[]) { return -1; }
#if wxUSE_GUI
        virtual wxMenu* GetPluginMenu() { return NULL; }
        virtual wxMenuBar* GetMainMenuBar() { return NULL; }
#endif //wxUSE_GUI

        virtual CslEngine* GetCslEngine() { return NULL; }
        virtual CslGame* GetSelectedGame() { return NULL; }
};

typedef bool (*CslPluginInitFn)(CslPluginHost*);
typedef void (*CslPluginDeinitFn)(CslPluginHost*);
typedef void (*CslPluginConfigFn)(CslPluginHost*);

typedef struct
{
    wxUint32 APIVersion;
    wxInt32 Type;
    wxUint32 FourCC;
    const wxChar *Name;
    const wxChar *Version;
    const wxChar *Author;
    const wxChar *Email;
    const wxChar *Website;
    const wxChar *License;
    const wxChar *Description;
    CslPluginInitFn InitFn;
    CslPluginDeinitFn DeinitFn;
    CslPluginConfigFn ConfigFn;
} CslPluginInfo;


class CSL_DLL_PLUGIN CslPlugin
{
    friend class CslPluginMgr;

    public:
        CslPlugin(const wxString& filename, CslPluginHost *host);
        ~CslPlugin();

        bool IsLoaded() { return m_handle.IsLoaded(); }

    private:
        wxString m_fileName;
        wxDynamicLibrary m_handle;
        CslPluginInfo *m_pluginInfo;
};

WX_DEFINE_USER_EXPORTED_ARRAY(CslPlugin*, CslPlugins, class CSL_DLL_PLUGIN);


class CSL_DLL_PLUGIN CslPluginMgr
{
    public:
        CslPluginMgr(wxUint32 type);

        wxInt32 LoadPlugins(CslPluginHost *host);
        void UnloadPlugins(CslPluginHost *host);

    private:
        wxInt32 m_type;
        wxString m_extension;
        CslPlugins m_plugins;
};

#endif // CSLPLUGIN_H
