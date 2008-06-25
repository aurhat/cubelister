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

#include <wx/clipbrd.h>
#include <wx/txtstrm.h>
#include <wx/wupdlock.h>
#include "CslDlgAddServer.h"
#include "CslDlgConnectWait.h"
#include <CslDlgConnectPass.h>
#include "CslDlgOutput.h"
#include "CslListCtrlServer.h"
#include "CslStatusBar.h"
#include "CslArt.h"
#include "CslMenu.h"
#include "CslSettings.h"

#include "img/ext_green_8.xpm"
#include "img/sortasc_16.xpm"
#include "img/sortdsc_16.xpm"
#include "img/sortasclight_16.xpm"
#include "img/sortdsclight_16.xpm"
#include "img/red_list_16.xpm"
#include "img/green_list_16.xpm"
#include "img/yellow_list_16.xpm"
#include "img/grey_list_16.xpm"
#include "img/red_ext_list_16.xpm"
#include "img/green_ext_list_16.xpm"
#include "img/yellow_ext_list_16.xpm"

enum
{
    SORT_HOST = 0, SORT_DESCRIPTION, SORT_VER,
    SORT_PING, SORT_MODE, SORT_MAP, SORT_TIME,
    SORT_PLAYER, SORT_MM, SORT_UNKNOWN
};

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE(wxCSL_EVT_PROCESS,wxID_ANY)
END_DECLARE_EVENT_TYPES()

#define CSL_EVT_PROCESS(id,fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
                               wxCSL_EVT_PROCESS,id,wxID_ANY, \
                               (wxObjectEventFunction)(wxEventFunction) \
                               wxStaticCastEvent(wxCommandEventFunction,&fn), \
                               (wxObject*)NULL \
                             ),

DEFINE_EVENT_TYPE(wxCSL_EVT_PROCESS)


BEGIN_EVENT_TABLE(CslListCtrlServer, wxListCtrl)
    EVT_SIZE(CslListCtrlServer::OnSize)
    EVT_MENU(wxID_ANY,CslListCtrlServer::OnMenu)
    EVT_LIST_COL_CLICK(wxID_ANY,CslListCtrlServer::OnColumnLeftClick)
    EVT_LIST_ITEM_ACTIVATED(wxID_ANY,CslListCtrlServer::OnItemActivated)
    EVT_LIST_ITEM_SELECTED(wxID_ANY,CslListCtrlServer::OnItemSelected)
    EVT_LIST_ITEM_DESELECTED(wxID_ANY,CslListCtrlServer::OnItemDeselected)
    EVT_CONTEXT_MENU(CslListCtrlServer::OnContextMenu)
    EVT_KEY_DOWN(CslListCtrlServer::OnKeyDown)
    CSL_EVT_PROCESS(wxID_ANY,CslListCtrlServer::OnEndProcess)
    EVT_TIMER(wxID_ANY,CslListCtrlServer::OnTimer)
END_EVENT_TABLE()


bool               CslConnectionState::m_playing=false;
wxInt32            CslConnectionState::m_waitTime=0;
CslListCtrlServer* CslConnectionState::m_activeList=NULL;
CslServerInfo*     CslConnectionState::m_activeInfo=NULL;


class CslProcess : public wxProcess
{
    public:
        CslProcess(CslListCtrlServer *parent,CslServerInfo *info,const wxString& cmd) :
                wxProcess((CslListCtrlServer*)parent),
                m_parent(parent),m_info(info),m_cmd(cmd)
        {
            m_self=this;
            m_watch.Start(0);
            Redirect();
        }

        virtual void OnTerminate(int pid,int code)
        {
            m_watch.Pause();

            wxUint32 time=m_watch.Time()/1000;
            if (time>g_cslSettings->minPlaytime)
                m_info->SetLastPlayTime(time);

            // Cube returns with 1 - weird
            if (!m_info->GetGame().ReturnOk(code))
                wxMessageBox(wxString::Format(_("%s returned with code: %d"),
                                              m_cmd.c_str(),code),_("Error"),wxICON_ERROR,m_parent);

            ProcessInputStream();
            ProcessErrorStream();

            if (g_cslSettings->autoSaveOutput && !g_cslSettings->gameOutputPath.IsEmpty())
                CslDlgOutput::SaveFile(g_cslSettings->gameOutputPath);

            wxCommandEvent evt(wxCSL_EVT_PROCESS);
            evt.SetClientData(m_info);
            wxPostEvent(m_parent,evt);

            m_self=NULL;

            delete this;
        }

        static void ProcessInputStream()
        {
            if (!m_self)
                return;

            wxInputStream *stream=m_self->GetInputStream();
            if (!stream)
                return;

            while (stream->CanRead())
            {
                char buf[1025];
                stream->Read((void*)buf,1024);
                wxUint32 last=stream->LastRead();
                if (last==0)
                    break;
                buf[last]=0;
                //Cube has color codes in it's outputm
                m_self->m_info->GetGame().ProcessOutput(buf,&last);
                CslDlgOutput::AddOutput(buf,last);
                //LOG_DEBUG("%s", buf);
            }
        }

        static void ProcessErrorStream()
        {
            if (!m_self)
                return;

            wxInputStream *stream=m_self->GetErrorStream();
            if (!stream)
                return;

            while (stream->CanRead())
            {
                char buf[1025];
                stream->Read((void*)buf,1024);

                wxUint32 last=stream->LastRead();
                if (last==0)
                    break;
                buf[last]=0;
                /*
                //Cube has color codes in it's outputm
                m_self->m_info->GetGame().ProcessOutput(buf,&last);
                CslDlgOutput::AddOutput(buf,last);
                */
                LOG_DEBUG("%s",buf);
            }
        }

    protected:
        static CslProcess *m_self;
        CslListCtrlServer *m_parent;
        CslServerInfo *m_info;
        wxString m_cmd;
        bool m_clear;
        wxStopWatch m_watch;
};

CslProcess* CslProcess::m_self=NULL;


enum
{
    CSL_LIST_IMG_GREEN,
    CSL_LIST_IMG_YELLOW,
    CSL_LIST_IMG_RED,
    CSL_LIST_IMG_GREY,
    CSL_LIST_IMG_GREEN_EXT,
    CSL_LIST_IMG_YELLOW_EXT,
    CSL_LIST_IMG_RED_EXT,
    CSL_LIST_IMG_SORT_ASC,
    CSL_LIST_IMG_SORT_DSC,
    CSL_LIST_IMG_SORT_ASC_LIGHT,
    CSL_LIST_IMG_SORT_DSC_LIGHT,
    CSL_LIST_IMG_GAMES_START,
};

