/***************************************************************************
 *   Copyright (C) 2007 -2009 by Glen Masgai                               *
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
#include "engine/CslTools.h"

#define CSL_OUTPUT_EXTENSION  _("Text files (*.txt)|*.txt")

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
    sizer_conv_filter_staticbox = new wxStaticBox(this, -1, _("Chat"));
    sizer_search_staticbox = new wxStaticBox(this, -1, _("Search"));
    text_ctrl_output = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY|wxHSCROLL|wxTE_RICH|wxTE_RICH2|wxTE_AUTO_URL);
    text_ctrl_search = new wxTextCtrl(this, TEXT_SEARCH, wxEmptyString);
    label_matches = new wxStaticText(this, wxID_ANY, _("%d matches"));
    button_search_prev = new wxButton(this, wxID_BACKWARD, _("&Back"));
    button_search_next = new wxButton(this, wxID_FORWARD, _("&Forward"));
    checkbox_conv_filter = new wxCheckBox(this, CHECK_CONV_FILTER, _("&Filter chat"));
    const wxString choice_conv_filter_choices[] =
    {
        _("0 (Low)"),
        _("1 (Default)")
    };
    choice_conv_filter = new wxChoice(this, CHOICE_CONV_FILTER, wxDefaultPosition, wxDefaultSize, 2, choice_conv_filter_choices, 0);
    button_load = new wxButton(this, wxID_OPEN, _("&Open"));
    button_save = new wxButton(this, wxID_SAVE, _("&Save"));
    button_close = new wxButton(this, wxID_CLOSE, _("&Close"));

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
    text_ctrl_output->SetMinSize(wxSize(-1,300));
    text_ctrl_search->SetMinSize(wxSize(120,-1));
    button_search_prev->Enable(false);
    button_search_next->Enable(false);
    choice_conv_filter->SetSelection(0);
    button_close->SetDefault();
    // end wxGlade

    label_matches->SetLabel(wxString::Format(label_matches->GetLabel(),0));
    choice_conv_filter->SetSelection(1);
    choice_conv_filter->Enable(false);
    m_filterLevel=1;
}

void CslDlgOutput::do_layout()
{
    // begin wxGlade: CslDlgOutput::do_layout
    wxFlexGridSizer* grid_sizer_main = new wxFlexGridSizer(3, 1, 0, 0);
    wxFlexGridSizer* grid_sizer_button = new wxFlexGridSizer(1, 4, 0, 0);
    wxFlexGridSizer* grid_sizer_control = new wxFlexGridSizer(1, 2, 0, 0);
    wxStaticBoxSizer* sizer_conv_filter = new wxStaticBoxSizer(sizer_conv_filter_staticbox, wxHORIZONTAL);
    wxFlexGridSizer* grid_sizer_conv_filter = new wxFlexGridSizer(1, 3, 0, 0);
    wxStaticBoxSizer* sizer_search = new wxStaticBoxSizer(sizer_search_staticbox, wxHORIZONTAL);
    wxFlexGridSizer* grid_sizer_search = new wxFlexGridSizer(1, 2, 0, 0);
    wxBoxSizer* sizer_search_button = new wxBoxSizer(wxHORIZONTAL);
    wxFlexGridSizer* grid_sizer_search_input = new wxFlexGridSizer(3, 1, 0, 0);
    wxFlexGridSizer* grid_sizer_search_text = new wxFlexGridSizer(1, 3, 0, 0);
    grid_sizer_main->Add(text_ctrl_output, 0, wxALL|wxEXPAND, 4);
    grid_sizer_search_input->Add(1, 1, 0, 0, 0);
    grid_sizer_search_text->Add(text_ctrl_search, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_search_text->Add(label_matches, 0, wxLEFT|wxRIGHT|wxALIGN_CENTER_VERTICAL, 6);
    grid_sizer_search_text->Add(20, 1, 0, 0, 0);
    grid_sizer_search_text->AddGrowableCol(0);
    grid_sizer_search_input->Add(grid_sizer_search_text, 1, wxEXPAND, 0);
    grid_sizer_search_input->Add(1, 1, 0, 0, 0);
    grid_sizer_search_input->AddGrowableRow(0);
    grid_sizer_search_input->AddGrowableRow(2);
    grid_sizer_search_input->AddGrowableCol(0);
    grid_sizer_search->Add(grid_sizer_search_input, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL, 0);
    sizer_search_button->Add(button_search_prev, 0, wxALL, 4);
    sizer_search_button->Add(button_search_next, 0, wxALL, 4);
    grid_sizer_search->Add(sizer_search_button, 1, wxEXPAND, 0);
    grid_sizer_search->AddGrowableCol(0);
    sizer_search->Add(grid_sizer_search, 1, wxEXPAND, 0);
    grid_sizer_control->Add(sizer_search, 1, wxALL|wxEXPAND, 4);
    grid_sizer_conv_filter->Add(checkbox_conv_filter, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    wxStaticText* label_filter_level = new wxStaticText(this, wxID_ANY, _("Level:"));
    grid_sizer_conv_filter->Add(label_filter_level, 0, wxLEFT|wxALIGN_CENTER_VERTICAL, 8);
    grid_sizer_conv_filter->Add(choice_conv_filter, 0, wxLEFT|wxRIGHT|wxALIGN_CENTER_VERTICAL, 4);
    sizer_conv_filter->Add(grid_sizer_conv_filter, 1, wxALIGN_CENTER_VERTICAL, 0);
    grid_sizer_control->Add(sizer_conv_filter, 1, wxALL|wxEXPAND, 4);
    grid_sizer_control->AddGrowableCol(0);
    grid_sizer_main->Add(grid_sizer_control, 1, wxEXPAND, 0);
    grid_sizer_button->Add(button_load, 0, wxALL, 4);
    grid_sizer_button->Add(button_save, 0, wxALL, 4);
    grid_sizer_button->Add(8, 1, 0, 0, 0);
    grid_sizer_button->Add(button_close, 0, wxALL, 4);
    grid_sizer_main->Add(grid_sizer_button, 1, wxBOTTOM|wxALIGN_RIGHT, 4);
    SetSizer(grid_sizer_main);
    grid_sizer_main->Fit(this);
    grid_sizer_main->AddGrowableRow(0);
    grid_sizer_main->AddGrowableCol(0);
    Layout();
    // end wxGlade

// TODO workaround possible?
#ifdef __WXMAC__
    button_search_prev->Hide();
    button_search_next->Hide();
    sizer_search_button->Detach(button_search_prev);
    sizer_search_button->Detach(button_search_next);
#endif

    grid_sizer_main->SetSizeHints(this);
    //CentreOnScreen();
}

void CslDlgOutput::OnClose(wxCloseEvent& event)
{
    Hide();
    wxPostEvent(GetParent(),event);
}

void CslDlgOutput::OnCommandEvent(wxCommandEvent& event)
{
    event.Skip();

    wxString s;

    switch (event.GetId())
    {
        case TEXT_SEARCH:
            SetSearchbarColour(Search(event.GetString()));
            break;

        case wxID_BACKWARD:
            text_ctrl_output->ShowPosition(m_searchResults.Item(--m_searchPos));
            SetSearchTextColour(m_searchResults.Item(m_searchPos),
                                m_searchResults.Item(m_searchPos+1));
            if (m_searchPos==0)
                button_search_prev->Enable(false);
            button_search_next->Enable();
            break;

        case wxID_FORWARD:
            text_ctrl_output->ShowPosition(m_searchResults.Item(++m_searchPos));
            SetSearchTextColour(m_searchResults.Item(m_searchPos),
                                m_searchResults.Item(m_searchPos-1));
            if (m_searchPos>=m_searchResults.GetCount()-1)
                button_search_next->Enable(false);
            button_search_prev->Enable();
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
            if (s.IsEmpty())
                text_ctrl_output->ShowPosition(0);
            else
                SetSearchbarColour(Search(s));
            break;
        }

        case wxID_OPEN:
        {
            if (wxDirExists(g_cslSettings->gameOutputPath))
                s=g_cslSettings->gameOutputPath;
            wxFileDialog dlg(this,_("Open log file"),s,wxEmptyString,
                             CSL_OUTPUT_EXTENSION,wxOPEN|wxFILE_MUST_EXIST);
            if (dlg.ShowModal()!=wxID_OK)
                break;

            s=dlg.GetPath();

            wxFile file(s,wxFile::read);
            if (!file.IsOpened())
                break;

            wxUint32 size=file.Length();
            char *buf=(char*)malloc(size+1);
            buf[size]=0;

            if (file.Read((void*)buf,size)!=(wxInt32)size)
            {
                free(buf);
                file.Close();
                break;
            }
            file.Close();

            Reset(wxString::Format(wxT("%s "),_("File"))+s);
            HandleOutput(buf,size);
            free(buf);
            break;
        }

        case wxID_SAVE:
        {
            SaveFile(wxEmptyString);
            break;
        }

        case wxID_CLOSE:
            Close();
            break;

        default:
            break;
    }
}

void CslDlgOutput::SetSearchTextColour(wxUint32 pos,wxUint32 posOld)
{
    wxTextAttr attr;
    wxTextAttr attrOld;

#ifdef __WXMAC__
    // TODO: check if the locker is necessary under wxMAC
    // causes flickering on wxGTK
    wxWindowUpdateLocker locker(text_ctrl_output);
    attr.SetFlags(wxTEXT_ATTR_TEXT_COLOUR);
    attr.SetTextColour(wxColour(255,64,64));
    attrOld.SetFlags(wxTEXT_ATTR_TEXT_COLOUR);
    attrOld.SetTextColour(*wxGREEN);
#else
    attr.SetFlags(wxTEXT_ATTR_BACKGROUND_COLOUR);
    attr.SetBackgroundColour(wxColour(255,64,64));
    attrOld.SetFlags(wxTEXT_ATTR_BACKGROUND_COLOUR);
    attrOld.SetBackgroundColour(g_cslSettings->colServerHigh);
#endif

    wxUint32 len=text_ctrl_search->GetValue().Len();
    text_ctrl_output->SetStyle(posOld,posOld+len,attrOld);
    text_ctrl_output->SetStyle(pos,pos+len,attr);
}

void CslDlgOutput::SetSearchbarColour(wxInt32 count)
{
    if (count>0 || text_ctrl_search->IsEmpty())
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

    const wxString& haystack=text_ctrl_output->GetValue();
    wxInt32 hlen=haystack.Len()-1;
    wxInt32 nlen=needle.Len();
    wxInt32 count=0;
    wxInt32 pos=0;

    button_search_prev->Enable(false);
    button_search_next->Enable(false);
    m_searchResults.Empty();
    m_searchPos=0;

    // TODO: check if the locker is necessary
    // causes flickering on wxGTK
    wxWindowUpdateLocker locker(text_ctrl_output);
#ifdef __WXMAC__
    attr.SetFlags(wxTEXT_ATTR_TEXT_COLOUR);
    attr.SetTextColour(SYSCOLOUR(wxSYS_COLOUR_WINDOWTEXT));
#else
    attr.SetFlags(wxTEXT_ATTR_BACKGROUND_COLOUR);
    attr.SetBackgroundColour(SYSCOLOUR(wxSYS_COLOUR_WINDOW));
#endif
    text_ctrl_output->SetStyle(0,hlen,attr);

    if (nlen && hlen)
    {
#ifdef __WXMAC__
        attr.SetTextColour(*wxGREEN);
#else
        attr.SetBackgroundColour(g_cslSettings->colServerHigh);
#endif
        while (pos<=hlen)
        {
            pos=haystack.find(needle,pos);
            if (pos>=0)
            {
                m_searchResults.Add(pos);
                text_ctrl_output->SetStyle(pos,pos+nlen,attr);
                pos+=nlen;
                count++;
            }
            else
                break;
        }
    }
    else
        count=-1;

    label_matches->SetLabel(wxString::Format(_("%d matches"),count<0 ? 0:count));

    switch (m_searchResults.GetCount())
    {
        case 0:
            text_ctrl_output->ShowPosition(0);
            return count;
        case 1:
            break;
        default:
            button_search_next->Enable();
            break;
    }

    text_ctrl_output->ShowPosition(m_searchResults.Item(0));

    return count;
}

wxString CslDlgOutput::Filter(wxUint32 start,wxUint32 end)
{
    bool colon=false;
    bool le=false;
    bool lb=true;
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
                    else if (!cmp.CmpNoCase(wxT("Renderer")))
                        skip=true;
                    else if (!cmp.CmpNoCase(wxT("Driver")))
                        skip=true;
                    else if (!cmp.CmpNoCase(wxT("Sound")))
                        skip=true;
                    else if (cmp==wxT("WARNING"))
                        skip=true;
                    else if (cmp==wxT("connected"))
                        skip=true;
                    else if (cmp==wxT("intermission"))
                        skip=true;
                }
                /*if (!skip && m_filterLevel>=2)
                {
                    if (cmp==wxT("NAME"))
                        skip=true;
                    if (cmp==wxT("PLAYER"))
                        skip=true;
                }*/
                if (!skip && i+1<end && m_text.GetChar(i+1)!=' ')
                    skip=true;

                break;

            case ' ':
                if (!colon&&!lb)
                {
                    skip=true;
                    /*if (m_filterLevel==2)
                    {
                        if (i<10)
                            break;
                        if (m_text.Mid(i-10,10)==wxT("(TEAMCHAT)"))
                            skip=false;
                    }*/

                }
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
                    lb=true;
                    colon=false;
                    skip=false;
                    continue;
                }
        }
        lb=false;
    }

    return s;
}

