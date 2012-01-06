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
#include "CslFileDirPicker.h"

DEFINE_EVENT_TYPE(wxEVT_COMMAND_CSL_EVT_FILEDIRPICK)

BEGIN_EVENT_TABLE(CslFileDirPickerBase, wxPanel)
    EVT_BUTTON(wxID_ANY, CslFileDirPickerBase::OnCommandEvent)
    EVT_TEXT(wxID_ANY, CslFileDirPickerBase::OnCommandEvent)
    EVT_TEXT_ENTER(wxID_ANY, CslFileDirPickerBase::OnCommandEvent)
END_EVENT_TABLE()


CslFileDirPickerBase::CslFileDirPickerBase(wxWindow *parent, wxInt32 id,
                                           const wxString& title,
                                           const wxString& path, wxInt32 style):
        wxPanel(parent),
        m_title(title), m_path(path), m_id(id), m_style(style)
{
    wxFlexGridSizer *sizer=new wxFlexGridSizer(1, 2, 0, 0);
    wxFlexGridSizer *sizer_text=new wxFlexGridSizer(3, 1, 0, 0);
    sizer_text->Add(1, 1);
    m_text=new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition,
                          wxDefaultSize, wxTE_PROCESS_ENTER);
    m_text->ChangeValue(path);
    sizer_text->Add(m_text, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL);
    sizer_text->Add(1, 1);
    sizer_text->AddGrowableCol(0);
    sizer_text->AddGrowableRow(0);
    sizer_text->AddGrowableRow(2);
    sizer->Add(sizer_text, 1, wxEXPAND|wxTOP|wxBOTTOM, 4);
    m_button=new wxButton(this, wxID_ANY, _("Browse"));
    sizer->Add(m_button, 0, wxALIGN_CENTER_VERTICAL|wxLEFT, 4);
    sizer->AddGrowableCol(0);

    sizer->Layout();
    SetSizer(sizer);
}

void CslFilePickerCtrl::OnCommandEvent(wxCommandEvent& event)
{
    if (event.GetEventObject()==(void*)m_text)
    {
        wxString path=event.GetString();

        if (event.GetEventType()==wxEVT_COMMAND_TEXT_UPDATED)
        {
            if (!path.IsEmpty())
            {
                if (!wxFileExists(path))
                {
                    SetTextCtrlErrorState(m_text, true);
                    return;
                }
                else
                    m_path=path;
            }
            SetTextCtrlErrorState(m_text, false);
        }
        else
        {
            if (wxFileExists(path))
            {
                CslFileDirPickerEvent evt(m_id, path);
                wxPostEvent(m_parent, evt);
            }
        }
    }
    else if (event.GetEventObject()==(void*)m_button)
    {
        wxFileDialog dlg(this, m_title, m_path, m_file, m_wildcard, m_style);

        if (dlg.ShowModal()==wxID_OK)
        {
            m_text->ChangeValue(dlg.GetPath());
            CslFileDirPickerEvent evt(m_id, dlg.GetPath(), dlg.GetFilename());
            wxPostEvent(m_parent, evt);
        }
    }
}

void CslDirPickerCtrl::OnCommandEvent(wxCommandEvent& event)
{
    if (event.GetEventObject()==(void*)m_text)
    {
        wxString path=event.GetString();

        if (event.GetEventType()==wxEVT_COMMAND_TEXT_UPDATED)
        {
            if (!path.IsEmpty())
            {
                if (!wxDirExists(path))
                {
                    SetTextCtrlErrorState(m_text, true);
                    return;
                }
                else
                    m_path=path;
            }
            SetTextCtrlErrorState(m_text, false);
        }
        else
        {
            if (wxDirExists(path))
            {
                CslFileDirPickerEvent evt(m_id, path);
                wxPostEvent(m_parent, evt);
            }
        }
    }
    else if (event.GetEventObject()==(void*)m_button)
    {
        wxDirDialog dlg(this, m_title, m_path, m_style);

        if (dlg.ShowModal()==wxID_OK)
        {
            m_text->ChangeValue(dlg.GetPath());
            CslFileDirPickerEvent evt(m_id, dlg.GetPath());
            wxPostEvent(m_parent, evt);
        }
    }
}