CslListCtrlServer::CslListCtrlServer(wxWindow* parent,wxWindowID id,const wxPoint& pos,
                                     const wxSize& size,long style,
                                     const wxValidator& validator, const wxString& name) :
        wxListCtrl(parent,id,pos,size,style,validator,name),
        m_id(id),m_engine(NULL),m_masterSelected(false),
        m_listInfo(NULL),m_sibling(NULL),
        m_dontUpdateInfo(false),m_dontRemoveOnDeselect(false),m_filterFlags(0)
{
    wxArtProvider::Push(new CslArt);
}

CslListCtrlServer::~CslListCtrlServer()
{
// avoids an assertion
#ifndef __WXMAC__
    ListClear();
#endif
}

void CslListCtrlServer::OnSize(wxSizeEvent& event)
{
    ListAdjustSize(event.GetSize());
    //wxPostEvent(m_parent,event);
    event.Skip();
}

void CslListCtrlServer::OnKeyDown(wxKeyEvent &event)
{
    wxInt32 i;
    wxInt32 code=event.GetKeyCode();

    if (event.ControlDown() && code=='A')
    {
        m_dontUpdateInfo=true;
        for (i=0;i<GetItemCount();i++)
            SetItemState(i,wxLIST_STATE_SELECTED,wxLIST_STATE_SELECTED);
        m_dontUpdateInfo=false;
    }
    else if (code==WXK_DELETE || code == WXK_NUMPAD_DELETE)
    {
        //TODO problem - multiple events
        if (event.ShiftDown())
            ListDeleteServers();
        else if (m_id!=CSL_LIST_MASTER)
            ListRemoveServers();
    }

    event.Skip();
}

void CslListCtrlServer::OnColumnLeftClick(wxListEvent& event)
{
    ListSort(event.GetColumn());
}

void CslListCtrlServer::OnItemActivated(wxListEvent& event)
{
    wxListItem item;
    item.SetId(event.GetIndex());
    CslListServerData *server=(CslListServerData*)GetItemData(item);
    CslServerInfo *info=server->Info;
    ConnectToServer(info);
}

void CslListCtrlServer::OnItemSelected(wxListEvent& event)
{
    event.Skip();

    wxListItem item;
    item.SetId(event.GetIndex());
    GetItem(item);

    CslListServerData *server=(CslListServerData*)GetItemData(item);
    m_selected.Add(server);

    if (m_dontUpdateInfo)
        return;

    event.SetClientData((void*)server->Info);
}

void CslListCtrlServer::OnItemDeselected(wxListEvent& event)
{
    event.Skip();

    if (m_dontRemoveOnDeselect)
        return;

    wxListItem item;
    item.SetId(event.GetIndex());
    GetItem(item);

    CslListServerData *server=(CslListServerData*)GetItemData(item);
    if (m_selected.Index(server)!=wxNOT_FOUND)
        m_selected.Remove(server);
}

void CslListCtrlServer::OnContextMenu(wxContextMenuEvent& event)
{
    wxInt32 selected;
    wxMenu menu,ext,filter,*pmenu=NULL;;
    wxMenuItem *item;
    CslServerInfo *info=NULL;
    wxPoint point=event.GetPosition();

    //from keyboard
    if (point.x==-1 && point.y==-1)
        point=wxGetMousePosition();

    if ((selected=m_selected.GetCount())>0)
        info=m_selected.Item(0)->Info;

    if (selected==1)
    {
        wxUint32 caps=info->GetGame().GetCapabilities();

        CslMenu::AddItem(&menu,MENU_SERVER_CONNECT,MENU_SERVER_CONN_STR,wxART_CONNECT);
        if (CSL_CAP_CONNECT_PASS(caps))
            CslMenu::AddItem(&menu,MENU_SERVER_CONNECT_PW,MENU_SERVER_CONN_PW_STR,wxART_CONNECT_PW);

        item=menu.AppendSubMenu(&ext,_("Extended information"));
        item->SetBitmap(GET_ART_MENU(wxART_ABOUT));
        if (info->ExtInfoStatus!=CSL_EXT_STATUS_OK || !PingOk(info))
            item->Enable(false);
        else
        {
            CslMenu::AddItem(&ext,MENU_SERVER_EXTENDED_FULL,_("Full"),wxART_ABOUT);
            ext.AppendSeparator();
            CslMenu::AddItem(&ext,MENU_SERVER_EXTENDED_MICRO,_("Micro"),wxART_EXTINFO_MICRO);
            CslMenu::AddItem(&ext,MENU_SERVER_EXTENDED_MINI,_("Mini"),wxART_EXTINFO_MINI);
            CslMenu::AddItem(&ext,MENU_SERVER_EXTENDED_DEFAULT,_("Default"),wxART_EXTINFO_DEFAULT);
        }

        menu.AppendSeparator();

        if (CslConnectionState::IsPlaying())
        {
            menu.Enable(MENU_SERVER_CONNECT,false);
            if (CSL_CAP_CONNECT_PASS(caps))
                menu.Enable(MENU_SERVER_CONNECT_PW,false);
        }
    }

    if (m_id==CSL_LIST_MASTER && selected)
        CslMenu::AddItem(&menu,MENU_SERVER_ADD,MENU_SERVER_MAS_ADD_STR,wxART_ADD_BOOKMARK);

    else if (m_id==CSL_LIST_FAVOURITE)
    {
        CslMenu::AddItem(&menu,MENU_SERVER_ADD,MENU_SERVER_FAV_ADD_STR,wxART_ADD_BOOKMARK);
        if (selected)
            CslMenu::AddItem(&menu,MENU_SERVER_REM,MENU_SERVER_FAV_REM_STR,wxART_DEL_BOOKMARK);
    }

    if (selected>0)
    {
        CslMenu::AddItem(&menu,MENU_SERVER_DEL,selected>1 ? MENU_SERVER_DELM_STR:MENU_SERVER_DEL_STR,wxART_DELETE);
        menu.AppendSeparator();

        CslMenu::AddItem(&menu,MENU_SERVER_COPYTEXT,MENU_SERVER_COPY_STR,wxART_COPY);
        menu.AppendSeparator();
        pmenu=(wxMenu*)1;
    }
    else if (m_id==CSL_LIST_FAVOURITE)
    {
        pmenu=(wxMenu*)1;
        menu.AppendSeparator();
    }

    if (pmenu)
    {
        pmenu=&filter;
        item=menu.AppendSubMenu(pmenu,MENU_SERVER_FILTER_STR);
        item->SetBitmap(GET_ART_MENU(wxART_FILTER));
    }
    else
        pmenu=&menu;

    item=&CslMenu::AddItem(pmenu,MENU_SERVER_FILTER_OFF,
                           MENU_SERVER_FILTER_OFF_STR,wxART_NONE,wxITEM_CHECK);
    item->Check(*m_filterFlags&CSL_FILTER_OFFLINE);
    item=&CslMenu::AddItem(pmenu,MENU_SERVER_FILTER_FULL,
                           MENU_SERVER_FILTER_FULL_STR,wxART_NONE,wxITEM_CHECK);
    item->Check(*m_filterFlags&CSL_FILTER_FULL);
    item=&CslMenu::AddItem(pmenu,MENU_SERVER_FILTER_EMPTY,
                           MENU_SERVER_FILTER_EMPTY_STR,wxART_NONE,wxITEM_CHECK);
    item->Check(*m_filterFlags&CSL_FILTER_EMPTY);
    item->Enable(!(*m_filterFlags&CSL_FILTER_NONEMPTY));
    item=&CslMenu::AddItem(pmenu,MENU_SERVER_FILTER_NONEMPTY,
                           MENU_SERVER_FILTER_NONEMPTY_STR,wxART_NONE,wxITEM_CHECK);
    item->Check(*m_filterFlags&CSL_FILTER_NONEMPTY);
    item->Enable(!(*m_filterFlags&CSL_FILTER_EMPTY));
    item=&CslMenu::AddItem(pmenu,MENU_SERVER_FILTER_MM2,
                           MENU_SERVER_FILTER_MM2_STR,wxART_NONE,wxITEM_CHECK);
    item->Check(*m_filterFlags&CSL_FILTER_MM2);
    item=&CslMenu::AddItem(pmenu,MENU_SERVER_FILTER_MM3,
                           MENU_SERVER_FILTER_MM3_STR,wxART_NONE,wxITEM_CHECK);
    item->Check(*m_filterFlags&CSL_FILTER_MM3);

    if (m_id==CSL_LIST_MASTER)
    {
        pmenu->AppendSeparator();
        item=&CslMenu::AddItem(pmenu,MENU_SERVER_FILTER_VER,MENU_SERVER_FILTER_VER_STR,wxART_NONE,wxITEM_CHECK);
        item->Enable(selected>0 || m_filterVersion>-1);
        item->Check(m_filterVersion>-1);
    }

    point=ScreenToClient(point);
    PopupMenu(&menu,point);
}

