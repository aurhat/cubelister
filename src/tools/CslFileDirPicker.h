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

#ifndef CSLFILEDIRPICKERBASE_H
#define CSLFILEDIRPICKERBASE_H

/**
    @author Glen Masgai <mimosius@users.sourceforge.net>
*/

class CslFileDirPickerEvent;

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE(wxEVT_COMMAND_CSL_EVT_FILEDIRPICK, wxID_ANY)
END_DECLARE_EVENT_TYPES()


typedef void (wxEvtHandler::*CslFileDirPickerEventFunction)(CslFileDirPickerEvent&);

#define CSL_EVT_FILEDIRPICK(id, fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
                               wxEVT_COMMAND_CSL_EVT_FILEDIRPICK, id, wxID_ANY, \
                               (wxObjectEventFunction)(wxEventFunction) \
                               wxStaticCastEvent(CslFileDirPickerEventFunction, &fn), \
                               (wxObject*)NULL \
                             ),

class CslFileDirPickerEvent : public wxCommandEvent
{
    public:
        CslFileDirPickerEvent(wxInt32 id=wxID_ANY,
                              const wxString& path=wxEmptyString,
                              const wxString& file=wxEmptyString) :
                wxCommandEvent(wxEVT_COMMAND_CSL_EVT_FILEDIRPICK, id),
                Path(path), File(file) { }

        virtual wxEvent* Clone() const
        {
            return new CslFileDirPickerEvent(*this);
        }

        wxString Path, File;
};


class CslFileDirPickerBase : public wxPanel
{
    public:
        CslFileDirPickerBase(wxWindow *parent, wxInt32 id=wxID_ANY,
                             const wxString& title=wxEmptyString,
                             const wxString& path=wxEmptyString,
                             wxInt32 style=0);

        const wxString& GetPath() const { return m_path; }
        void SetPath(const wxString& path) { m_path=path; }

        const wxString& GetTitle() const { return m_title; }
        void SetTitle(const wxString& title) { m_title=title; }

    protected:
        wxTextCtrl *m_text;
        wxButton *m_button;
        wxString m_title, m_path;
        wxInt32 m_id, m_style;

    private:
        virtual void OnCommandEvent(wxCommandEvent& event) = 0;

        DECLARE_EVENT_TABLE()
};

#define CSL_FP_OPEN_EXIST  (wxFD_OPEN|wxFD_FILE_MUST_EXIST)

class CslFilePickerCtrl : public CslFileDirPickerBase
{
    public:
        CslFilePickerCtrl(wxWindow *parent, wxInt32 id=wxID_ANY,
                          const wxString& title=wxEmptyString,
                          const wxString& path=wxEmptyString,
                          const wxString& file=wxEmptyString,
                          const wxString& wildcard=wxT("*.*"),
                          wxInt32 style=0) :
                CslFileDirPickerBase(parent, id, title, path, style),
                m_wildcard(wildcard) { }

        const wxString& GetWildcard() const { return m_wildcard; }
        void SetWildcard(const wxString& wildcard) { m_wildcard=wildcard; }

    private:
        wxString m_file, m_wildcard;

        virtual void OnCommandEvent(wxCommandEvent& event);
};

#define CSL_DP_OPEN_EXIST (wxDD_DEFAULT_STYLE|wxDD_DIR_MUST_EXIST)

class CslDirPickerCtrl : public CslFileDirPickerBase
{
    public:
        CslDirPickerCtrl(wxWindow *parent, wxInt32 id=wxID_ANY,
                         const wxString& title=wxEmptyString,
                         const wxString& path=wxEmptyString,
                         wxInt32 style=0) :
                CslFileDirPickerBase(parent, id, title, path, style) { }

    private:
        virtual void OnCommandEvent(wxCommandEvent& event);
};

#endif //CSLFILEDIRPICKERBASE_H
