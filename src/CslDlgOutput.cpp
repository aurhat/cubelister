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

#include <wx/file.h>
#include <wx/wupdlock.h>
#include "CslDlgOutput.h"
#include "CslSettings.h"
#include "CslTools.h"

BEGIN_EVENT_TABLE(CslDlgOutput,wxDialog)
    EVT_CLOSE(CslDlgOutput::OnClose)
    EVT_BUTTON(wxID_ANY,CslDlgOutput::OnCommandEvent)
    EVT_CHECKBOX(wxID_ANY,CslDlgOutput::OnCommandEvent)
    EVT_CHOICE(wxID_ANY,CslDlgOutput::OnCommandEvent)
    EVT_TEXT(wxID_ANY,CslDlgOutput::OnCommandEvent)
END_EVENT_TABLE()

enum
{
    TEXT_SEARCH        = wxID_HIGHEST + 1,
    CHECK_CONV_FILTER,
    CHOICE_CONV_FILTER
};

CslDlgOutput* CslDlgOutput::m_self=NULL;

CslDlgOutput::CslDlgOutput(wxWindow* parent,int id,const wxString& title,
                           const wxPoint& pos,const wxSize& size, long style):
        wxDialog(parent, id, title, pos, size, style)
{
    // begin wxGlade: CslDlgOutput::CslDlgOutput
    text_ctrl_output = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY|wxHSCROLL|wxTE_RICH|wxTE_RICH2|wxTE_AUTO_URL);
    text_ctrl_search = new wxTextCtrl(this, TEXT_SEARCH, wxEmptyString);
    label_matches = new wxStaticText(this, wxID_ANY, _("%d matches"));
    checkbox_conv_filter = new wxCheckBox(this, CHECK_CONV_FILTER, _("Filter conversation (Beta)"));
    const wxString choice_conv_filter_choices[] =
    {
        _("0 (Low)"),
        _("1 (Default)"),
        _("2 (TC-Server)")
    };
    choice_conv_filter = new wxChoice(this, CHOICE_CONV_FILTER, wxDefaultPosition, wxDefaultSize, 3, choice_conv_filter_choices, 0);
    static_line = new wxStaticLine(this, wxID_ANY);
    button_load = new wxButton(this, wxID_OPEN, _("&Load"));
    button_save = new wxButton(this, wxID_SAVE, _("&Save"));
    button_close_copy = new wxButton(this, wxID_CLOSE, _("&Close"));

    set_properties();
    do_layout();
    // end wxGlade

    m_self=this;

    Reset(wxEmptyString);
}

void CslDlgOutput::set_properties()
{
    // begin wxGlade: CslDlgOutput::set_properties
    SetTitle(_("CSL - Game output"));
    text_ctrl_output->SetMinSize(wxSize(550,300));
    choice_conv_filter->SetSelection(0);
    button_close_copy->SetDefault();
    // end wxGlade

    label_matches->SetLabel(wxString::Format(label_matches->GetLabel(),0));
    choice_conv_filter->SetSelection(1);
    choice_conv_filter->Enable(false);
    m_filterLevel=1;
}