void CslListCtrlServer::ListRemoveServers()
{
    wxInt32 i;
    CslServerInfo *info;
    wxListItem item;

    for (i=m_selected.GetCount()-1;i>=0;i--)
    {
        CslListServerData *ls=m_selected.Item(i);
        info=ls->Info;

        if (ListFindItem(info,item)==wxNOT_FOUND)
            continue;

        ListDeleteItem(&item);
        m_selected.RemoveAt(i);
        m_servers.Remove(ls);

        switch (m_id)
        {
            case CSL_LIST_FAVOURITE:
                info->RemoveFavourite();
                break;
            default:
                break;
        }
    }
}

#define CSL_DELETE_YESNOCANCEL_STR _("\nChoose Yes to keep these servers, " \
                                     "No to delete them or\nCancel the operation.")

void CslListCtrlServer::ListDeleteServers()
{
    wxInt32 i;
    wxString msg,s;
    CslServerInfo *info;
    wxInt32 skipFav=wxCANCEL,skipStats=wxCANCEL;

    for (i=m_selected.GetCount()-1;i>=0;i--)
    {
        msg.Empty();
        CslListServerData *ls=m_selected.Item(i);

        if (skipFav==wxCANCEL && ls->Info->IsFavourite())
        {
            msg=_("You are about to delete servers which are also favourites!\n");
            msg+=CSL_DELETE_YESNOCANCEL_STR;
            skipFav=wxMessageBox(msg,_("Warning"),wxYES_NO|wxCANCEL|wxICON_WARNING,this);
            if (skipFav==wxCANCEL)
                return;
            if (skipFav!=wxNO)
                continue;
        }
        if (skipStats==wxCANCEL && ls->Info->HasStats())
        {
            msg=_("You are about to delete servers which have statistics!\n");
            msg+=CSL_DELETE_YESNOCANCEL_STR;
            skipStats=wxMessageBox(msg,_("Warning"),wxYES_NO|wxCANCEL|wxICON_WARNING,this);
            if (skipStats==wxCANCEL)
                return;
            if (skipStats!=wxNO)
                continue;
        }
    }

    for (i=m_selected.GetCount()-1;i>=0;i--)
    {
        wxListItem item;
        CslListServerData *ls=m_selected.Item(i);
        info=ls->Info;

        if (ListFindItem(info,item)==wxNOT_FOUND)
            continue;

        if (ls->Info->IsLocked())
        {
            msg=wxString::Format(_("Server \"%s\" is currently locked,\nso deletion is not possible!"),
                                 info->GetBestDescription().c_str());
            wxMessageBox(msg,_("Error"),wxICON_ERROR,this);
            continue;
        }

        if (info->IsFavourite() && skipFav!=wxNO)
            continue;

        if (info->HasStats() && skipStats!=wxNO)
            continue;

        RemoveServer(ls,info,i);

        if (m_id==CSL_LIST_MASTER && info->IsFavourite())
            m_sibling->RemoveServer(NULL,info,-1);
        else if (m_id==CSL_LIST_FAVOURITE)
            m_sibling->RemoveServer(NULL,info,-1);

        wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED,MENU_SERVER_DEL);
        event.SetClientData((void*)info);
        wxPostEvent(m_parent,event);
    }
}
#undef CSL_DELETE_YESNOCANCEL_STR