void CslDlgOutput::HandleOutput(char *text,wxUint32 size)
{
    wxInt32 start=0,end=0;

    if (!text || !size)
        return;

    start=m_text.Len();
    end=start+size;
    m_text.Alloc(start+size+sizeof(wxChar));
    m_text+=A2U(text);
    text_ctrl_output->AppendText(checkbox_conv_filter->GetValue() ?
                                 Filter(start,end) : m_text.Mid(start,size));

    SetSearchbarColour(Search(text_ctrl_search->GetValue()));
}

void CslDlgOutput::AddOutput(char *text,wxUint32 size)
{
    m_self->HandleOutput(text,size);
}

void CslDlgOutput::Reset(const wxString& title)
{
    wxString s;

    m_self->m_title=title;

    m_self->button_search_prev->Enable(false);
    m_self->button_search_next->Enable(false);
    m_self->text_ctrl_output->Clear();
    m_self->m_text.Clear();

    s=_("CSL - Game output");
    if (!title.IsEmpty())
        s+=wxT(": ")+title;
    m_self->SetTitle(s);

}

void CslDlgOutput::FixFilename(wxString *name)
{
    wxUint32 i,j;
    wxString exclude=wxT("\\/:*?\"<>| ");

    for (i=0;i<name->Length();i++)
        for (j=0;j<exclude.Length();j++)
            if (name->GetChar(i)==exclude.GetChar(j))
                name->SetChar(i,wxT('_'));
}