void CslDlgOutput::do_layout()
{
    // begin wxGlade: CslDlgOutput::do_layout
    wxFlexGridSizer* grid_sizer_main = new wxFlexGridSizer(5, 1, 0, 0);
    wxFlexGridSizer* grid_sizer_button = new wxFlexGridSizer(1, 4, 0, 0);
    wxFlexGridSizer* grid_sizer_conv_filter = new wxFlexGridSizer(1, 4, 0, 0);
    wxFlexGridSizer* grid_sizer_search = new wxFlexGridSizer(1, 4, 0, 0);
    grid_sizer_main->Add(text_ctrl_output, 0, wxALL|wxEXPAND, 4);
    wxStaticText* label_search = new wxStaticText(this, wxID_ANY, _("Search:"));
    grid_sizer_search->Add(label_search, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_search->Add(text_ctrl_search, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_search->Add(label_matches, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_search->Add(30, 1, 0, wxADJUST_MINSIZE, 0);
    grid_sizer_search->AddGrowableCol(1);
    grid_sizer_main->Add(grid_sizer_search, 1, wxEXPAND, 0);
    grid_sizer_conv_filter->Add(checkbox_conv_filter, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_conv_filter->Add(1, 1, 0, wxADJUST_MINSIZE, 0);
    wxStaticText* label_filter_level = new wxStaticText(this, wxID_ANY, _("Level:"));
    grid_sizer_conv_filter->Add(label_filter_level, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_conv_filter->Add(choice_conv_filter, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_conv_filter->AddGrowableCol(1);
    grid_sizer_main->Add(grid_sizer_conv_filter, 1, wxEXPAND, 0);
    grid_sizer_main->Add(static_line, 0, wxEXPAND, 0);
    grid_sizer_button->Add(button_load, 0, wxALL, 4);
    grid_sizer_button->Add(button_save, 0, wxALL, 4);
    grid_sizer_button->Add(8, 1, 0, wxADJUST_MINSIZE, 0);
    grid_sizer_button->Add(button_close_copy, 0, wxALL, 4);
    grid_sizer_main->Add(grid_sizer_button, 1, wxBOTTOM|wxALIGN_RIGHT, 4);
    SetSizer(grid_sizer_main);
    grid_sizer_main->Fit(this);
    grid_sizer_main->AddGrowableRow(0);
    grid_sizer_main->AddGrowableCol(0);
    Layout();
    // end wxGlade

    CentreOnScreen();
    grid_sizer_main->SetSizeHints(this);
}

void CslDlgOutput::OnClose(wxCloseEvent& event)
{
    if (event.CanVeto())
    {
        Hide();
        wxPostEvent(GetParent(),event);
        return;
    }
    event.Skip();
}

void CslDlgOutput::OnCommandEvent(wxCommandEvent& event)
{
    event.Skip();

    wxString s;
    wxString ext=_("Text files (*.txt)|*.txt");

    switch (event.GetId())
    {
        case TEXT_SEARCH:
            SetSearchbarColour(Search(event.GetString()));
            break;

        case CHOICE_CONV_FILTER:
            m_filterLevel=event.GetSelection();
        case CHECK_CONV_FILTER:
        {
            bool checked=checkbox_conv_filter->GetValue();
            text_ctrl_output->Clear();
            text_ctrl_output->AppendText(checked ? Filter(0,m_text.Len()) : m_text);
            choice_conv_filter->Enable(checked);

            s=text_ctrl_search->GetValue();
            if (!s.IsEmpty())
                SetSearchbarColour(Search(s));

            text_ctrl_output->ShowPosition(0);
            break;
        }

        case wxID_OPEN:
        {
            if (wxDirExists(g_cslSettings->m_outputPath))
                s=g_cslSettings->m_outputPath;
            wxFileDialog dlg(this,_("Open log file"),s,wxEmptyString,
                             ext,wxOPEN|wxFILE_MUST_EXIST);
            if (dlg.ShowModal()!=wxID_OK)
                break;

            s=dlg.GetPath();
            g_cslSettings->m_outputPath=::wxPathOnly(s);

            wxFile file(s,wxFile::read);
            if (!file.IsOpened())
                break;

            wxUint32 size=file.Length();
            char *buf=(char*)malloc(size+1);
            buf[size]=0;

            if (file.Read((void*)buf,size)!=(wxInt32)size)
            {
                free(buf);
                break;
            }
            file.Close();

            Reset(wxString::Format(wxT("%s "),wxT("File"))+s);
            HandleOutput(buf,size);
            free(buf);
            break;
        }

        case wxID_SAVE:
        {
            wxDateTime now=wxDateTime::Now();
            s=now.Format(wxT("%Y%m%d-%H%M%S"));
            if (checkbox_conv_filter->GetValue())
                s+=wxT("_conversation");
            else
                s+=wxT("_full");
            s+=wxT(".txt");

            wxFileDialog dlg(this,_("Save log file"),wxEmptyString,s,
                             ext,wxSAVE|wxOVERWRITE_PROMPT);
            // FIXME wxGTK: hmm, doesn't work in the dtor?!
            if (wxDirExists(g_cslSettings->m_outputPath))
                dlg.SetPath(g_cslSettings->m_outputPath+wxT("/")+s);
            if (dlg.ShowModal()!=wxID_OK)
                return;

            s=dlg.GetPath();
            g_cslSettings->m_outputPath=::wxPathOnly(s);

            wxFile file(s,wxFile::write);
            if (!file.IsOpened())
                return;

            wxString text=text_ctrl_output->GetValue();
            file.Write((void*)U2A(text),text.Len());
            file.Close();

            break;
        }

        case wxID_CLOSE:
            Close();
            break;

        default:
            break;
    }
}

void CslDlgOutput::SetSearchbarColour(wxInt32 count)
{
    if (count)
    {
#ifdef __WXMAC__
        // very ugly - setting back to black doesnt work, so add 1
        text_ctrl_search->SetForegroundColour(SYSCOLOUR(wxSYS_COLOUR_WINDOWTEXT).Red()+1);
#else
        text_ctrl_search->SetBackgroundColour(SYSCOLOUR(wxSYS_COLOUR_WINDOW));
        text_ctrl_search->SetForegroundColour(SYSCOLOUR(wxSYS_COLOUR_WINDOWTEXT));
#endif
    }
    else
    {
#ifdef __WXMAC__
        text_ctrl_search->SetForegroundColour(*wxRED);
#else
        text_ctrl_search->SetBackgroundColour(wxColour(255,100,100));
        text_ctrl_search->SetForegroundColour(*wxWHITE);
#endif
    }

#ifdef __WXMSW__
    text_ctrl_search->Refresh();
#endif
}

wxInt32 CslDlgOutput::Search(const wxString& needle)
{
    wxTextAttr attr;

    wxInt32 count=0;
    wxInt32 pos=0;
    wxString haystack=text_ctrl_output->GetValue();
    wxInt32 hlen=haystack.Len();
    wxInt32 nlen=needle.Len();

    wxWindowUpdateLocker locler(text_ctrl_output);

#ifdef __WXMAC__
    attr.SetFlags(wxTEXT_ATTR_TEXT_COLOUR);
    attr.SetTextColour(SYSCOLOUR(wxSYS_COLOUR_WINDOWTEXT));
#else
    attr.SetFlags(wxTEXT_ATTR_BACKGROUND_COLOUR);
    attr.SetBackgroundColour(SYSCOLOUR(wxSYS_COLOUR_WINDOW));
#endif
    text_ctrl_output->SetStyle(0,hlen,attr);

    if (!nlen)
    {
        count=-1;
        goto finish;
    }

#ifdef __WXMAC__
    attr.SetTextColour(*wxGREEN);
#else
    attr.SetBackgroundColour(g_cslSettings->m_colServerHigh);
#endif

    while (pos<=hlen)
    {
        pos=haystack.find(needle,pos);
        if (pos>=0)
        {
            text_ctrl_output->SetStyle(pos,pos+nlen,attr);
            pos+=nlen;
            count++;
        }
        else
            break;
    }

finish:
    wxInt32 c=count<0 ? 0:count;
    label_matches->SetLabel(wxString::Format(_("%d matches"),c));

    return count;
}

wxString CslDlgOutput::Filter(wxUint32 start,wxUint32 end)
{
    bool colon=false;
    bool le=false;
    bool skip=false;
    wxUint32 i=start;
    wxUint32 b=start;
    wxString s;
    wxString cmp;
    char c;

    s.Alloc(end-start);

    for (;i<end;i++)
    {
        c=m_text.GetChar(i);

        switch (c)
        {
            case ':':
                if (colon||skip)
                    continue;

                colon=true;

                if (!m_filterLevel)
                    break;

                if (m_filterLevel>=1)
                {
                    cmp=m_text.Mid(b,i-b);
                    if (cmp==wxT("init"))
                        skip=true;
                    else if (cmp==wxT("Renderer"))
                        skip=true;
                    else if (cmp==wxT("Driver"))
                        skip=true;
                    else if (cmp==wxT("WARNING"))
                        skip=true;
                    else if (cmp==wxT("connected"))
                        skip=true;
                    else if (cmp==wxT("intermission"))
                        skip=true;
                }
                if (!skip && m_filterLevel>=2)
                {
                    if (cmp.CmpNoCase(wxT("NAME"))==0) // FIXME remove NoCase
                        skip=true;
                    if (cmp==wxT("PLAYER"))
                        skip=true;
                }
                if (!skip && i+1<end && m_text.GetChar(i+1)!=' ')
                    skip=true;

                break;

            case ' ':
                if (!colon)
                    skip=true;
                break;

            case '\r':
            case '\n':
                le=true;

            default:
                if (le)
                {
                    if (!skip && colon)
                        s+=m_text.Mid(b,i-b+1);
                    b=i+1;

                    le=false;
                    colon=false;
                    skip=false;
                    continue;
                }
        }
    }

    return s;
}

void CslDlgOutput::HandleOutput(char *text,wxUint32 size)
{
    wxInt32 start=0,end=0;

    if (text && size)
    {
        start=m_text.Len();
        end=start+size;
        m_text.Alloc(start+size+sizeof(wxChar));
        m_text+=A2U(text);
        text_ctrl_output->AppendText(checkbox_conv_filter->GetValue() ?
                                     Filter(start,end) : m_text.Mid(start,size));
    }
    else
    {
        end=m_text.Len();
        text_ctrl_output->AppendText(checkbox_conv_filter->GetValue() ?
                                     Filter(start,end) : m_text);
    }

    text_ctrl_output->ShowPosition(0);

    wxString s=text_ctrl_search->GetValue();
    if (!s.IsEmpty())
        SetSearchbarColour(Search(s));
}

void CslDlgOutput::AddOutput(char *text,wxUint32 size)
{
    if (m_self)
        m_self->HandleOutput(text,size);
}

void CslDlgOutput::Reset(const wxString& title)
{
    if (!m_self)
        return;

    m_self->text_ctrl_output->Clear();
    m_self->m_text.Clear();
    m_self->SetTitle(wxString::Format(wxT("%s: "),_("CSL - Game output"))+title);
}