void CslListCtrlServer::OnMenu(wxCommandEvent& event)
{
    CslServerInfo *info;
    wxListItem item;
    wxString s;
    wxUint32 i=0;
    wxInt32 c=0;

    wxInt32 id=event.GetId();

    switch (id)
    {
        case MENU_SERVER_CONNECT:
            ConnectToServer(m_selected.Item(0)->Info);
            break;

        case MENU_SERVER_CONNECT_PW:
        {
            info=m_selected.Item(0)->Info;
            i=CSL_CAP_CONNECT_ADMIN_PASS(info->GetGame().GetCapabilities());
            CslConnectPassInfo pi(info->Password,info->PasswordAdmin,i!=0);
            if (CslDlgConnectPass(this,&pi).ShowModal()==wxID_OK)
            {
                info->Password=pi.password;
                info->PasswordAdmin=pi.passwordAdmin;
                ConnectToServer(info,pi.admin ? CslGame::CSL_CONNECT_ADMIN_PASS:CslGame::CSL_CONNECT_PASS);
            }
            break;
        }

        case MENU_SERVER_EXTENDED_MICRO:
        case MENU_SERVER_EXTENDED_MINI:
        case MENU_SERVER_EXTENDED_DEFAULT:
        case MENU_SERVER_EXTENDED_FULL:
        {
            info=m_selected.Item(0)->Info;
            event.SetClientData((void*)info);
            wxPostEvent(m_parent,event);
            break;
        }

        case MENU_SERVER_ADD:
        {
            switch (m_id)
            {
                case CSL_LIST_MASTER:
                {
                    bool sort=false;
                    for (i=0;i<m_selected.GetCount();i++)
                    {
                        info=m_selected.Item(i)->Info;

                        if (ListFindItem(info,item)==wxNOT_FOUND)
                            continue;
                        info->SetFavourite();
                        sort=true;
                        m_sibling->ListUpdateServer(info);
                        /*if (!info->IsDefault())
                        {
                            item.SetId(c);
                            DeleteItem(item);
                        }*/
                    }
                    if (sort)
                        m_sibling->ListSort();
                    break;
                }
                case CSL_LIST_FAVOURITE:
                {
                    CslDlgAddServer dlg((wxWindow*)this);
                    info=new CslServerInfo;
                    dlg.InitDlg(m_engine,info);
                    if (dlg.ShowModal()!=wxID_OK)
                    {
                        delete info;
                        break;
                    }
                    ListUpdateServer(info);
                    ListSort();
                    break;
                }
            }
            break;
        }

        case MENU_SERVER_REM:
        {
            ListRemoveServers();
            break;
        }

        case MENU_SERVER_DEL:
        {
            ListDeleteServers();
            break;
        }

        case MENU_SERVER_COPYTEXT:
        {
            wxString s1;
            item.SetMask(wxLIST_MASK_TEXT);

            for (i=0;i<m_selected.GetCount();i++)
            {
                info=m_selected.Item(i)->Info;

                if (ListFindItem(info,item)==wxNOT_FOUND)
                    continue;

                for (c=0;c<GetColumnCount();c++)
                {
                    item.SetColumn(c);
                    GetItem(item);
                    s1=item.GetText();
                    if (!s1.IsEmpty())
                        s+=s1+wxT("  ");
                }
                if (!s.IsEmpty())
                    s+=wxT("\r\n");
            }

            if (s.IsEmpty())
                break;

            if (wxTheClipboard->Open())
            {
                wxTheClipboard->SetData(new wxTextDataObject(s));
                wxTheClipboard->Close();
            }
            break;
        }

        case MENU_SERVER_FILTER_OFF:
            i=CSL_FILTER_OFFLINE;
        case MENU_SERVER_FILTER_FULL:
            if (i==0)
                i=CSL_FILTER_FULL;
        case MENU_SERVER_FILTER_EMPTY:
            if (i==0)
                i=CSL_FILTER_EMPTY;
        case MENU_SERVER_FILTER_NONEMPTY:
            if (i==0)
                i=CSL_FILTER_NONEMPTY;
        case MENU_SERVER_FILTER_MM2:
            if (i==0)
                i=CSL_FILTER_MM2;
        case MENU_SERVER_FILTER_MM3:
            if (i==0)
                i=CSL_FILTER_MM3;
        case MENU_SERVER_FILTER_VER:
        {
            if (event.IsChecked())
            {
                if (i==0)
                {
                    if (m_selected.GetCount())
                        m_filterVersion=m_selected.Item(0)->Info->Protocol;
                    break;
                }
                *m_filterFlags|=i;
            }
            else
            {
                if (i==0)
                {
                    m_filterVersion=-1;
                    break;
                }
                *m_filterFlags&=~i;
            }
            break;
        }

        default:
            break;
    }

    if (id>=MENU_SERVER_FILTER_OFF && id<=MENU_SERVER_FILTER_VER)
    {
        ListFilter();
        event.SetClientData((void*)m_id);
        wxPostEvent(m_parent,event);
    }
}

void CslListCtrlServer::OnEndProcess(wxCommandEvent& event)
{
    CslServerInfo *info=(CslServerInfo*)event.GetClientData();

    info->Lock(false);
    info->GetGame().GameEnd();
    CslConnectionState::Reset();

    ListUpdateServer(info);
    m_listInfo->UpdateInfo(info);
}

void CslListCtrlServer::OnTimer(wxTimerEvent& event)
{
    CslServerInfo *info=CslConnectionState::GetInfo();

    if (CslConnectionState::IsPlaying())
    {
        CslProcess::ProcessInputStream();
        CslProcess::ProcessErrorStream();
    }
    else if (CslConnectionState::IsWaiting())
    {
        if (info->Players!=info->PlayersMax)
        {
            CslConnectionState::Reset();
            ConnectToServer(info);
        }
        else if (m_timerCount%2==0)
        {
            if (CslConnectionState::CountDown())
                CslStatusBar::SetText(1,wxString::Format(_("Waiting %s for a free slot on \"%s\" " \
                                      "(press ESC to abort or join another server)"),
                                      FormatSeconds(CslConnectionState::GetWaitTime(),true,true).c_str(),
                                      info->GetBestDescription().c_str()));
            else
                CslStatusBar::SetText(1,wxT(""));
        }
    }
    m_timerCount++;
}

void CslListCtrlServer::ListInit(CslEngine *engine,
                                 CslListCtrlInfo *listInfo,
                                 CslListCtrlServer *sibling)
