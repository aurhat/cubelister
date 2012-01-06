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

#ifndef CSLPLUGIN_H
#define CSLPLUGIN_H

#define CSL_PLUGIN_VERSION_API     1

#ifdef __WXMSW__
#define CSL_LIBRARY_EXTENSION wxT("*.dll")
#elif __WXMAC__
#define CSL_LIBRARY_EXTENSION wxT("*.dylib")
#else
#define CSL_LIBRARY_EXTENSION wxT("*.so")
#endif

#define IMPLEMENT_PLUGIN(fcc, api, type, version, name, author, email, website, license, description, class) \
    extern "C" \
    { \
        WXEXPORT CslPluginInfo* csl_init_plugin(CslPluginHostInfo *host) \
        { \
            static CslPluginInfo info; \
            \
            info.FourCC=fcc; \
            info.APIVersion=api; \
            info.Type=type; \
            info.Version=version; \
            info.Name=name; \
            info.Author=author; \
            info.Email=email; \
            info.Website=website; \
            info.License=license; \
            info.Description=description; \
            info.Plugin=new class(host, &info); \
            \
            return &info; \
        } \
    }

class CslPlugin;

typedef struct
{
    wxInt32 Type;
    wxUint32 APIVersion;
    wxUint32 FourCC;
    wxUint32 Version;
    wxString Name;
    wxString Author;
    wxString Email;
    wxString Website;
    wxString License;
    wxString Description;
    CslPlugin *Plugin;
} CslPluginInfo;


class CslPluginHostInfo
{
    public:
        virtual wxWindow* GetMainWindow() { return NULL; }
        virtual wxEvtHandler* GetEvtHandler()  { return NULL; }

        virtual wxInt32 GetFreeId() { return -1; }
        virtual wxMenu* GetPluginMenu()  { return NULL; }
        virtual wxMenuBar* GetMainMenuBar()  { return NULL; }

        virtual CslEngine* GetCslEngine()  { return NULL; }
        virtual CslGame* GetSelectedGame()  { return NULL; }
};


class CslPlugin: public wxEvtHandler
{
    public:
        enum { TYPE_NONE, TYPE_ENGINE, TYPE_GUI };

        CslPlugin(CslPluginHostInfo *host, CslPluginInfo *plugin) :
                wxEvtHandler(),
                m_plugin(plugin), m_host(host) { }
        virtual ~CslPlugin() { };

        virtual bool Create() = 0;
        virtual void CreateMenu(wxMenu *menu) { };

    protected:
        CslPluginInfo *m_plugin;
        CslPluginHostInfo *m_host;
};


typedef CslPluginInfo* (*PluginInitFunction)(CslPluginHostInfo*);

class CslPluginObject
{
    public:
        CslPluginObject(const wxString& filename, CslPluginHostInfo *hostinfo);
        ~CslPluginObject();

        bool IsLoaded() { return m_handle.IsLoaded(); }

        friend class CslPluginMgr;

    private:
        wxInt32 m_id;
        wxString m_fileName;
        wxDynamicLibrary m_handle;
        CslPluginInfo *m_initInfo;

        PluginInitFunction m_initFunc;
};

typedef vector<CslPluginObject*> CslPlugins;


class CslPluginMgr
{
    public:
        CslPluginMgr(wxUint32 type=CslPlugin::TYPE_NONE) : m_type(type) { }

        wxInt32 LoadPlugins(CslPluginHostInfo *hostinfo);
        void UnloadPlugins();

        void BuildMenu(wxMenu *menu);

    private:
        wxInt32 m_type;
        CslPlugins m_plugins;
};

#endif // CSLPLUGIN_H
