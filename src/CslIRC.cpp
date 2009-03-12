/***************************************************************************
 *   Copyright (C) 2007-2009 by Glen Masgai                                *
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

#include <wx/tokenzr.h>
#include "CslIRC.h"
#include "CslIPC.h"
#include "CslMenu.h"

CslIrcEngine* CslIrcNotebook::m_engine=NULL;

BEGIN_EVENT_TABLE(CslIrcNotebook,wxAuiNotebook)
    EVT_AUINOTEBOOK_PAGE_CHANGED(wxID_ANY,CslIrcNotebook::OnNotebookPageSelected)
    EVT_AUINOTEBOOK_PAGE_CLOSE(wxID_ANY,CslIrcNotebook::OnNotebookPageClose)
END_EVENT_TABLE()

CslIrcNotebook::CslIrcNotebook(wxWindow* parent,wxWindowID id,long style) :
        wxAuiNotebook(parent,id,wxDefaultPosition,wxDefaultSize,style)
{
    m_engine=new CslIrcEngine;
    m_engine->InitEngine();
}

CslIrcNotebook::~CslIrcNotebook()
{
    if (m_engine)
    {
        m_engine->DeinitEngine();
        delete m_engine;
        m_engine=NULL;
    }

    WX_CLEAR_ARRAY(g_CslIrcNetworks);
}

void CslIrcNotebook::OnNotebookPageSelected(wxAuiNotebookEvent& event)
{
    CslIrcPanel *page;

    if ((page=(CslIrcPanel*)GetPage(event.GetSelection())))
        page->OnFocus();
}

void CslIrcNotebook::OnNotebookPageClose(wxAuiNotebookEvent& event)
{
    CslIrcPanel *page=(CslIrcPanel*)GetPage(event.GetSelection());

    if (page->GetType()==CslIrcPanel::TYPE_SESSION)
    {
        if (wxYES==wxMessageBox(_("Are you sure you want to end this chat session?"),
                                _("Close chat session"),wxYES_NO,this))
            page->EndIrcSession();

        event.Veto();
    }
    else
        page->EndIrcSession();
}


BEGIN_EVENT_TABLE(CslIrcPanel,wxPanel)
    EVT_SIZE(CslIrcPanel::OnSize)
    EVT_MENU(wxID_ANY,CslIrcPanel::OnCommandEvent)
    EVT_LIST_ITEM_ACTIVATED(wxID_ANY,CslIrcPanel::OnListCtrlActivated)
    EVT_CHOICE(wxID_ANY,CslIrcPanel::OnCommandEvent)
    EVT_TEXT_ENTER(wxID_ANY,CslIrcPanel::OnCommandEvent)
    EVT_CONTEXT_MENU(CslIrcPanel::OnContextMenu)
    EVT_SPLITTER_SASH_POS_CHANGED(wxID_ANY,CslIrcPanel::OnSplitter)
    EVT_TIMER(wxID_ANY,CslIrcPanel::OnTimer)
    CSL_EVT_IRC(wxID_ANY,CslIrcPanel::OnIrcEvent)
END_EVENT_TABLE()

enum
{
    CHOICE_CONNECTION = wxID_HIGHEST+1000,
    TEXT_CTRL_INPUT,
    MENU_ENCODING_START,
    MENU_ENCODING_END = MENU_ENCODING_START+CSL_NUM_CHAR_ENCODINGS,
    MENU_TAB
};

CslIrcPanel::CslIrcPanel(CslIrcNotebook* parent,CslIrcSession *session,
                         wxInt32 type,const wxString& channel) :
        wxPanel(parent,wxID_ANY),m_parent(parent),
        m_session(session),m_type(type),
        m_textLength(0),m_missedLines(0),
        m_commandBufferPos(0),m_rejoin(false)
{
    splitter=new wxSplitterWindow(this,wxID_ANY,wxDefaultPosition,wxDefaultSize,wxSP_3D|wxSP_BORDER);
    pane_chat=new wxPanel(splitter,wxID_ANY);
    pane_users=new wxPanel(splitter,wxID_ANY);
    list_ctrl_users=new wxListCtrl(pane_users,wxID_ANY,wxDefaultPosition,wxDefaultSize,
                                   wxLC_REPORT|wxSUNKEN_BORDER|wxLC_NO_HEADER);
    static_topic=new wxStaticText(pane_chat,wxID_ANY,wxEmptyString);
#ifdef CSL_USE_RICHTEXT
    text_ctrl_chat=new wxRichTextCtrl(pane_chat,wxID_ANY,wxEmptyString,wxDefaultPosition,wxDefaultSize,
                                      wxRE_MULTILINE|wxTE_READONLY);
#else
    text_ctrl_chat=new wxTextCtrl(pane_chat,wxID_ANY,wxEmptyString,wxDefaultPosition,wxDefaultSize,
                                  wxTE_MULTILINE|wxTE_READONLY|wxTE_AUTO_URL|wxTE_RICH|wxTE_RICH2);
#endif
    wxString choices[]={ _("Disconnect") };
    choice_connection=new wxChoice(this,CHOICE_CONNECTION,wxDefaultPosition,wxDefaultSize,1,choices);
    text_ctrl_input=new wxTextCtrl(this,TEXT_CTRL_INPUT,wxEmptyString,wxDefaultPosition,wxDefaultSize,
                                   wxTE_PROCESS_ENTER|wxTE_PROCESS_TAB);

    //Layout
    wxFlexGridSizer *grid_sizer_main=new wxFlexGridSizer(2,1,0,0);
    wxFlexGridSizer *grid_sizer_bottom=new wxFlexGridSizer(1,2,0,0);
    wxFlexGridSizer *grid_sizer_input=new wxFlexGridSizer(3,1,0,0);
    wxFlexGridSizer *grid_sizer_users = new wxFlexGridSizer(1,1,0,0);
    grid_sizer_chat=new wxFlexGridSizer(2,1,0,0);

    grid_sizer_users->Add(list_ctrl_users,1,wxEXPAND,4);
    pane_users->SetSizer(grid_sizer_users);
    grid_sizer_users->AddGrowableRow(0);
    grid_sizer_users->AddGrowableCol(0);
    grid_sizer_chat->Add(static_topic,0,wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL,2);
    grid_sizer_chat->Add(text_ctrl_chat,0,wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL,0);
    pane_chat->SetSizer(grid_sizer_chat);
    grid_sizer_chat->AddGrowableRow(1);
    grid_sizer_chat->AddGrowableCol(0);
    splitter->SplitVertically(pane_users,pane_chat);
    grid_sizer_main->Add(splitter,1,wxEXPAND,0);
    grid_sizer_bottom->Add(choice_connection,0,wxALL|wxALIGN_CENTER_VERTICAL,4);
    grid_sizer_input->Add(20,1,0,0,0);
    grid_sizer_input->Add(text_ctrl_input,0,wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_CENTER_VERTICAL,4);
    grid_sizer_input->Add(20,1,0,0,0);
    grid_sizer_input->AddGrowableRow(0);
    grid_sizer_input->AddGrowableRow(2);
    grid_sizer_input->AddGrowableCol(0);
    grid_sizer_bottom->Add(grid_sizer_input,1,wxEXPAND,0);
    grid_sizer_bottom->AddGrowableCol(1);
    grid_sizer_main->Add(grid_sizer_bottom,1,wxEXPAND,0);
    SetSizer(grid_sizer_main);
    grid_sizer_main->Fit(this);
    grid_sizer_main->AddGrowableRow(0);
    grid_sizer_main->AddGrowableCol(0);

    splitter->SetSashGravity(0.0);
    splitter->SetMinimumPaneSize(40);
    splitter->SetSashPosition(100);

    wxListItem item;
    item.SetMask(wxLIST_MASK_TEXT);
    item.SetText(wxT(""));
    list_ctrl_users->InsertColumn(0,item);
    list_ctrl_users->SetColumnWidth(0,list_ctrl_users->GetClientSize().x);

    for (wxUint32 i=0;i<g_CslIrcNetworks.GetCount();i++)
    {
        CslIrcNetwork *network=g_CslIrcNetworks.Item(i);
        choice_connection->Append(network->Name);
        choice_connection->SetClientData(choice_connection->GetCount()-1,network);
    }

    m_channel.Name=channel;

    if (type==TYPE_NONE || type==TYPE_SESSION || type==TYPE_PRIVATE)
    {
        pane_users->Hide();
        splitter->Unsplit(pane_users);
    }
    if (type>TYPE_SESSION)
        choice_connection->Hide();

#ifdef CSL_USE_RICHTEXT
    //disable undo, otherwise it's eating all the ram
    text_ctrl_chat->BeginSuppressUndo();
#else
#ifndef __WXGTK__
    //set some colour here otherwise further calls to SetDefaultStyle() don't work
    text_ctrl_chat->SetDefaultStyle(GetForegroundColour());
#endif
#endif

    text_ctrl_input->Connect(wxEVT_CHAR,wxKeyEventHandler(CslIrcPanel::OnKeypress),NULL,this);

    Layout();

    m_timer.SetOwner(this);
}

CslIrcPanel::~CslIrcPanel()
{
    if (!m_session || m_type!=TYPE_SESSION)
        return;

    CslIrcEngine *engine=CslIrcNotebook::GetEngine();

    if (engine)
        engine->DestroySession(m_session);
}

void CslIrcPanel::OnKeypress(wxKeyEvent& event)
{
    switch (event.GetKeyCode())
    {
        case WXK_UP:
            if (!m_commandBuffer.GetCount())
                return;
            if (m_commandBufferPos)
                m_commandBufferPos--;
            text_ctrl_input->ChangeValue(m_commandBuffer.Item(m_commandBufferPos));
            text_ctrl_input->SetInsertionPointEnd();
            return;

        case WXK_DOWN:
            if (m_commandBuffer.GetCount()==0)
                return;
            m_commandBufferPos++;
            if (m_commandBufferPos>=m_commandBuffer.GetCount())
            {
                m_commandBufferPos=m_commandBuffer.GetCount();
                text_ctrl_input->Clear();
                return;
            }
            text_ctrl_input->ChangeValue(m_commandBuffer.Item(m_commandBufferPos));
            text_ctrl_input->SetInsertionPointEnd();
            return;

        case WXK_TAB:
        {
            if (m_type==TYPE_CHANNEL)
            {
                wxUint32 i,l;
                wxMenu menu;
                wxString s,u;
                CslIrcUser *user;

                if (!(l=list_ctrl_users->GetItemCount()) ||
                    (s=text_ctrl_input->GetValue()).IsEmpty())
                    return;
                if (s.Find(wxT(' ')))
                    s=s.AfterLast(wxT(' '));

                s=s.Lower();

                for (i=0;i<l;i++)
                {
                    user=(CslIrcUser*)list_ctrl_users->GetItemData(i);
                    if (user->Nick.Lower().StartsWith(s))
                        CslMenu::AddItem(&menu,MENU_TAB+i,user->Nick);
                }

                if (menu.GetMenuItemCount())
                    PopupMenu(&menu);
            }
            else if (m_type==TYPE_PRIVATE && !m_channel.Name.IsEmpty())
                text_ctrl_input->AppendText(m_channel.Name+wxT(" "));
            return;
        }

        case WXK_ESCAPE:
            text_ctrl_input->Clear();
            break;
    }

    event.Skip();
}

void CslIrcPanel::OnListCtrlActivated(wxListEvent& event)
{
    CslIrcUser *user=(CslIrcUser*)list_ctrl_users->GetItemData(event.GetIndex());
    GetOrCreatePanel(user->Nick,m_session);
}

void CslIrcPanel::OnSize(wxSizeEvent& event)
{
    const wxSize& size=static_topic->GetClientSize();

    if (size.x)
    {
        if (!m_channel.Topic.IsEmpty())
        {
            static_topic->SetLabel(m_channel.Topic);
            static_topic->Wrap(size.x-4);
            grid_sizer_chat->Layout();
        }
    }

    event.Skip();
}

void CslIrcPanel::OnCommandEvent(wxCommandEvent& event)
{
    wxString s;
    wxInt32 id;

    switch ((id=event.GetId()))
    {
        case CHOICE_CONNECTION:
        {
            if ((id=choice_connection->GetSelection()))
            {
                CslIrcPanel *panel;
                CslIrcNetwork *network=(CslIrcNetwork*)event.GetClientData();
                CslIrcServer *server=network->Servers.Item(0);

                for (wxUint32 i=0;i<m_parent->GetPageCount();i++)
                {
                    panel=(CslIrcPanel*)m_parent->GetPage(i);
                    if (panel->m_session &&
                        panel->m_session->GetContext().Server->Network->Name==network->Name)
                    {
                        m_parent->SetSelection(i);
                        if (panel->m_session->GetState()==CslIrcSession::STATE_DISCONNECTED)
                            panel->m_session->Connect(CslIrcContext(server,panel));
                        return;
                    }
                }

                panel=GetOrCreatePanel(network->Name,NULL);
                panel->m_session=CslIrcNotebook::GetEngine()->CreateSession();

                if (panel->m_session && panel->m_session->Connect(CslIrcContext(server,panel)))
                {
                    s<<wxT("\0032*** ")<<_("Connecting to server...")<<wxT("\003");
                    panel->AddLine(s);
                }
            }
            else
                EndIrcSession();
            break;
        }

        case TEXT_CTRL_INPUT:
        {
            HandleInput(text_ctrl_input->GetValue());
            text_ctrl_input->Clear();
            break;
        }

        default:
        {
            if (id>=MENU_ENCODING_START && id<MENU_ENCODING_END)
            {
                wxString s=CslCharEncodings[id-MENU_ENCODING_START].Encoding;
                CslIrcChannel *channel=m_session->FindChannel(m_channel);
                if (channel)
                    channel->Encoding.SetEncoding(s);
                m_channel.Encoding.SetEncoding(s);
            }
            else if (id>=MENU_TAB)
            {
                wxInt32 pos;
                wxString s,u;

                id-=MENU_TAB;

                if (id>=list_ctrl_users->GetItemCount() ||
                    (s=text_ctrl_input->GetValue()).IsEmpty())
                    return;

                u=((CslIrcUser*)list_ctrl_users->GetItemData(id))->Nick;

                if ((pos=s.Find(wxT(' '),true))!=wxNOT_FOUND)
                {
                    s=s.Mid(pos+1);
                    u+=wxT(" ");
                }
                else
                    u+=wxT(": ");

                if (u.Lower().StartsWith(s.Lower()))
                    text_ctrl_input->Replace(pos+1,pos+1+s.Length(),u);
            }
        }
    }
}

void CslIrcPanel::OnContextMenu(wxContextMenuEvent& event)
{
    wxUint32 i;
    wxString s;
    wxMenu menu,*sub;
    wxMenuItem *item;
    wxPoint point=event.GetPosition();

    //from keyboard
    if (point==wxDefaultPosition)
        point=wxGetMousePosition();


    sub=new wxMenu;
    item=menu.AppendSubMenu(sub,_("Character encodings"));

    for (i=0;i<CSL_NUM_CHAR_ENCODINGS;i++)
    {
        s.Empty();
        s<<CslCharEncodings[i].Name<<wxT(" (")<<CslCharEncodings[i].Encoding<<wxT(")");
        CslMenu::AddItem(sub,MENU_ENCODING_START+i,s,wxART_NONE,wxITEM_CHECK);
    }

    i=m_channel.Encoding.GetEncodingId();
    CslMenu::CheckItem(*sub,MENU_ENCODING_START+i);

    point=ScreenToClient(point);
    PopupMenu(&menu,point);
}

void CslIrcPanel::OnSplitter(wxSplitterEvent& WXUNUSED(event))
{
    list_ctrl_users->SetColumnWidth(0,list_ctrl_users->GetBestSize().x);
    LOG_DEBUG("\n");
}

void CslIrcPanel::OnTimer(wxTimerEvent& event)
{
    OnAutojoin(false);
}

void CslIrcPanel::OnIrcEvent(CslIrcEvent& event)
{
    wxUint32 u;
    wxString s;
    CslIrcPanel *panel;

    switch (event.Type)
    {
        case CslIrcEvent::ERR:
            switch (event.Ints.Item(0))
            {
                case LIBIRC_ERR_RESOLV:
                    s<<wxT("\0034*** ")<<_("Couldn't resolve hostname.")<<wxT("\003");
                    AddLine(s);
                    EndIrcSession();
                    break;

                case LIBIRC_ERR_CLOSED:
                case LIBIRC_ERR_TERMINATED:
                    OnDisconnect();
                    break;
            }
            break;

        case CslIrcEvent::TOPIC:
        {
            if (!(panel=GetPanel(event.Channel,m_session)))
                break;
            if (event.Strings.GetCount()>1)
            {
                u=1;
                s<<wxT("\00313*** ")<<event.Strings.Item(0)<<wxT(" ")<<_("changed topic to");
                s<<wxT(": ")<<event.Strings.Item(1)<<wxT("\003");
                panel->AddLine(s);
            }
            else
                u=0;
            panel->m_channel.Topic=event.Strings.Item(u);
            //panel->static_topic->SetLabel(FormatToText(panel->m_channel.Topic));
            wxSizeEvent evt;
            wxPostEvent(panel,evt);
            //panel->static_topic->Wrap(panel->static_topic->GetSize().GetWidth()-2);
            //grid_sizer_chat->Layout();
            break;
        }

        case CslIrcEvent::NUMERIC:
            switch (event.Ints.Item(0))
            {
                case LIBIRC_RFC_ERR_CANNOTSENDTOCHAN:
                    s<<wxT("***\0034 ")<<event.Strings.Item(0)<<wxT("\003");
                    GetOrCreatePanel(event.Channel,m_session)->AddLine(s);
                    break;

                case 333: //topic info "channel nickname time"
                    break;

                default:
                    if (!event.Channel.IsEmpty())
                    {
                        if (!(panel=GetActivePanel(m_session)))
                            panel=this;
                        s<<wxT("\0035*** ")<<event.Channel<<wxT("\003");
                        panel->AddLine(s);
                    }
                    break;
            }
            break;

        case CslIrcEvent::CONNECT:
            s<<wxT("\0033*** ")<<_("Connection established...")<<wxT("\003");
            AddLine(s);
            OnAutojoin(true);
            break;

        case CslIrcEvent::JOIN:
            s<<wxT("\0035*** ")<<_("You joined channel.")<<wxT("\003");
            panel=GetOrCreatePanel(event.Channel,m_session);
            panel->AddLine(s);
            break;

        case CslIrcEvent::JOINED:
            s<<wxT("\0035*** ")<<event.Strings.Item(0);
            s<<wxT(" ")<<_("joined channel.")<<wxT("\003");
            panel=GetOrCreatePanel(event.Channel,m_session);
            panel->AddLine(s);
            panel->AddUser(event.Strings.Item(0));
            break;

        case CslIrcEvent::NAMES:
            panel=GetOrCreatePanel(event.Channel,m_session);
            for (u=0;u<event.Strings.GetCount();u++)
                panel->AddUser(event.Strings.Item(u));
            break;

        case CslIrcEvent::NICK:
            for (u=0;u<m_parent->GetPageCount();u++)
            {
                panel=(CslIrcPanel*)m_parent->GetPage(u);

                if (panel->m_session!=m_session)
                    continue;

                switch (panel->GetType())
                {
                    case TYPE_CHANNEL:
                        panel->ChangeUser(event.Channel,event.Strings.Item(0));
                        break;

                    case TYPE_PRIVATE:
                        if (panel->m_channel.Name==event.Channel)
                        {
                            panel->m_channel=event.Strings.Item(0);
                            m_parent->SetPageText(u,event.Strings.Item(0));
                        }
                        break;
                }
            }
            break;

        case CslIrcEvent::KICK:
            OnPart(event.Channel,
                   event.Strings.GetCount()>1 ? event.Strings.Item(1):wxString(wxEmptyString),
                   event.Strings.GetCount()>2 ? event.Strings.Item(2):wxString(wxEmptyString),
                   event.Strings.GetCount()>0 ? event.Strings.Item(0):wxString(wxEmptyString));
            break;

        case CslIrcEvent::QUIT:
            OnQuit(event.Channel,event.Strings.GetCount() ?
                   event.Strings.Item(0):wxString(wxEmptyString));
            break;

        case CslIrcEvent::PART:
            OnPart(event.Channel,event.Strings.Item(0),event.Strings.GetCount()>1 ?
                   event.Strings.Item(1):wxString(wxEmptyString));
            break;

        case CslIrcEvent::CHANMSG:
            s<<wxT("<")<<event.Strings.Item(0)<<wxT("> ")<<event.Strings.Item(1);
            GetOrCreatePanel(event.Channel,m_session)->AddLine(s);
            break;

        case CslIrcEvent::PRIVMSG:
            s<<wxT("<")<<event.Channel<<wxT("> ")<<event.Strings.Item(0);
            GetOrCreatePanel(event.Channel,m_session)->AddLine(s);
            break;

        case CslIrcEvent::NOTICE:
            s<<wxT("\0035*** NOTICE (")<<event.Strings.Item(0)<<wxT("):\017 ")<<event.Strings.Item(1);
            if (event.Strings.Item(0).IsEmpty() || !event.Channel.CmpNoCase(m_session->GetNick()))
                AddLine(s);
            else
                GetOrCreatePanel(event.Channel,m_session)->AddLine(s);
            break;

        case CslIrcEvent::ACTION:
            panel=GetOrCreatePanel(event.Channel,m_session);
            panel->AddLine(wxT("* \0032")+event.Strings.Item(0)+wxT("\003 ")+event.Strings.Item(1));
            break;

        case CslIrcEvent::MODE:
            GetOrCreatePanel(event.Channel,m_session)->OnUserModeChange(event.Strings.Item(0),
                    event.Strings.Item(1),event.Strings.Item(2));
            break;
    }
}

void CslIrcPanel::OnFocus()
{
    text_ctrl_input->SetFocus();
    text_ctrl_chat->ShowPosition(m_textLength);

    if (m_missedLines)
    {
        wxUint32 i;

        for (i=0;i<m_parent->GetPageCount();i++)
        {
            if (m_parent->GetPage(i)==this)
            {
                wxString s=m_parent->GetPageText(i).BeforeFirst(wxT(' '));
                m_parent->SetPageText(i,s);
                break;
            }
        }
        m_missedLines=0;
    }
}

void CslIrcPanel::OnUserModeChange(const wxString& initiator,const wxString& target,const wxString& mode)
{
    wxInt32 i;
    CslIrcUser *user;
    wxString s,msg,targetname;

    if ((user=ListFindUser(target,&i)))
    {
        if (!mode.CmpNoCase(wxT("+o")))
        {
            user->Status|=CslIrcUser::STATUS_OP;
            s=_("gives operator status to");
        }
        else if (!mode.CmpNoCase(wxT("-o")))
        {
            user->Status&=~CslIrcUser::STATUS_OP;
            s=_("takes operator status from");
        }
        else if (!mode.CmpNoCase(wxT("+v")))
        {
            user->Status|=CslIrcUser::STATUS_VOICE;
            s=_("gives voice status to");
        }
        else if (!mode.CmpNoCase(wxT("-v")))
        {
            user->Status&=~CslIrcUser::STATUS_VOICE;
            s=_("takes voice status from");
        }
        else
            return;

        ChangeUser(user->Nick,user->Nick,i);

        msg<<wxT("*** \0036")<<initiator<<wxT(" ")<<s<<wxT(" ")<<user->Nick<<wxT(".\003");
        AddLine(msg);
    }
}

void CslIrcPanel::OnAutojoin(bool start)
{
    if (!m_session || !m_session->GetContext().Server)
        return;

    static wxUint32 join;
    CslIrcChannels channels;

    if (start)
        join=0;

    if (m_rejoin)
        channels=m_session->GetChannels();
    else
        channels=m_session->GetContext().Server->Network->AutoChannels;

    if (join<channels.GetCount())
    {
        CslIrcChannel *channel=channels.Item(join);

        m_session->JoinChannel(channel->Name,channel->Password);

        if (++join<channels.GetCount())
        {
            m_timer.Start(500+join*250,true);
            return;
        }
    }

    if (m_rejoin)
        m_rejoin=false;

    join=0;
}

void CslIrcPanel::OnQuit(const wxString& nick,const wxString& reason)
{
    wxUint32 i;
    wxString s;
    CslIrcPanel *panel;

    s<<wxT("\0035*** ")<<nick<<wxT(" ")<<_("left server");
    if (!reason.IsEmpty())
        s<<wxT(" (")<<reason<<wxT(")\003");

    for (i=0;i<m_parent->GetPageCount();i++)
    {
        panel=(CslIrcPanel*)m_parent->GetPage(i);

        if (panel->m_session==m_session && panel->ListFindUser(nick))
        {
            panel->AddLine(s);
            panel->RemoveUser(nick);
        }
    }
}

void CslIrcPanel::OnPart(const wxString& channel,const wxString& nick,
                         const wxString& reason,const wxString& initiator)
{
    if (!m_session->GetNick().CmpNoCase(nick))
        return;

    wxString s;
    CslIrcPanel *panel;

    s<<wxT("\0035*** ")<<nick<<wxT(" ");
    if (initiator.IsEmpty())
        s<<_("left channel");
    else
        s<<_("got kicked by")<<wxT(" ")<<initiator;
    if (!reason.IsEmpty())
        s<<wxT(" (")<<reason<<wxT(")");
    s<<wxT(".\003");

    panel=GetOrCreatePanel(channel,m_session);
    panel->AddLine(s);
    panel->RemoveUser(nick);
}

void CslIrcPanel::OnDisconnect(bool forced)
{
    wxUint32 i;

    for (i=0;i<m_parent->GetPageCount();i++)
    {
        CslIrcPanel *pane=(CslIrcPanel*)m_parent->GetPage(i);

        if (pane->m_session==m_session && pane->GetType()==TYPE_CHANNEL)
            pane->RemoveUsers();
    }

    wxString s;

    if (!forced)
    {
        s<<wxT("\0034*** ")<<_("Connecting lost. Reconnecting...")<<wxT("\003");
        m_rejoin=true;
    }
    else
        s<<wxT("\0032*** ")<<_("Disconnected from server.")<<wxT("\003");

    AddLine(s);
}

CslIrcUser* CslIrcPanel::ListFindUser(const wxString& name,wxInt32 *index)
{
    wxInt32 i;
    CslIrcUser *user;

    for (i=0;i<list_ctrl_users->GetItemCount();i++)
    {
        user=(CslIrcUser*)list_ctrl_users->GetItemData(i);

        if (!user->Nick.CmpNoCase(name))
        {
            if (index)
                *index=i;
            return user;
        }
    }

    if (index)
        *index=wxNOT_FOUND;

    return NULL;
}

void CslIrcPanel::AddUser(const wxString& name)
{
    CslIrcUser *user=new CslIrcUser();

    if (name.StartsWith(wxT("@")))
    {
        user->Nick=name.Mid(1);
        user->Status=CslIrcUser::STATUS_OP;
    }
    else if (name.StartsWith(wxT("+")))
    {
        user->Nick=name.Mid(1);
        user->Status=CslIrcUser::STATUS_VOICE;
    }
    else
        user->Nick=name;

    list_ctrl_users->InsertItem(0,name);
    list_ctrl_users->SetItemData(0,(long)user);
    list_ctrl_users->SortItems(ListSortCompareFunc,0);
}

void CslIrcPanel::RemoveUser(const wxString& name)
{
    wxInt32 i;
    CslIrcUser *user;

    if ((user=ListFindUser(name,&i)))
    {
        list_ctrl_users->DeleteItem(i);
        list_ctrl_users->SortItems(ListSortCompareFunc,0);
        delete user;
    }
}

void CslIrcPanel::RemoveUsers()
{
    wxInt32 i;

    for (i=list_ctrl_users->GetItemCount();i>0;i--)
    {
        CslIrcUser *user=(CslIrcUser*)list_ctrl_users->GetItemData(i-1);
        list_ctrl_users->DeleteItem(i-1);
        delete user;
    }
}

void CslIrcPanel::ChangeUser(const wxString& oldName,const wxString& newName,wxInt32 index)
{
    wxString name;
    CslIrcUser *user;

    if (index==wxNOT_FOUND)
        user=ListFindUser(oldName,&index);
    else
        user=(CslIrcUser*)list_ctrl_users->GetItemData(index);

    if (user)
    {
        if (user->Status&CslIrcUser::STATUS_OP)
            name=wxT("@")+newName;
        else if (user->Status&CslIrcUser::STATUS_VOICE)
            name=wxT("+")+newName;
        else
            name=newName;

        user->Nick=newName;
        list_ctrl_users->SetItemText(index,name);
        list_ctrl_users->SortItems(ListSortCompareFunc,0);
    }
}

CslIrcPanel* CslIrcPanel::GetActivePanel(CslIrcSession *session)
{
    CslIrcPanel *panel=(CslIrcPanel*)m_parent->GetPage(m_parent->GetSelection());

    if (panel->m_session==session)
        return panel;

    return NULL;
}

CslIrcPanel* CslIrcPanel::GetPanel(const wxString& channel,CslIrcSession *session)
{
    wxUint32 i;
    CslIrcPanel *panel;

    for (i=0;i<m_parent->GetPageCount();i++)
    {
        panel=(CslIrcPanel*)m_parent->GetPage(i);

        if (panel->m_session==session &&
            panel->m_channel.Name.CmpNoCase(channel)==0)
            return panel;
    }

    return NULL;
}

CslIrcPanel* CslIrcPanel::GetOrCreatePanel(const wxString& channel,CslIrcSession *session)
{
    wxUint32 i,type;
    wxString name;
    CslIrcPanel *panel;

    wxASSERT(!channel.IsEmpty());

    type=session ? (channel.StartsWith(wxT("#")) ?
                    TYPE_CHANNEL : TYPE_PRIVATE) : TYPE_SESSION;

    if (channel.StartsWith(wxT("@")) || channel.StartsWith(wxT("+")))
        name=channel.Mid(1);
    else
        name=channel;

    for (i=0;i<m_parent->GetPageCount();i++)
    {
        panel=(CslIrcPanel*)m_parent->GetPage(i);

        if (!session)
        {
            if (panel->m_type==TYPE_NONE)
            {
                panel->m_type=TYPE_SESSION;
                m_parent->SetPageText(i,name);
                return panel;
            }
            continue;
        }

        if (panel->m_type==type &&
            panel->m_session==session &&
            panel->m_channel.Name.CmpNoCase(name)==0)
            return panel;
    }

    panel=new CslIrcPanel(m_parent,session,type,name);
    m_parent->AddPage(panel,name,true);
    panel->OnFocus();

    return panel;
}

bool CslIrcPanel::GetIRCColour(const char *str,wxInt32& fg,wxInt32& bg,wxUint32& len)
{
    fg=-1; bg=-1; len=0;

    if (*str && isdigit(*str))
    {
        fg=*str-0x30;
        str++; len++;

        if (*str && isdigit(*str))
        {
            fg=fg*10+(*str-0x30);
            str++; len++;
        }
    }
    if (*str && *str==',')
    {
        str++; len++;

        if (*str && isdigit(*str))
        {
            bg=*str-0x30;
            str++; len++;
        }

        if (*str && isdigit(*str))
        {
            bg=bg*10+(*str-0x30);
            len++;
        }
    }

    return len && (fg+bg>=-1);
}

void CslIrcPanel::AddLine(const wxString& line,bool scroll)
{
    wxChar c;
    wxFont font;
    wxString buf;
    wxTextAttrEx attr,basic;
    wxInt32 fg=-1,bg=-1;
    wxUint32 i,j,len=line.Length();
    wxDateTime now=wxDateTime::Now();
    bool bold=false,italic=false,strike=false,underline=false;
    static wxUint32 lines=0;

#ifdef CSL_USE_RICHTEXT
    scroll|=text_ctrl_chat->IsPositionVisible(m_textLength);
    basic=text_ctrl_chat->GetBasicStyle();
#else
    wxInt32 pos=text_ctrl_chat->GetScrollPos(wxVERTICAL);
    wxInt32 range=text_ctrl_chat->GetScrollRange(wxVERTICAL);
    wxInt32 height=text_ctrl_chat->GetClientSize().y;
    scroll|=(range-pos<=height+1);
#if 0
    LOG_DEBUG("pos: %d - range:%d - height:%d - range-pos:%d -  scroll:%d\n",
              pos,range,height,range-pos,scroll);
#endif //0
#ifdef __WXGTK__
    if (!scroll)
        text_ctrl_chat->SetWindowStyle(text_ctrl_chat->GetWindowStyle()&~wxTE_READONLY);
#endif //__WXGTK__
    basic=text_ctrl_chat->GetDefaultStyle();
#endif //CSL_USE_RICHTEXT

    text_ctrl_chat->Freeze();

    attr=basic;

    if (m_textLength)
        buf=wxT("\n");

    buf+=wxString::Format(wxT("[%-2.2d:%-2.2d] "),now.GetHour(),now.GetMinute());

#define WRITEBUFFER \
    if (!buf.IsEmpty()) \
    { \
        text_ctrl_chat->AppendText(buf); \
        m_textLength+=buf.Length(); \
        buf.Empty(); \
    }

    for (i=0;i<len;i++)
    {
        c=line.GetChar(i);

        if (c==wxT('\x02')) //bold
        {
            WRITEBUFFER
            font=attr.GetFont();
            font.SetWeight(bold ? wxNORMAL:wxBOLD);
            attr.SetFont(font);
            text_ctrl_chat->SetDefaultStyle(attr);
            bold=!bold;
            continue;
        }
        else if (c==wxT('\x03')) //color
        {
            WRITEBUFFER

            wxUint32 l;
            wxInt32 f,b;

            GetIRCColour(U2A(line.Mid(i+1,5)),f,b,l);

            if (f<0)
            {
                fg=-1;
                wxColour colour=basic.GetTextColour();
                attr.SetTextColour(basic.GetTextColour());
            }
            else
                attr.SetTextColour(IrcColours[(fg=f)]);

            if (b<0)
            {
                bg=-1;
                attr.SetBackgroundColour(basic.GetBackgroundColour());
            }
            else
                attr.SetBackgroundColour(IrcColours[(bg=b)]);

            text_ctrl_chat->SetDefaultStyle(attr);

            i+=l;
            continue;
        }
        else if (c==wxT('\x09')) //italic
        {
            WRITEBUFFER
            font=attr.GetFont();
            font.SetStyle(italic ? wxNORMAL:wxITALIC);
            attr.SetFont(font);
            text_ctrl_chat->SetDefaultStyle(attr);
            italic=!italic;
            continue;
        }
        else if (c==wxT('\x0F')) //reset
        {
            WRITEBUFFER
            fg=-1;
            bg=-1;
            bold=false;
            italic=false;
            strike=false;
            underline=false;
            text_ctrl_chat->SetDefaultStyle(basic);
            attr=basic;
            continue;
        }
        else if (c==wxT('\x13')) //striked
        {
            WRITEBUFFER
            attr.SetTextEffectFlags(attr.GetFlags()|wxTEXT_ATTR_EFFECTS|wxTEXT_ATTR_EFFECT_STRIKETHROUGH);
            if (!strike)
                attr.SetTextEffects(wxTEXT_ATTR_EFFECT_STRIKETHROUGH);
            else
                attr.SetTextEffects(attr.GetTextEffects()&~wxTEXT_ATTR_EFFECT_STRIKETHROUGH);
            text_ctrl_chat->SetDefaultStyle(attr);
            strike=!strike;
            continue;
        }
        else if (c==wxT('\x16')) //reverse
        {
            continue;
        }
        else if (c==wxT('\x1F')) //underline
        {
            WRITEBUFFER
            font=attr.GetFont();
            font.SetUnderlined(underline ? false:true);
            attr.SetFont(font);
            text_ctrl_chat->SetDefaultStyle(attr);
            underline=!underline;
            continue;
        }
        else if (c==wxT('\n') || c==wxT('\r'))
            continue;

        buf+=c;
    }

    WRITEBUFFER
    lines++;

    if ((i=line.Find(CSL_URI_SCHEME_STR))!=(wxUint32)wxNOT_FOUND)
    {
        for (j=i+6;j<len;j++)
        {
            if (line.GetChar(j)==wxT(' '))
                break;
        }

        attr=basic;
        font=attr.GetFont();
        font.SetUnderlined(true);
        attr.SetFont(font);
        attr.SetTextColour(*wxBLUE);
        text_ctrl_chat->SetStyle(m_textLength-len+i,m_textLength-len+i+(j-i),attr);
    }

    text_ctrl_chat->SetDefaultStyle(basic);

#ifdef CSL_USE_RICHTEXT
    text_ctrl_chat->Thaw();
#endif //CSL_USE_RICHTEXT
    if (scroll)
    {
        text_ctrl_chat->ScrollLines(lines+1);
        text_ctrl_chat->ShowPosition(m_textLength);
    }
#ifndef CSL_USE_RICHTEXT
#ifdef __WXGTK__
    else
        text_ctrl_chat->SetWindowStyle(text_ctrl_chat->GetWindowStyle()|wxTE_READONLY);
#endif //__WXGTK__
    text_ctrl_chat->Thaw();
#endif //CSL_USE_RICHTEXT

    if (!IsShownOnScreen())
    {
        m_missedLines++;

        for (i=0;i<m_parent->GetPageCount();i++)
        {
            if (m_parent->GetPage(i)==this)
            {
                buf=m_parent->GetPageText(i).BeforeFirst(wxT(' '));
                buf<<wxT(" (")<<m_missedLines<<wxT(")");
                m_parent->SetPageText(i,buf);
                break;
            }
        }
    }
}

wxString CslIrcPanel::FormatToText(const wxString& text)
{
    wxChar c;
    wxString buf;
    wxUint32 i,len=text.Length();

    for (i=0;i<len;i++)
    {
        c=text.GetChar(i);

        if (c==wxT('\x02')) //bold
            continue;
        else if (c==wxT('\x03')) //color
        {
            wxUint32 l;
            wxInt32 f,b;

            GetIRCColour(U2A(text.Mid(i+1,5)),f,b,l);
            i+=l;
            continue;
        }
        else if (c==wxT('\x09')) //italic
            continue;
        else if (c==wxT('\x0F')) //reset
            continue;
        else if (c==wxT('\x13')) //striked
            continue;
        else if (c==wxT('\x16')) //reverse
            continue;
        else if (c==wxT('\x1F')) //underline
            continue;
        else if (c==wxT('\n') || c==wxT('\r'))
            continue;

        buf+=c;
    }

    return buf;
}

wxString& CslIrcPanel::TextToFormat(wxString& text)
{
    text.Replace(wxT("%B"),wxT("\002"));
    text.Replace(wxT("%C"),wxT("\003"));
    text.Replace(wxT("%I"),wxT("\011"));
    text.Replace(wxT("%O"),wxT("\017"));
    text.Replace(wxT("%R"),wxT("\026"));
    text.Replace(wxT("%S"),wxT("\023"));
    text.Replace(wxT("%U"),wxT("\037"));

    return text;
}

void CslIrcPanel::EndIrcSession()
{
    if (!m_session)
        return;

    if (m_type==TYPE_SESSION)
    {
        if (m_timer.IsRunning())
            m_timer.Stop();

        m_rejoin=false;

        if (m_session->GetState()==CslIrcSession::STATE_CONNECTED)
        {
            m_session->Disconnect();
            OnDisconnect(true);
        }
    }
    else if (m_type==TYPE_CHANNEL)
        m_session->LeaveChannel(m_channel.Name);
}

void CslIrcPanel::HandleInput(const wxString& text)
{
    if (text.IsEmpty())
        return;

    wxString s;

    m_commandBuffer.Add(text);
    m_commandBufferPos=m_commandBuffer.GetCount();

    if (text.StartsWith(wxT("/")))
    {
        if (text.Mid(1,5).CmpNoCase(wxT("JOIN "))==0)
        {
            wxString channel,password;
            wxStringTokenizer tkz(text,wxT(" \t"));

            tkz.GetNextToken();
            channel=tkz.GetNextToken();
            password=tkz.GetNextToken();

            if (channel.IsEmpty())
            {
                s<<wxT("\0034*** ")<<_("Channel missing.")<<wxT("\003");
                AddLine(s,true);
                return;
            }
            else if (m_session)
            {
                m_session->JoinChannel(channel,password);
                return;
            }
        }
        else if (text.Mid(1,5).CmpNoCase(wxT("PART "))==0)
        {
            wxString channel,reason;
            wxStringTokenizer tkz(text,wxT(" \t"));

            tkz.GetNextToken();
            channel=tkz.GetNextToken();
            reason=tkz.GetNextToken();

            if (channel.IsEmpty())
            {
                s<<wxT("\0034*** ")<<_("Channel missing.")<<wxT("\003");
                AddLine(s,true);
                return;
            }
            else if (m_session)
            {
                CslIrcPanel *panel;

                if ((panel=GetPanel(channel,m_session)) &&
                    m_session->LeaveChannel(channel,reason))
                {
                    panel->RemoveUsers();
                    s<<wxT("\0035*** ")<<_("Left channel.")<<wxT("\003");
                    panel->AddLine(s,true);
                }
                return;
            }
        }
        else if (text.Mid(1,5).CmpNoCase(wxT("QUIT "))==0)
        {
            wxString reason;
            wxStringTokenizer tkz(text,wxT(" \t"));

            tkz.GetNextToken();
            reason=tkz.GetNextToken();

            if (m_session)
            {
                EndIrcSession();
                return;
            }
        }
        else if (text.Mid(1,5).CmpNoCase(wxT("NICK "))==0)
        {
            wxString nick;
            wxStringTokenizer tkz(text,wxT(" \t"));
            tkz.GetNextToken();

            if (!tkz.HasMoreTokens())
            {
                s<<wxT("\0034*** ")<<_("Nick missing.")<<wxT("\003");
                AddLine(s,true);
                return;
            }
            else if (m_session)
            {
                m_session->ChangeNick(tkz.GetNextToken());
                return;
            }
        }
        else if (text.Mid(1,3).CmpNoCase(wxT("ME "))==0)
        {
            if (m_type!=TYPE_SESSION)
            {
                if (m_session)
                {
                    s=text.Mid(4);
                    TextToFormat(s);
                    if (m_session->SendCtcpAction(m_channel.Name,s))
                        AddLine(wxT("* \0032")+m_session->GetNick()+wxT("\003 ")+s,true);
                }
                return;
            }
        }

        if (m_session)
        {
            m_session->SendRawCommand(text.Mid(1));
            return;
        }
        else
            s<<wxT("\0034")<<text<<wxT(": ")<<_("*** Not connected.")<<wxT("\003");
    }
    else if (m_session)
    {
        if (m_type!=TYPE_SESSION && m_session->GetState()==CslIrcSession::STATE_CONNECTED)
        {
            s=text;
            TextToFormat(s);
            m_session->SendTextMessage(m_channel.Name,s);
            s=wxT("<")+m_session->GetNick()+wxT(">\0033 ")+s+wxT("\003");
        }
    }

    AddLine(s.IsEmpty() ? text:s,false);
}

int wxCALLBACK CslIrcPanel::ListSortCompareFunc(long item1,long item2,long data)
{
    CslIrcUser *user1,*user2;

    user1=(CslIrcUser*)item1;
    user2=(CslIrcUser*)item2;

    if (user1->Status>user2->Status)
        return -1;
    else if (user1->Status<user2->Status)
        return 1;

    return user1->Nick.Cmp(user2->Nick);
}
