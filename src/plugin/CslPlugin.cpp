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

#include "Csl.h"
#include "CslEngine.h"
#include "CslPlugin.h"

CslPluginObject::CslPluginObject(const wxString& filename, CslPluginHostInfo *hostinfo) :
        m_fileName(filename), m_handle(filename), m_initInfo(NULL), m_initFunc(NULL)
{
    if (m_handle.IsLoaded())
    {
        if (m_handle.HasSymbol(wxT("csl_init_plugin")) &&
            (m_initFunc=(PluginInitFunction)m_handle.GetSymbol(wxT("csl_init_plugin"))))
        {
            m_initInfo=m_initFunc(hostinfo);
            return;
        }

        m_handle.Unload();
    }
}

CslPluginObject::~CslPluginObject()
{
    if (m_initInfo)
        delete m_initInfo->Plugin;
}


wxInt32 CslPluginMgr::LoadPlugins(CslPluginHostInfo *hostinfo)
{
    wxInt32 c=0;
    wxString path;
    wxArrayString files;
    wxArrayString dirs=GetPluginDirs();

    for (wxInt32 i=0, l=dirs.GetCount(); i<l; i++)
    {
        wxString path=dirs.Item(i)+wxT("plugins");
        c+=FindFiles(path, CSL_LIBRARY_EXTENSION, files);
    }

    files.Sort(true);

    LOG_DEBUG("found %d possible plugins\n", c);

    while (c>0)
    {
        const wxString& file=files.Item(--c);

        CslPluginObject *p=new CslPluginObject(file, hostinfo);

        if (!p->IsLoaded())
        {
            LOG_DEBUG("Couldn't load plugin %s\n", U2A(file));
            goto fail;
        }
        if (p->m_initInfo->APIVersion!=CSL_PLUGIN_VERSION_API)
        {
            LOG_DEBUG("Couldn't load plugin %s. Invalid API version (%d!=%d)\n",
                      U2A(file), p->m_initInfo->APIVersion!=CSL_PLUGIN_VERSION_API);
            goto fail;
        }
        if (p->m_initInfo->Type!=m_type)
        {
            LOG_DEBUG("Couldn't load plugin %s. Invalid Type (%d!=%d)\n",
                      U2A(file), p->m_initInfo->Type, m_type);
            goto fail;
        }
        loopv(m_plugins)
        {
            if (m_plugins[i]->m_initInfo->FourCC==p->m_initInfo->FourCC)
            {
                LOG_DEBUG("Couldn't load plugin %s. Plugin was already loaded.\n", U2A(file));
                goto fail;
            }
        }
        if (!p->m_initInfo->Plugin->Create())
        {
            LOG_DEBUG("Couldn't load plugin %s. Create() failed.\n", U2A(file));
            goto fail;
        }

        m_plugins.add(p);
        LOG_DEBUG("loaded plugin %s (%s / %s)\n", U2A(file),
                  U2A(p->m_initInfo->Name),
                  U2A(CSL_GET_VERSION(p->m_initInfo->Version)));
        continue;

    fail:
        delete p;
    }

    return m_plugins.length();
}

void CslPluginMgr::UnloadPlugins()
{
    loopvrev(m_plugins)
        delete m_plugins.remove(i);
}

void CslPluginMgr::BuildMenu(wxMenu *menu)
{
    loopv(m_plugins)
    {
        CslPlugin *plugin=m_plugins[i]->m_initInfo->Plugin;
        plugin->CreateMenu(menu);
    }
}