void CslDlgOutput::SaveFile(const wxString& path)
{
    wxString filename;
    wxDateTime now=wxDateTime::Now();

    if (!path.IsEmpty())
    {
        filename=m_self->m_title+wxT("-");
        m_self->FixFilename(&filename);
    }
    filename+=now.Format(wxT("%Y%m%d_%H%M%S"));

    if (path.IsEmpty() && m_self->checkbox_conv_filter->GetValue())
        filename+=wxT("-conversation");
    else
        filename+=wxT("-full");
    filename+=wxT(".txt");

    if (path.IsEmpty())
    {
        wxFileDialog dlg(m_self,_("Save log file"),wxEmptyString,filename,
                         CSL_OUTPUT_EXTENSION,wxSAVE|wxOVERWRITE_PROMPT);
        // wxGTK: hmm, doesn't work in the ctor?!
        if (wxDirExists(g_cslSettings->gameOutputPath))
            dlg.SetPath(g_cslSettings->gameOutputPath+wxT("/")+filename);
        if (dlg.ShowModal()!=wxID_OK)
            return;

        filename=dlg.GetPath();
        g_cslSettings->gameOutputPath=::wxPathOnly(filename);
    }
    else
    {
        wxString pathname=path;
        if (!path.EndsWith(PATHDIV))
            pathname+=PATHDIV;
        filename=pathname+filename;
    }

    const wxString &out=path.IsEmpty() ? m_self->text_ctrl_output->GetValue():m_self->m_text;
    WriteTextFile(filename,out,wxFile::write);
}