//CslDlgExtended *extendedDlg);
{
    m_engine=engine;
    m_listInfo=listInfo;
    m_sibling=sibling;
    m_filterFlags=m_id==CSL_LIST_MASTER ? &g_cslSettings->filterMaster:&g_cslSettings->filterFavourites;
    m_filterVersion=-1;

    m_stdColourListText=GetTextColour();
    m_stdColourListItem=GetBackgroundColour();

    ListCreateGameBitmaps();

    wxListItem item;

    item.SetMask(wxLIST_MASK_TEXT|wxLIST_MASK_FORMAT);
    item.SetImage(-1);

    item.SetAlign(wxLIST_FORMAT_LEFT);
    item.SetText(_("Host"));
    InsertColumn(0,item);
    SetColumn(0,item);

    item.SetText(_("Description"));
    InsertColumn(1,item);
    SetColumn(1,item);

    item.SetText(_("Version"));
    InsertColumn(2,item);
    SetColumn(2,item);

    item.SetAlign(wxLIST_FORMAT_RIGHT);
    item.SetText(_("Ping"));
    InsertColumn(3,item);
    SetColumn(3,item);

    item.SetAlign(wxLIST_FORMAT_LEFT);
    item.SetText(_("Mode"));
    InsertColumn(4,item);
    SetColumn(4,item);

    item.SetText(_("Map"));
    InsertColumn(5,item);
    SetColumn(5,item);

    item.SetAlign(wxLIST_FORMAT_RIGHT);
    item.SetText(_("Time"));
    InsertColumn(6,item);
    SetColumn(6,item);

    item.SetAlign(wxLIST_FORMAT_LEFT);
    item.SetText(_("Player"));
    InsertColumn(7,item);
    SetColumn(7,item);

    item.SetText(_("MM"));
    InsertColumn(8,item);
    SetColumn(8,item);

    m_sortHelper.Init(CSL_SORT_DSC,SORT_PLAYER);
    ToggleSortArrow();
}

void CslListCtrlServer::ListAdjustSize(const wxSize& size,bool init)
{
    if (!g_cslSettings->autoFitColumns && !init)
        return;

    wxInt32 w=size.x-8;

    SetColumnWidth(0,(wxInt32)(w*g_cslSettings->colServerS1));
    SetColumnWidth(1,(wxInt32)(w*g_cslSettings->colServerS2));
    SetColumnWidth(2,(wxInt32)(w*g_cslSettings->colServerS3));
    SetColumnWidth(3,(wxInt32)(w*g_cslSettings->colServerS4));
    SetColumnWidth(4,(wxInt32)(w*g_cslSettings->colServerS5));
    SetColumnWidth(5,(wxInt32)(w*g_cslSettings->colServerS6));
    SetColumnWidth(6,(wxInt32)(w*g_cslSettings->colServerS7));
    SetColumnWidth(7,(wxInt32)(w*g_cslSettings->colServerS8));
    SetColumnWidth(8,(wxInt32)(w*g_cslSettings->colServerS9));
}

void CslListCtrlServer::ListCreateGameBitmaps()
{
    m_imageList.Create(16,16,true);

    m_imageList.Add(wxBitmap(green_list_16_xpm));
    m_imageList.Add(wxBitmap(yellow_list_16_xpm));
    m_imageList.Add(wxBitmap(red_list_16_xpm));
    m_imageList.Add(wxBitmap(grey_list_16_xpm));
    m_imageList.Add(wxBitmap(green_ext_list_16_xpm));
    m_imageList.Add(wxBitmap(yellow_ext_list_16_xpm));
    m_imageList.Add(wxBitmap(red_ext_list_16_xpm));
    m_imageList.Add(wxBitmap(sortasc_16_xpm));
    m_imageList.Add(wxBitmap(sortdsc_16_xpm));
    m_imageList.Add(wxBitmap(sortasclight_16_xpm));
    m_imageList.Add(wxBitmap(sortdsclight_16_xpm));

    //now create the icons for favourites list
    if (m_id==CSL_LIST_FAVOURITE)
    {
        wxBitmap bmpExt(ext_green_8_xpm);

        //magic colour and brush for transparency
        wxColour magicColour(255,0,255);
        wxBrush magicBrush(magicColour);

        wxMemoryDC dc;

        vector<CslGame*>& games=m_engine->GetGames();
        loopv(games)
        {
            const char **icon=games[i]->GetIcon(16);

            wxBitmap bmpGame=icon ? wxBitmap(icon):wxBitmap(16,16);
            m_imageList.Add(bmpGame);

            wxBitmap bmp(16,16);
            dc.SelectObject(bmp);
            //draw transparent background
            dc.SetBrush(magicBrush);
            dc.SetPen(*wxTRANSPARENT_PEN);
            dc.DrawRectangle(0,0,16,16);
            //draw the bitmaps
            dc.DrawBitmap(bmpGame,0,0,true);
            dc.DrawBitmap(bmpExt,8,8,true);
            //unref the bitmap and set the mask
            dc.SelectObject(wxNullBitmap);
            bmp.SetMask(new wxMask(bmp,magicColour));

            m_imageList.Add(bmp);
        }
    }

    SetImageList(&m_imageList,wxIMAGE_LIST_SMALL);
}

void CslListCtrlServer::Highlight(wxInt32 type,bool high,CslServerInfo *info,wxListItem *listitem)
{
    wxListItem item;
    wxUint32 i;

    if (info)
    {
        if (!listitem)
        {
            if (ListFindItem(info,item)==wxNOT_FOUND)
                return;

            listitem=&item;
        }

        CslListServerData *server=(CslListServerData*)GetItemData(*listitem);
        high=server->SetHighlight(type,high);

        SetItemBackgroundColour(*listitem,high ? g_cslSettings->colServerHigh :
                                info->IsLocked() ? g_cslSettings->colServerPlay : m_stdColourListItem);
        return;
    }

    for (i=0;i<m_servers.GetCount();i++)
    {
        info=m_servers.Item(i)->Info;

        if (ListFindItem(info,item)==wxNOT_FOUND)
            return;

        high=m_servers.Item(i)->SetHighlight(type,high);

        SetItemBackgroundColour(item,high ? g_cslSettings->colServerHigh :
                                info->IsLocked() ? g_cslSettings->colServerPlay : m_stdColourListItem);
    }
}

bool CslListCtrlServer::PingOk(CslServerInfo *info)
{
    return info->Ping>-1 && (wxInt32)(info->PingSend-info->PingResp) <
           g_cslSettings->updateInterval*2;
}

