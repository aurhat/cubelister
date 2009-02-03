/***************************************************************************
 *   Copyright (C) 2007 -2009 by Glen Masgai                               *
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

#ifndef CSLIRC_H
#define CSLIRC_H

//allows stroken text but the memory usage nearly explodes
//#define CSL_USE_RICHTEXT

#include "wx/wxprec.h"
#ifdef __BORLANDC__
#pragma hdrstop
#endif //__BORLANDC__
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif //WX_PRECOMP
#include <wx/listctrl.h>
#include <wx/aui/aui.h>
#include <wx/splitter.h>
#ifdef CSL_USE_RICHTEXT
#include <wx/richtext/richtextctrl.h>
#else
#include <wx/richtext/richtextbuffer.h>
#endif //CSL_USE_RICHTEXT
#include "irc/CslIRCEngine.h"
#include "engine/CslTools.h"

/**
 @author Glen Masgai <mimosius@users.sourceforge.net>
*/


class CslIrcNotebook : public wxAuiNotebook
{
    public:
        CslIrcNotebook(wxWindow* parent,wxWindowID id=wxID_ANY,
                       long style=wxAUI_NB_DEFAULT_STYLE);
        ~CslIrcNotebook();

        static CslIrcEngine* GetEngine() { return m_engine; }

    protected:
        static CslIrcEngine *m_engine;

        void OnNotebookPageSelected(wxAuiNotebookEvent& event);
        void OnNotebookPageClose(wxAuiNotebookEvent& event);

        DECLARE_EVENT_TABLE()
};


class CslIrcPanel: public wxPanel
{
    public:
        enum { TYPE_NONE, TYPE_SESSION, TYPE_CHANNEL, TYPE_PRIVATE };
        enum { CHOICE_CONNECTION = wxID_HIGHEST+1000, TEXT_CTRL_INPUT, MENU_TAB };

        CslIrcPanel(CslIrcNotebook* parent,CslIrcSession *session=NULL,
                    wxInt32 type=TYPE_NONE,const wxString& channel=wxEmptyString);
        ~CslIrcPanel();

        void OnFocus();
        void EndIrcSession();
        void HandleInput(const wxString& text);

        wxUint32 GetType() { return m_type; }
        CslIrcSession* GetSession() { return m_session; }

    private:
        CslIrcNotebook *m_parent;
        CslIrcSession *m_session;
        CslIrcChannel m_channel;
        wxUint32 m_type;
        wxUint32 m_textLength;
        wxUint32 m_missedLines;
        wxUint32 m_commandBufferPos;
        wxArrayString m_commandBuffer;
        wxTimer m_timer;
        bool m_rejoin;

        void OnCtcpAction(const wxString& nick,const wxString& text,bool own);
        void OnUserModeChange(const wxString& initiator,const wxString& target,const wxString& mode);
        void OnAutojoin(bool start);
        void OnQuit(const wxString& nick,const wxString& reason);
        void OnPart(const wxString& channel,const wxString& nick,const wxString& reason,
                    const wxString& initiator=wxEmptyString);
        void OnDisconnect(bool force=false);

        CslIrcPanel* GetActivePanel(CslIrcSession *session);
        CslIrcPanel* GetPanel(const wxString& channel,CslIrcSession *session);
        CslIrcPanel* GetOrCreatePanel(const wxString& channel,CslIrcSession *session);

        bool GetIRCColour(const char *str,wxInt32& fg,wxInt32& bg,wxUint32& len);
        void AddLine(const wxString& line,bool scroll=false);
        wxString FormatToText(const wxString& text);
        wxString& TextToFormat(wxString& text);

        CslIrcUser* ListFindUser(const wxString& name,wxInt32 *index=NULL);
        void AddUser(const wxString& name);
        void RemoveUser(const wxString& name);
        void RemoveUsers();
        void ChangeUser(const wxString& old,const wxString& name,wxInt32 index=wxNOT_FOUND);

    protected:
        wxFlexGridSizer *grid_sizer_chat;
        wxPanel* pane_chat;
        wxStaticText *static_topic;
#ifdef CSL_USE_RICHTEXT
        wxRichTextCtrl* text_ctrl_chat;
#else
        wxTextCtrl *text_ctrl_chat;
#endif
        wxPanel *pane_users;
        wxListCtrl *list_ctrl_users;
        wxChoice *choice_connection;
        wxTextCtrl *text_ctrl_input;
        wxSplitterWindow *splitter;

        static int wxCALLBACK ListSortCompareFunc(long item1,long item2,long data);

        void OnKeypress(wxKeyEvent& event);
        void OnListCtrlActivated(wxListEvent& event);
        void OnSize(wxSizeEvent& event);
        void OnCommandEvent(wxCommandEvent& event);
        void OnSplitter(wxSplitterEvent& event);
        void OnTimer(wxTimerEvent& event);
        void OnIrcEvent(CslIrcEvent& event);

        DECLARE_EVENT_TABLE()
};

#endif //CSLIRC_H
