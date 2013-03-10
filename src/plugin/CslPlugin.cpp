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

#include "Csl.h"
#include "CslPlugin.h"

CslPlugin::CslPlugin(const wxString& filename) :
        m_fileName(filename), m_handle(filename), m_pluginInfo(NULL)
{
    if (m_handle.IsLoaded())
    {
        if (m_handle.HasSymbol(wxT("csl_plugin_info")))
            m_pluginInfo = (CslPluginInfo*)m_handle.GetSymbol(wxT("csl_plugin_info"));
        else
            m_handle.Unload();
    }
}

CslPlugin::~CslPlugin()
{
    if (m_handle.IsLoaded())
        m_handle.Unload();
}

CslPluginMgr::CslPluginMgr(wxUint32 type) :
        m_type(type)
{
    m_extension = wxDynamicLibrary::CanonicalizeName(wxT("*"), wxDL_MODULE);
}

wxInt32 CslPluginMgr::LoadPlugins(CslPluginHost *host)
{
    wxInt32 c = 0;
    wxString path;
    wxArrayString files;
    wxArrayString dirs=GetPluginDirs();

    for (wxInt32 i = 0, l = dirs.GetCount(); i<l; i++)
    {
        wxString path = dirs.Item(i)+wxT("plugins");
        c += FindFiles(path, m_extension, files);
    }

    files.Sort(true);

    CSL_LOG_DEBUG("found %d possible plugins\n", c);

    while (c>0)
    {
        const wxString& file = files.Item(--c);

        CslPlugin *p = new CslPlugin(file);

        if (!p->IsLoaded())
        {
            CSL_LOG_DEBUG("Couldn't load plugin %s\n", U2C(file));
            goto fail;
        }
        if (p->m_pluginInfo->APIVersion!=CSL_PLUGIN_VERSION_API)
        {
            CSL_LOG_DEBUG("Couldn't load plugin %s. Invalid API version (%d!=%d)\n",
                          U2C(file), p->m_pluginInfo->APIVersion!=CSL_PLUGIN_VERSION_API);
            goto fail;
        }
        if (p->m_pluginInfo->Type<CSL_PLUGIN_TYPE_ENGINE ||
            p->m_pluginInfo->Type>CSL_PLUGIN_TYPE_GUI)
        {
            CSL_LOG_DEBUG("Couldn't load plugin %s. Invalid Type (%d)\n",
                          U2C(file), p->m_pluginInfo->Type);
            goto fail;
        }
        if (p->m_pluginInfo->Type!=m_type)
            goto fail;

        loopv(m_plugins)
        {
            if (m_plugins[i]->m_pluginInfo->FourCC==p->m_pluginInfo->FourCC)
            {
                CSL_LOG_DEBUG("Couldn't load plugin %s. Plugin was already loaded.\n", U2C(file));
                goto fail;
            }
        }
        if (!p->m_pluginInfo->InitFn(host))
        {
            CSL_LOG_DEBUG("Couldn't load plugin %s. Create() failed.\n", U2C(file));
            goto fail;
        }

        m_plugins.push_back(p);

        CSL_LOG_DEBUG("loaded plugin %s (%s / %s)\n", U2C(file),
                      U2C(p->m_pluginInfo->Name),
                      U2C(p->m_pluginInfo->Version));

        continue;

    fail:
        delete p;
    }

    return m_plugins.size();
}

void CslPluginMgr::UnloadPlugins(CslPluginHost *host)
{
    loopvrev(m_plugins)
    {
        CslPlugin *p = m_plugins[i];

        if (p->m_pluginInfo->DeinitFn)
            p->m_pluginInfo->DeinitFn(host);

        delete p;
    }

    m_plugins.Empty();
}