wxInt32 CslListCtrlServer::ListFindItem(CslServerInfo *info,wxListItem& item)
{
    wxInt32 i;

    for (i=0;i<GetItemCount();i++)
    {
        item.SetId(i);
        CslServerInfo *infoCmp=((CslListServerData*)GetItemData(item))->Info;
        if (info==infoCmp)
            return i;
    }

    return wxNOT_FOUND;
}

bool CslListCtrlServer::ListSearchItemMatches(CslServerInfo *info)
{
    wxString s;

    s=info->Host.Lower();
    if (s.Find(m_searchString)!=wxNOT_FOUND)
        return true;
    else
    {
        s=info->Description.Lower();
        if (s.Find(m_searchString)!=wxNOT_FOUND)
            return true;
        else
        {
            s=wxString(info->Version.Lower());
            if (s.Find(m_searchString)!=wxNOT_FOUND)
                return true;
            else
            {
                s=info->GameMode.Lower();
                if (s.Find(m_searchString)!=wxNOT_FOUND)
                    return true;
                else
                {
                    s=info->Map.Lower();
                    if (s.Find(m_searchString)!=wxNOT_FOUND)
                        return true;
                }
            }
        }
    }

    return false;
}

bool CslListCtrlServer::ListFilterItemMatches(CslServerInfo *info)
{
    if (m_filterVersion>-1 && info->Protocol!=m_filterVersion)
        return true;
    else if (*m_filterFlags&CSL_FILTER_OFFLINE && !PingOk(info))
        return true;
    else if (*m_filterFlags&CSL_FILTER_FULL && info->PlayersMax>0 &&
             info->Players==info->PlayersMax)
        return true;
    else if (*m_filterFlags&CSL_FILTER_EMPTY && info->Players==0)
        return true;
    else if (*m_filterFlags&CSL_FILTER_NONEMPTY && info->Players>0)
        return true;
    else if (*m_filterFlags&CSL_FILTER_MM2 && info->MM==2)
        return true;
    else if (*m_filterFlags&CSL_FILTER_MM3 && info->MM==3)
        return true;

    return false;
}

bool CslListCtrlServer::ListUpdateServer(CslServerInfo *info)
{
    if (!info)
    {
        wxASSERT_MSG(info,wxT("invalid info"));
        return false;
    }

    wxInt32 i,j;
    wxString s;
    wxListItem item;
    bool found=false;
    CslListServerData *infoCmp=NULL;

    i=m_servers.GetCount();
    for (j=0;j<i;j++)
    {
        if (m_servers.Item(j)->Info==info)
        {
            infoCmp=m_servers.Item(j);
            break;
        }
    }

    j=ListFindItem(info,item);
    if (j==wxNOT_FOUND)
        i=GetItemCount();
    else
        i=j;

    item.SetId(i);
    item.SetMask(wxLIST_MASK_TEXT|wxLIST_MASK_DATA);

    // filter
    found=m_filterFlags||m_filterVersion>-1 ? ListFilterItemMatches(info) : false;
    if (found && j!=wxNOT_FOUND)
    {
        ListDeleteItem(&item);
        wxInt32 pos=m_selected.Index(infoCmp);
        if (pos!=wxNOT_FOUND)
            m_selected.RemoveAt(pos);
    }

    if (j==wxNOT_FOUND)
    {
        if (!infoCmp)
        {
            infoCmp=new CslListServerData(info);
            m_servers.Add(infoCmp);
        }
        else
            infoCmp->Reset();

        if (found)
        {
            if (!m_searchString.IsEmpty())
                return ListSearchItemMatches(info);
            return true;
        }

        item.SetData((long)infoCmp);
        InsertItem(item);

        SetItem(i,0,info->Host);
    }
    else
    {
        if (found)
        {
            if (!m_searchString.IsEmpty())
                return ListSearchItemMatches(info);
            return true;
        }
        infoCmp=(CslListServerData*)GetItemData(item);
    }

    if (PingOk(info) || info->PingResp)
    {
        if (infoCmp->Description!=info->Description)
            SetItem(i,1,info->Description);

        if (infoCmp->Protocol!=info->Protocol)
            SetItem(i,2,info->Version);

        if (infoCmp->Ping!=info->Ping)
            SetItem(i,3,wxString::Format(wxT("%d"),info->Ping));

        if (infoCmp->GameMode!=info->GameMode)
            SetItem(i,4,info->GameMode);

        if (infoCmp->Map!=info->Map)
            SetItem(i,5,info->Map);

        if (infoCmp->TimeRemain!=info->TimeRemain)
        {
            if (info->TimeRemain<0)
                s=_("no limit");
            else
                s=s.Format(wxT("%d"),info->TimeRemain);
            SetItem(i,6,s);
        }

        if (infoCmp->Players!=info->Players ||
            infoCmp->PlayersMax!=info->PlayersMax)
        {
            if (info->PlayersMax<=0)
                s=s.Format(wxT("%d"),info->Players);
            else
                s=s.Format(wxT("%d/%d"),info->Players,info->PlayersMax);
            SetItem(i,7,s);
        }

        if (infoCmp->MM!=info->MM)
        {
            if (info->MM>=0)
            {
                s=s.Format(wxT("%d"),info->MM);
                if (info->MM==CslGame::MM_OPEN)
                    s+=wxT(" (O)");
                else if (info->MM==CslGame::MM_VETO)
                    s+=wxT(" (V)");
                else if (info->MM==CslGame::MM_LOCKED)
                    s+=wxT(" (L)");
                else if (info->MM==CslGame::MM_PRIVATE)
                    s+=wxT(" (P)");
                SetItem(i,8,s);
            }
        }
    }
    else
    {
        if (info->Description.IsEmpty() && !info->DescriptionOld.IsEmpty())
            SetItem(i,1,info->DescriptionOld);
    }

    wxColour colour=m_stdColourListText;
    if (!PingOk(info))
        colour=g_cslSettings->colServerOff;
    else if (info->PlayersMax>0 && info->Players==info->PlayersMax)
        colour=g_cslSettings->colServerFull;
    else if (info->MM==CslGame::MM_VETO)
        colour=g_cslSettings->colServerMM1;
    else if (info->MM==CslGame::MM_LOCKED)
        colour=g_cslSettings->colServerMM2;
    else if (info->MM==CslGame::MM_PRIVATE)
        colour=g_cslSettings->colServerMM3;
    else if (info->Players==0)
        colour=g_cslSettings->colServerEmpty;
    SetItemTextColour(i,colour);

    // search
    found=m_searchString.IsEmpty() ? false : ListSearchItemMatches(info);
    Highlight(CSL_HIGHLIGHT_SEARCH_SERVER,found,info,&item);

    item.SetMask(wxLIST_MASK_TEXT|wxLIST_MASK_IMAGE|wxLIST_MASK_DATA);
    if (m_id==CSL_LIST_MASTER)
    {
        if (!PingOk(info))
            i=CSL_LIST_IMG_GREY;
        else if (info->Ping>(wxInt32)g_cslSettings->pingbad)
            i=info->ExtInfoStatus!=CSL_EXT_STATUS_FALSE ? CSL_LIST_IMG_RED_EXT : CSL_LIST_IMG_RED;
        else if (info->Ping>(wxInt32)g_cslSettings->pinggood)
            i=info->ExtInfoStatus!=CSL_EXT_STATUS_FALSE ? CSL_LIST_IMG_YELLOW_EXT : CSL_LIST_IMG_YELLOW;
        else
            i=info->ExtInfoStatus!=CSL_EXT_STATUS_FALSE ? CSL_LIST_IMG_GREEN_EXT : CSL_LIST_IMG_GREEN;

        if (infoCmp->ImgId!=i)
        {
            SetItemImage(item,i);
            infoCmp->ImgId=i;
        }
    }
    else
    {
        i=CSL_LIST_IMG_GAMES_START+info->GetGame().GetId()*2-
          (info->ExtInfoStatus==CSL_EXT_STATUS_OK ? 1:2);
        if (infoCmp->ImgId!=i)
        {
            SetItemImage(item,i);
            infoCmp->ImgId=i;
        }
    }

    *infoCmp=*info;

    return found;
}

void CslListCtrlServer::ListDeleteItem(wxListItem *item)
{
    m_dontRemoveOnDeselect=true;
    DeleteItem(*item);
    m_dontRemoveOnDeselect=false;
}

void CslListCtrlServer::RemoveServer(CslListServerData *server,CslServerInfo *info,wxInt32 selId)
{
    wxInt32 i,l,c;
    wxListItem item;

    if (ListFindItem(info,item)!=wxNOT_FOUND)
        ListDeleteItem(&item);

    if (!server)
    {
        l=m_servers.GetCount();
        for (i=0;i<l;i++)
            if (m_servers.Item(i)->Info==info)
            {
                server=m_servers.Item(i);
                break;
            }
    }

    if (!server)
        return;

    if (selId<0)
    {
        if ((c=m_selected.Index(server))!=wxNOT_FOUND)
            m_selected.RemoveAt(c);
    }
    else
        m_selected.RemoveAt(selId);

    m_servers.Remove(server);
    delete server;
}

wxUint32 CslListCtrlServer::ListUpdate(vector<CslServerInfo*>& servers)
{
    bool sort=false;
    wxUint32 c=0;

    loopv(servers)
    {
        CslServerInfo *info=servers[i];
        switch (m_id)
        {
            case CSL_LIST_MASTER:
                if (m_masterSelected && !info->IsDefault() && !info->IsUnused())
                    continue;
                break;
            case CSL_LIST_FAVOURITE:
                if (!info->IsFavourite())
                    break;
        }
        sort=true;
        if (ListUpdateServer(info))
            c++;
    }

    if (!sort || !g_cslSettings->autoSortColumns)
        return c;

//WORKAROUND create an event and sending it elimates flicker
//SortItems(ListSortCompareFunc,(long)&m_sortMast);
//ListSort(-1);
    wxListEvent event(wxEVT_COMMAND_LIST_COL_CLICK);
    event.m_col=-1;
    wxPostEvent(this,event);

    return c;
}

void CslListCtrlServer::ListClear()
{
    DeleteAllItems();
    m_selected.Clear();
    WX_CLEAR_ARRAY(m_servers);
    m_filterVersion=-1;
}

wxUint32 CslListCtrlServer::ListSearch(const wxString& search)
{
    wxUint32 i,l,c=0;

    m_searchString=search.Lower();

    Freeze();

    l=m_servers.GetCount();
    for (i=0;i<l;i++)
    {
        CslServerInfo *info=m_servers.Item(i)->Info;
        if (ListUpdateServer(info))
            c++;
    }

    Thaw();

    return c;
}

wxUint32 CslListCtrlServer::ListFilter()
{
    wxUint32 i,l,c=0;

    wxWindowUpdateLocker locker(this);

    l=m_servers.GetCount();
    for (i=0;i<l;i++)
    {
        CslServerInfo *info=m_servers.Item(i)->Info;
        if (ListUpdateServer(info))
            c++;
    }

    ListSort();

    return c;
}

void CslListCtrlServer::ConnectToServer(CslServerInfo *info,wxUint32 mode)
{
    if (CslConnectionState::IsPlaying())
    {
        wxMessageBox(_("You are currently playing, so quit the game and try again."),
                     _("Error"),wxOK|wxICON_ERROR,this);
        return;
    }

    if (info->Players>0 && info->Players==info->PlayersMax)
    {
        wxInt32 time=g_cslSettings->waitServerFull;

        CslDlgConnectWait *dlg=new CslDlgConnectWait(this,&time);
        if (dlg->ShowModal()==wxID_OK)
        {
            m_timerCount=0;
            CslConnectionState::CreateWaitingState(info,this,time);
        }
        return;
    }

    wxString error;
    wxString cmd=info->GetGame().GameStart(info,mode,&error);

    if (cmd.IsEmpty())
    {
        if (!error.IsEmpty())
            wxMessageBox(error,_("Error"),wxICON_ERROR,this);
        return;
    }

    CslDlgOutput::Reset(info->GetBestDescription());

    info->Lock();
    ::wxSetWorkingDirectory(info->GetGame().GetClientSettings().GamePath);

    CslProcess *process=new CslProcess(this,info,cmd);
    if (!(::wxExecute(cmd,wxEXEC_ASYNC,process)>0))
    {
        wxMessageBox(_("Failed to start: ")+cmd,
                     _("Error"),wxICON_ERROR,this);
        info->Lock(false);
        return;
    }

    m_timerCount=0;
    CslConnectionState::CreatePlayingState(info,this);

    info->ConnectedTimes++;
    info->PlayLast=wxDateTime::Now().GetTicks();
    m_listInfo->UpdateInfo(info);
    ListUpdateServer(info);
}

void CslListCtrlServer::ToggleSortArrow()
{
    wxListItem item;
    wxInt32 img=-1;

    if (g_cslSettings->autoSortColumns)
    {
        if (m_sortHelper.m_sortMode==CSL_SORT_ASC)
            img=CSL_LIST_IMG_SORT_ASC;
        else
            img=CSL_LIST_IMG_SORT_DSC;
    }
    item.SetImage(img);
    SetColumn(m_sortHelper.m_sortType,item);
}

void CslListCtrlServer::ListSort(wxInt32 column)
{
    wxListItem item;
    wxInt32 img;

    if (column==-1)
        column=m_sortHelper.m_sortType;
    else
    {
        item.SetMask(wxLIST_MASK_IMAGE);
        GetColumn(column,item);

        if (item.GetImage()==-1 ||
            item.GetImage()==CSL_LIST_IMG_SORT_DSC ||
            item.GetImage()==CSL_LIST_IMG_SORT_DSC_LIGHT)
        {
            g_cslSettings->autoSortColumns ? img=CSL_LIST_IMG_SORT_ASC : img=CSL_LIST_IMG_SORT_ASC_LIGHT;
            m_sortHelper.m_sortMode=CSL_SORT_ASC;
        }
        else
        {
            g_cslSettings->autoSortColumns ? img=CSL_LIST_IMG_SORT_DSC : img=CSL_LIST_IMG_SORT_DSC_LIGHT;
            m_sortHelper.m_sortMode=CSL_SORT_DSC;
        }

        item.Clear();
        item.SetImage(-1);
        SetColumn(m_sortHelper.m_sortType,item);

        item.SetImage(img);
        SetColumn(column,item);

        m_sortHelper.m_sortType=column;
    }

    if (!GetItemCount())
        return;

#ifdef CSL_USE_WX_LIST_DESELECT_WORKAROUND
    CslServerInfo **selected=new CslServerInfo*[GetItemCount()];
    wxInt32 i,j;

    /*for (i=0;i<GetItemCount();i++)
        if (GetItemState(i,wxLIST_STATE_SELECTED) & wxLIST_STATE_SELECTED)
            selected[c++]=(CslServerInfo*)((CslListServerData*)GetItemData(i))->Info;*/
    for (i=0;i<(wxInt32)m_selected.GetCount();i++)
        selected[i]=m_selected.Item(i)->Info;
#endif

    SortItems(ListSortCompareFunc,(long)&m_sortHelper);

#ifdef CSL_USE_WX_LIST_DESELECT_WORKAROUND
    while (i>=0)
    {
        if ((j=ListFindItem(selected[i--],item))==wxNOT_FOUND)
            continue;
        SetItemState(j,wxLIST_STATE_SELECTED,wxLIST_STATE_SELECTED);
    }

    delete[] selected;
#endif
}

int wxCALLBACK CslListCtrlServer::ListSortCompareFunc(long item1,long item2,long data)
{
    CslListServerData *info1=(CslListServerData*)item1;
    CslListServerData *info2=(CslListServerData*)item2;

    bool ping1Ok=PingOk(info1->Info);
    bool ping2Ok=PingOk(info2->Info);

    wxInt32 type;
    wxInt32 sortMode=((CslListSortHelper*)data)->m_sortMode;
    wxInt32 sortType=((CslListSortHelper*)data)->m_sortType;
    wxInt32 vi1=0,vi2=0;
    wxUint32 vui1=0,vui2=0;
    wxString vs1=wxEmptyString,vs2=wxEmptyString;

    if (sortType!=SORT_HOST)
    {
        if (!ping1Ok && !ping2Ok)
            return 0;
        if (!ping1Ok)
            return 1;
        if (!ping2Ok)
            return -1;
    }

    switch (sortType)
    {
        case SORT_HOST:
        {
            type=CSL_LIST_SORT_STRING;
            vs1=info1->Host;
            vs2=info2->Host;
            bool isip1=IsIP(vs1);
            bool isip2=IsIP(vs2);
            if (isip1&&!isip2)
                return sortMode==CSL_SORT_ASC ? -1 : 1;
            else if (!isip1&&isip2)
                return sortMode==CSL_SORT_ASC ? 1 : -1;
            else if (isip1&&isip2)
            {
                type=CSL_LIST_SORT_UINT;
                IP2Int(vs1,&vui1);
                IP2Int(vs2,&vui2);
            }
            break;
        }

        case SORT_DESCRIPTION:
            type=CSL_LIST_SORT_STRING;
            vs1=info1->Description;
            vs2=info2->Description;
            break;

        case SORT_PING:
            type=CSL_LIST_SORT_UINT;
            vui1=info1->Ping;
            vui2=info2->Ping;
            break;

        case SORT_VER:
            type=CSL_LIST_SORT_INT;
            vi1=info1->Protocol;
            vi2=info2->Protocol;
            break;

        case SORT_MODE:
            type=CSL_LIST_SORT_STRING;
            vs1=info1->GameMode;
            vs2=info2->GameMode;
            break;

        case SORT_MAP:
            type=CSL_LIST_SORT_STRING;
            vs1=info1->Map;
            vs2=info2->Map;
            break;

        case SORT_TIME:
            type=CSL_LIST_SORT_UINT;
            vui1=info1->TimeRemain;
            vui2=info2->TimeRemain;
            break;

        case SORT_PLAYER:
            type=CSL_LIST_SORT_INT;
            vi1=info1->Players;
            vi2=info2->Players;
            break;

        case SORT_MM:
            type=CSL_LIST_SORT_INT;
            vi1=info1->MM;
            vi2=info2->MM;
            break;

        default:
            return 0;
    }

    if (type==CSL_LIST_SORT_INT)
    {
        if (vi1==vi2)
            return 0;
        if (vi1<vi2)
            return sortMode==CSL_SORT_ASC ? -1 : 1;
        else
            return sortMode==CSL_SORT_ASC ? 1 : -1;
    }
    else if (type==CSL_LIST_SORT_UINT)
    {
        if (vui1==vui2)
            return 0;
        if (vui1<vui2)
            return sortMode==CSL_SORT_ASC ? -1 : 1;
        else
            return sortMode==CSL_SORT_ASC ? 1 : -1;
    }
    else if (type==CSL_LIST_SORT_STRING)
    {
        if (vs1==vs2)
            return 0;
        if (vs1.CmpNoCase(vs2)<0)
            return sortMode==CSL_SORT_ASC ? -1 : 1;
        else
            return sortMode==CSL_SORT_ASC ? 1 : -1;
    }

    return 0;
}
