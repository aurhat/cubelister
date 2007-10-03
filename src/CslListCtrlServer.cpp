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
#include <wx/file.h>
#include <wx/txtstrm.h>
#include "CslDlgAddServer.h"
#include "CslDlgConnectWait.h"
#include "CslDlgOutput.h"
#include "CslListCtrlServer.h"
#include "CslStatusBar.h"
#include "CslArt.h"
#include "CslMenu.h"
#include "CslSettings.h"

#ifndef _MSC_VER
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
#include "img/sb_16.xpm"
#include "img/ac_16.xpm"
#include "img/bf_16.xpm"
#include "img/cb_16.xpm"
#include "img/sb_ext_16.xpm"
#include "img/ac_ext_16.xpm"
#include "img/bf_ext_16.xpm"
#endif

enum
{
    SORT_HOST = 0, SORT_DESC, SORT_VER,
    SORT_PING, SORT_MODE,
    SORT_MAPS, SORT_TIME, SORT_PLAY, SORT_MM, SORT_UNKNOWN
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
wxString           CslConnectionState::m_activeCfg=wxEmptyString;


class CslProcess : public wxProcess
{
    public:
        CslProcess(CslListCtrlServer *parent,CslServerInfo *info,wxString cmd) :
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
            if (time>g_cslSettings->m_minPlaytime)
                m_info->SetLastPlayTime(time);

            // Cube returns with 1 - weird
            if (code!=0 && m_info->m_type!=CSL_GAME_CB)
                wxMessageBox(wxString::Format(_("%s returned with code: %d"),m_cmd.c_str(),code),
                             _("Error"),wxICON_ERROR,m_parent);

            ProcessInputStream();

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
                buf[last]=0;
                if (m_self->m_info->m_type==CSL_GAME_CB)
                    StripColours(buf,&last,1);
                CslDlgOutput::AddOutput(buf,last);
                //LOG_DEBUG("%s", buf);
            }
        }

    protected:
        static CslProcess *m_self;
        CslListCtrlServer *m_parent;
        CslServerInfo *m_info;
        wxString m_cmd;
        bool m_clear;
        wxStopWatch m_watch;
        wxInputStream *stream;
};

CslProcess* CslProcess::m_self=NULL;


CslListCtrlServer::CslListCtrlServer(wxWindow* parent,wxWindowID id,const wxPoint& pos,
                                     const wxSize& size,long style,
                                     const wxValidator& validator, const wxString& name) :
        wxListCtrl(parent,id,pos,size,style,validator,name),
        m_id(id),m_engine(NULL),m_masterSelected(false),
        m_listInfo(NULL),m_listMaster(NULL),m_listFavourites(NULL),m_extendedDlg(NULL),
        m_dontUpdateInfo(false),m_dontRemoveOnDeselect(false),m_filterFlags(0)
{

#define CSL_LIST_IMG_GREEN            0
#define CSL_LIST_IMG_YELLOW           1
#define CSL_LIST_IMG_RED              2
#define CSL_LIST_IMG_GREY             3
#define CSL_LIST_IMG_GREEN_EXT        4
#define CSL_LIST_IMG_YELLOW_EXT       5
#define CSL_LIST_IMG_RED_EXT          6
#define CSL_LIST_IMG_SORT_ASC         7
#define CSL_LIST_IMG_SORT_DSC         8
#define CSL_LIST_IMG_SORT_ASC_LIGHT   9
#define CSL_LIST_IMG_SORT_DSC_LIGHT  10
#define CSL_LIST_IMG_SB              11
#define CSL_LIST_IMG_AC              12
#define CSL_LIST_IMG_BF              13
#define CSL_LIST_IMG_CB              14
#define CSL_LIST_IMG_SB_EXT          15
#define CSL_LIST_IMG_AC_EXT          16
#define CSL_LIST_IMG_BF_EXT          17

    m_imageList.Create(16,16,true);

#ifndef _MSC_VER
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
    m_imageList.Add(wxBitmap(sb_16_xpm));
    m_imageList.Add(wxBitmap(ac_16_xpm));
    m_imageList.Add(wxBitmap(bf_16_xpm));
    m_imageList.Add(wxBitmap(cb_16_xpm));
    m_imageList.Add(wxBitmap(sb_ext_16_xpm));
    m_imageList.Add(wxBitmap(ac_ext_16_xpm));
    m_imageList.Add(wxBitmap(bf_ext_16_xpm));
#else
    m_imageList.Add(wxIcon(wxT("ICON_LIST_GREEN"),wxBITMAP_TYPE_ICO_RESOURCE));
    m_imageList.Add(wxIcon(wxT("ICON_LIST_YELLOW"),wxBITMAP_TYPE_ICO_RESOURCE));
    m_imageList.Add(wxIcon(wxT("ICON_LIST_RED"),wxBITMAP_TYPE_ICO_RESOURCE));
    m_imageList.Add(wxIcon(wxT("ICON_LIST_GREY"),wxBITMAP_TYPE_ICO_RESOURCE));
    m_imageList.Add(wxIcon(wxT("ICON_LIST_GREEN_EXT"),wxBITMAP_TYPE_ICO_RESOURCE));
    m_imageList.Add(wxIcon(wxT("ICON_LIST_YELLOW_EXT"),wxBITMAP_TYPE_ICO_RESOURCE));
    m_imageList.Add(wxIcon(wxT("ICON_LIST_RED_EXT"),wxBITMAP_TYPE_ICO_RESOURCE));
    m_imageList.Add(wxIcon(wxT("ICON_LIST_ASC"),wxBITMAP_TYPE_ICO_RESOURCE));
    m_imageList.Add(wxIcon(wxT("ICON_LIST_DSC"),wxBITMAP_TYPE_ICO_RESOURCE));
    m_imageList.Add(wxIcon(wxT("ICON_LIST_ASC_LIGHT"),wxBITMAP_TYPE_ICO_RESOURCE));
    m_imageList.Add(wxIcon(wxT("ICON_LIST_DSC_LIGHT"),wxBITMAP_TYPE_ICO_RESOURCE));
    m_imageList.Add(wxIcon(wxT("ICON_LIST_SB_16"),wxBITMAP_TYPE_ICO_RESOURCE));
    m_imageList.Add(wxIcon(wxT("ICON_LIST_AC_16"),wxBITMAP_TYPE_ICO_RESOURCE));
    m_imageList.Add(wxIcon(wxT("ICON_LIST_BF_16"),wxBITMAP_TYPE_ICO_RESOURCE));
    m_imageList.Add(wxIcon(wxT("ICON_LIST_CB_16"),wxBITMAP_TYPE_ICO_RESOURCE));
    m_imageList.Add(wxIcon(wxT("ICON_LIST_SB_EXT_16"),wxBITMAP_TYPE_ICO_RESOURCE));
    m_imageList.Add(wxIcon(wxT("ICON_LIST_AC_EXT_16"),wxBITMAP_TYPE_ICO_RESOURCE));
    m_imageList.Add(wxIcon(wxT("ICON_LIST_BF_EXT_16"),wxBITMAP_TYPE_ICO_RESOURCE));
#endif

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
        /*
        if (event.ShiftDown())
            ListDeleteServers();
        else
            ListRemoveServers();
        */
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
    CslListServer *server=(CslListServer*)GetItemData(item);
    CslServerInfo *info=server->GetPtr();
    ConnectToServer(info);
}

void CslListCtrlServer::OnItemSelected(wxListEvent& event)
{
    event.Skip();

    wxListItem item;
    wxInt32 i=event.GetIndex();

    item.SetId(i);
    GetItem(item);

    CslListServer *server=(CslListServer*)GetItemData(item);
    m_selected.Add(server);

    if (m_dontUpdateInfo)
        return;

    m_listInfo->UpdateInfo(m_selected.Last()->GetPtr());
}

void CslListCtrlServer::OnItemDeselected(wxListEvent& event)
{
    event.Skip();

    if (m_dontRemoveOnDeselect)
        return;

    wxInt32 i=event.GetIndex();

    wxListItem item;
    item.SetId(i);
    GetItem(item);

    CslListServer *server=(CslListServer*)GetItemData(item);
    if (m_selected.Index(server)!=wxNOT_FOUND)
        m_selected.Remove(server);
}

void CslListCtrlServer::OnContextMenu(wxContextMenuEvent& event)
{
    wxInt32 c;
    wxMenu menu;
    wxPoint point=event.GetPosition();

//from keyboard
    if (point.x==-1 && point.y==-1)
    {
        //TODO handle mouse pos if pointer is outside window
        //wxSize size=GetSize();
        //point.x=size.x/2;
        //point.y=size.y/2;
        point=wxGetMousePosition();
    }

    c=m_selected.GetCount();
    if (c==1)
    {
        CslMenu::AddItemToMenu(&menu,MENU_SERVER_CONNECT,MENU_SERVER_CONN_STR,wxART_CONNECT);
#ifdef CSL_EXT_SERVER_INFO
        CslMenu::AddItemToMenu(&menu,MENU_SERVER_EXTENDED,_("Extended information"),wxART_ABOUT);
#endif
        menu.AppendSeparator();

        if (CslConnectionState::IsPlaying())
            menu.Enable(MENU_SERVER_CONNECT,false);
#ifdef CSL_EXT_SERVER_INFO
        CslServerInfo *info=m_selected.Item(0);
        if (!info->m_extended || !PingOk(info))
            menu.Enable(MENU_SERVER_EXTENDED,false);
#endif
    }

    switch (m_id)
    {
        case CSL_LIST_MASTER:
            if (!c)
                break;
            CslMenu::AddItemToMenu(&menu,MENU_SERVER_ADD,MENU_SERVER_MAS_ADD_STR,wxART_ADD_BOOKMARK);
            if (m_masterSelected)
                CslMenu::AddItemToMenu(&menu,MENU_SERVER_REM,MENU_SERVER_MAS_REM_STR,wxART_DEL_BOOKMARK);
            menu.AppendSeparator();
            break;

        case CSL_LIST_FAVOURITE:
            CslMenu::AddItemToMenu(&menu,MENU_SERVER_ADD,MENU_SERVER_FAV_ADD_STR,wxART_ADD_BOOKMARK);
            if (!c)
                break;
            CslMenu::AddItemToMenu(&menu,MENU_SERVER_REM,MENU_SERVER_FAV_REM_STR,wxART_DEL_BOOKMARK);
            menu.AppendSeparator();
            break;
        default:
            return;
    }

    if (c)
    {
        CslMenu::AddItemToMenu(&menu,MENU_SERVER_COPYTEXT,MENU_SERVER_COPY_STR,wxART_COPY);
        menu.AppendSeparator();
        CslMenu::AddItemToMenu(&menu,MENU_SERVER_DEL,
                               c>1 ? MENU_SERVER_DELM_STR : MENU_SERVER_DEL_STR,wxART_DELETE);
    }

    point=ScreenToClient(point);
    PopupMenu(&menu,point);
}

void CslListCtrlServer::ListRemoveServers()
{
    wxInt32 c,i;
    CslServerInfo *info;
    wxListItem item;

    for (i=m_selected.GetCount()-1;i>=0;i--)
    {
        CslListServer *ls=m_selected.Item(i);
        info=ls->GetPtr();

        if ((c=ListFindItem(info))==wxNOT_FOUND)
            continue;

        item.SetId(c);
        ListDeleteItem(&item);
        m_selected.RemoveAt(i);

        m_servers.Remove(ls);

        switch (m_id)
        {
            case CSL_LIST_MASTER:
                m_engine->RemoveServerFromMaster(info);
                break;
            case CSL_LIST_FAVOURITE:
                m_engine->RemoveServerFromFavourites(info);
                break;
        }
    }
}

#define CSL_DELETE_YESNOCANCEL_STR _("\nChoose Yes to keep these servers, " \
                                     "No to delete them or\nCancel the operation.")

void CslListCtrlServer::ListDeleteServers()
{
    wxInt32 c,i;
    wxUint32 l;
    wxString msg,s;
    CslServerInfo *info;
    wxInt32 skipFav=wxCANCEL,skipStats=wxCANCEL;

    for (i=m_selected.GetCount()-1;i>=0;i--)
    {
        msg.Empty();
        CslListServer *ls=m_selected.Item(i);

        if (skipFav==wxCANCEL && ls->IsFavourite())
        {
            msg=_("You are about to delete servers which are also favourites!\n");
            msg+=CSL_DELETE_YESNOCANCEL_STR;
            skipFav=wxMessageBox(msg,_("Warning"),wxYES_NO|wxCANCEL|wxICON_WARNING,this);
            if (skipFav==wxCANCEL)
                return;
            if (skipFav!=wxNO)
                continue;
        }
        if (skipStats==wxCANCEL && ls->HasStats())
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
        CslListServer *ls=m_selected.Item(i);
        info=ls->GetPtr();

        if ((c=ListFindItem(info))==wxNOT_FOUND)
            continue;

        if (ls->IsLocked())
        {
            s=info->GetBestDescription();
            l=s.Len();
            msg=wxString::Format(_("Server \"%s\" is currently locked,\nso deletion is not possible!"),
                                 A2U(StripColours(U2A(s),&l,2)).c_str());
            wxMessageBox(msg,_("Error"),wxICON_ERROR,this);
            continue;
        }
#ifdef CSL_EXT_SERVER_INFO
        if (m_extendedDlg->IsShown() && m_extendedDlg->GetInfo()==info)
        {
            s=info->GetBestDescription();
            l=s.Len();
            msg=wxString::Format(_("Player statistics are shown for Server \"%s\",\nso deletion is not possible!"),
                                 A2U(StripColours(U2A(s),&l,2)).c_str());
            wxMessageBox(msg,_("Error"),wxICON_ERROR,this);
            continue;
        }
#endif

        if (ls->IsFavourite() && skipFav!=wxNO)
            continue;

        if (ls->HasStats() && skipStats!=wxNO)
            continue;

        RemoveServer(ls,info,i);

        if (m_id==CSL_LIST_MASTER && ls->IsFavourite())
            m_listFavourites->RemoveServer(NULL,info,-1);
        else if (m_id==CSL_LIST_FAVOURITE)
            m_listMaster->RemoveServer(NULL,info,-1);

        m_engine->DeleteServer(info);
    }
}
#undef CSL_DELETE_YESNOCANCEL_STR

void CslListCtrlServer::OnMenu(wxCommandEvent& event)
{
    CslServerInfo *info;
    wxListItem item;
    wxUint32 i=0;
    wxInt32 c=0;
    wxString s;

    wxInt32 id=event.GetId();

    switch (id)
    {
        case MENU_SERVER_CONNECT:
            ConnectToServer(m_selected.Item(0)->GetPtr());
            break;

        case MENU_SERVER_EXTENDED:
        {
            m_extendedDlg->DoShow(m_selected.Item(0)->GetPtr());
            break;
        }

        case MENU_SERVER_ADD:
            switch (m_id)
            {
                case CSL_LIST_MASTER:
                {
                    bool sort=false;
                    for (i=0;i<m_selected.GetCount();i++)
                    {
                        info=m_selected.Item(i)->GetPtr();

                        if ((c=ListFindItem(info))==wxNOT_FOUND)
                            continue;
                        if (!m_engine->AddServerToFavourites(info))
                            continue;
                        sort=true;
                        m_listFavourites->ListUpdateServer(info);
                        /*if (!info->IsDefault())
                        {
                            item.SetId(c);
                            DeleteItem(item);
                        }*/
                    }
                    if (sort)
                        m_listFavourites->ListSort(-1);
                    break;
                }
                case CSL_LIST_FAVOURITE:
                {
                    CslServerInfo *ret;
                    info=new CslServerInfo();
                    CslDlgAddServer *dlg=new CslDlgAddServer((wxWindow*)this,info);
                    if (dlg->ShowModal()==wxID_OK)
                    {
                        ret=m_engine->AddServer(info,-1);
                        if (ret)
                        {
                            if (ret!=info)
                            {
                                delete info;
                                info=ret;
                            }
                            ListUpdateServer(info);
                            ListSort(-1);
                            break;
                        }
                    }
                    delete info;
                    break;
                }
            }
            break;

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
                info=m_selected.Item(i)->GetPtr();

                if ((c=ListFindItem(info))==wxNOT_FOUND)
                    continue;

                item.SetId(c);

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

        default:
            break;;
    }
}

void CslListCtrlServer::OnEndProcess(wxCommandEvent& event)
{
    CslServerInfo *info=(CslServerInfo*)event.GetClientData();

    CslGame::ConnectCleanup(info->m_type,CslConnectionState::GetConfig());
    CslConnectionState::Reset();

    if (!info)
    {
        wxASSERT(info);
        return;
    }

    info->Lock(false);

    ListUpdateServer(info);
    m_listInfo->UpdateInfo(info);
}

void CslListCtrlServer::OnTimer(wxTimerEvent& event)
{
    CslServerInfo *info=CslConnectionState::GetInfo();

    if (CslConnectionState::IsPlaying())
    {
        CslProcess::ProcessInputStream();
        if (m_timerCount==10)
            CslGame::ConnectCleanup(info->m_type,CslConnectionState::GetConfig());
    }
    else if (CslConnectionState::IsWaiting())
    {
        if (info->m_players!=info->m_playersMax)
        {
            CslConnectionState::Reset();
            ConnectToServer(info);
        }
        else if (m_timerCount%2==0)
        {
            if (CslConnectionState::CountDown())
            {
                wxString s=info->GetBestDescription();
                if (info->m_type==CSL_GAME_AC)
                {
                    wxUint32 l=s.Len();
                    s=A2U(StripColours(U2A(s),&l,2));
                }

                CslStatusBar::SetText(
                    wxString::Format(_("Waiting %d seconds for a free slot on \"%s\" " \
                                       "(press ESC to abort or join another server)"),
                                     CslConnectionState::GetWaitTime(),s.c_str()),1);
            }
            else
                CslStatusBar::SetText(wxT(""),1);
        }
    }
    m_timerCount++;
}

void CslListCtrlServer::ListInit(CslEngine *engine,
                                 CslListCtrlInfo *listInfo,
                                 CslListCtrlServer *listMaster,
                                 CslListCtrlServer *listFavourites,
                                 CslDlgExtended *extendedDlg,
                                 wxUint32 filterFlags)
{
    m_engine=engine;
    m_listInfo=listInfo;
    m_listMaster=listMaster;
    m_listFavourites=listFavourites;
    m_extendedDlg=extendedDlg;
    m_filterFlags=filterFlags;

    m_stdColourListText=GetTextColour();
    m_stdColourListItem=GetBackgroundColour();

    SetImageList(&m_imageList,wxIMAGE_LIST_SMALL);

    wxListItem item;

    item.SetMask(wxLIST_MASK_TEXT);
    item.SetImage(-1);

    item.SetText(_("Host"));
    InsertColumn(0,item);

    item.SetText(_("Description"));
    InsertColumn(1,item);

    item.SetText(_("Version"));
    InsertColumn(2,item);

    item.SetAlign(wxLIST_FORMAT_RIGHT);
    item.SetText(_("Ping"));
    InsertColumn(3,item);

    item.SetMask(wxLIST_MASK_TEXT);
    item.SetAlign(wxLIST_FORMAT_LEFT);
    item.SetText(_("Mode"));
    InsertColumn(4,item);

    item.SetText(_("Map"));
    InsertColumn(5,item);

    item.SetAlign(wxLIST_FORMAT_RIGHT);
    item.SetText(_("Time"));
    InsertColumn(6,item);

    item.SetAlign(wxLIST_FORMAT_LEFT);
    item.SetText(_("Player"));
    InsertColumn(7,item);

    item.SetText(_("MM"));
    InsertColumn(8,item);

    m_sortHelper.Init(CSL_SORT_ASC,SORT_PING);
    ToggleSortArrow();
}

void CslListCtrlServer::ListAdjustSize(wxSize size,bool init)
{
    if (!g_cslSettings->m_autoFitColumns && !init)
        return;

    wxInt32 w=size.x-8;

    SetColumnWidth(0,(wxInt32)(w*g_cslSettings->m_colServerS1));
    SetColumnWidth(1,(wxInt32)(w*g_cslSettings->m_colServerS2));
    SetColumnWidth(2,(wxInt32)(w*g_cslSettings->m_colServerS3));
    SetColumnWidth(3,(wxInt32)(w*g_cslSettings->m_colServerS4));
    SetColumnWidth(4,(wxInt32)(w*g_cslSettings->m_colServerS5));
    SetColumnWidth(5,(wxInt32)(w*g_cslSettings->m_colServerS6));
    SetColumnWidth(6,(wxInt32)(w*g_cslSettings->m_colServerS7));
    SetColumnWidth(7,(wxInt32)(w*g_cslSettings->m_colServerS8));
    SetColumnWidth(8,(wxInt32)(w*g_cslSettings->m_colServerS9));
}

bool CslListCtrlServer::PingOk(CslServerInfo *info)
{
    return info->m_ping>-1 && (wxInt32)(info->m_pingSend-info->m_pingResp) <
           g_cslSettings->m_updateInterval*2;
}

wxInt32 CslListCtrlServer::ListFindItem(CslServerInfo *info)
{
    wxListItem item;
    wxInt32 i;

    for (i=0;i<GetItemCount();i++)
    {
        item.SetId(i);
        CslServerInfo *infoCmp=((CslListServer*)GetItemData(item))->GetPtr();;
        if (info==infoCmp)
            return i;
    }

    return wxNOT_FOUND;
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
    bool pingOk=PingOk(info);
    CslListServer *infoCmp=NULL;

    i=m_servers.GetCount();
    for (j=0;j<i;j++)
    {
        if (m_servers.Item(j)->GetPtr()==info)
        {
            infoCmp=m_servers.Item(j);
            break;
        }
    }

    j=ListFindItem(info);
    if (j==wxNOT_FOUND)
        i=GetItemCount();
    else
        i=j;

    item.SetId(i);
    item.SetMask(wxLIST_MASK_TEXT|wxLIST_MASK_DATA);

    if (m_filterFlags)
    {
        if (m_filterFlags&CSL_FILTER_OFFLINE && !pingOk)
            found=true;
        else if (m_filterFlags&CSL_FILTER_FULL && info->m_playersMax>0 &&
                 info->m_players==info->m_playersMax)
            found=true;
        else if (m_filterFlags&CSL_FILTER_EMPTY && info->m_players==0)
            found=true;
        else if (m_filterFlags&CSL_FILTER_NONEMPTY && info->m_players>0)
            found=true;
        else if (m_filterFlags&CSL_FILTER_MM2 && info->m_mm==2)
            found=true;
        else if (m_filterFlags&CSL_FILTER_MM3 && info->m_mm==3)
            found=true;
        if (found && j!=wxNOT_FOUND)
        {
            ListDeleteItem(&item);
            wxInt32 pos=m_selected.Index(infoCmp);
            if (pos!=wxNOT_FOUND)
                m_selected.RemoveAt(pos);
        }
    }

    if (j==wxNOT_FOUND)
    {
        if (!infoCmp)
        {
            infoCmp=new CslListServer(info);
            m_servers.Add(infoCmp);
        }
        else
            infoCmp->Reset();

        if (found)
            return true;

        item.SetData((long)infoCmp);
        InsertItem(item);

        SetItem(i,0,info->m_host);
    }
    else
    {
        if (found)
            return true;
        infoCmp=(CslListServer*)GetItemData(item);
    }

    if (pingOk || info->m_pingResp)
    {
        if (infoCmp->m_desc != info->m_desc)
        {
            s=info->m_desc;
            if (info->m_type==CSL_GAME_AC)
            {
                wxUint32 l=s.Len();
                s=A2U(StripColours(U2A(info->m_desc),&l,2));
            }
            SetItem(i,1,s);
        }

        if (infoCmp->m_protocol != info->m_protocol)
            SetItem(i,2,info->GetVersionStr());

        if (infoCmp->m_ping != info->m_ping)
            SetItem(i,3,wxString::Format(wxT("%d"),info->m_ping));

        if (infoCmp->m_gameMode != info->m_gameMode)
            SetItem(i,4,info->m_gameMode);

        if (infoCmp->m_map != info->m_map)
            SetItem(i,5,info->m_map);

        if (infoCmp->m_timeRemain != info->m_timeRemain)
        {
            if (info->m_timeRemain<0)
                s=_("no limit");
            else
                s=s.Format(wxT("%d"),info->m_timeRemain);
            SetItem(i,6,s);
        }

        if (infoCmp->m_players != info->m_players ||
            infoCmp->m_playersMax != info->m_playersMax)
        {
            if (info->m_playersMax<=0)
                s=s.Format(wxT("%d"),info->m_players);
            else
                s=s.Format(wxT("%d/%d"),info->m_players,info->m_playersMax);
            SetItem(i,7,s);
        }

        if (infoCmp->m_mm != info->m_mm)
        {
            if (info->m_mm>=0)
            {
                s=s.Format(wxT("%d"),info->m_mm);
                if (info->m_mm==MM_OPEN)
                    s+=wxT(" (O)");
                else if (info->m_mm==MM_VETO)
                    s+=wxT(" (V)");
                else if (info->m_mm==MM_LOCKED)
                    s+=wxT(" (L)");
                else if (info->m_mm==MM_PRIVATE)
                    s+=wxT(" (P)");
                SetItem(i,8,s);
            }
        }
    }
    /*else
    {
        wxInt32 k;
        for (k=1;k<GetColumnCount();k++)
            SetItem(i,k,wxT(""));
    }*/

    wxColour colour=m_stdColourListText;
    if (!pingOk)
        colour=g_cslSettings->m_colServerOff;
    else if (info->m_playersMax>0 && info->m_players==info->m_playersMax)
        colour=g_cslSettings->m_colServerFull;
    else if (info->m_mm==MM_VETO)
        colour=g_cslSettings->m_colServerMM1;
    else if (info->m_mm==MM_LOCKED)
        colour=g_cslSettings->m_colServerMM2;
    else if (info->m_mm==MM_PRIVATE)
        colour=g_cslSettings->m_colServerMM3;
    else if (info->m_players==0)
        colour=g_cslSettings->m_colServerEmpty;
    SetItemTextColour(i,colour);

    found=false;
    if (!m_searchString.IsEmpty())
    {
        s=info->m_host.Lower();
        if (s.Find(m_searchString)!=wxNOT_FOUND)
            found=true;
        else
        {
            s=info->m_desc.Lower();
            if (s.Find(m_searchString)!=wxNOT_FOUND)
                found=true;
            else
            {
                s=info->GetVersionStr().Lower();
                if (s.Find(m_searchString)!=wxNOT_FOUND)
                    found=true;
                else
                {
                    s=info->m_gameMode.Lower();
                    if (s.Find(m_searchString)!=wxNOT_FOUND)
                        found=true;
                    else
                    {
                        s=info->m_map.Lower();
                        if (s.Find(m_searchString)!=wxNOT_FOUND)
                            found=true;
                    }
                }
            }
        }
    }
    SetItemBackgroundColour(item,found ? g_cslSettings->m_colServerHigh :
                            info->IsLocked() ? g_cslSettings->m_colServerPlay : m_stdColourListItem);

    item.SetMask(wxLIST_MASK_TEXT|wxLIST_MASK_IMAGE|wxLIST_MASK_DATA);
    if (m_id==CSL_LIST_MASTER)
    {
        if (!pingOk)
            i=CSL_LIST_IMG_GREY;
        else if (info->m_ping>(wxInt32)g_cslSettings->m_ping_bad)
            i=info->m_extended ? CSL_LIST_IMG_RED_EXT : CSL_LIST_IMG_RED;
        else if (info->m_ping>(wxInt32)g_cslSettings->m_ping_good)
            i=info->m_extended ? CSL_LIST_IMG_YELLOW_EXT : CSL_LIST_IMG_YELLOW;
        else
            i=info->m_extended ? CSL_LIST_IMG_GREEN_EXT : CSL_LIST_IMG_GREEN;

        if (infoCmp->ImgId()!=i)
        {
            SetItemImage(item,i);
            infoCmp->ImgId(i);
        }
    }
    else
    {
        switch (info->m_type)
        {
            case CSL_GAME_SB:
                i=info->m_extended ? CSL_LIST_IMG_SB_EXT : CSL_LIST_IMG_SB;
                break;
            case CSL_GAME_AC:
                i=info->m_extended ? CSL_LIST_IMG_AC_EXT : CSL_LIST_IMG_AC;
                break;
            case CSL_GAME_BF:
                i=info->m_extended ? CSL_LIST_IMG_BF_EXT : CSL_LIST_IMG_BF;
                break;
            case CSL_GAME_CB:
                i=CSL_LIST_IMG_CB;
                break;
            default:
                i=-1;
        }
        if (infoCmp->ImgId()!=i)
        {
            SetItemImage(item,i);
            infoCmp->ImgId(i);
        }
    }

    dynamic_cast<CslServerInfo&>(*infoCmp)=*info;

    return found;
}

void CslListCtrlServer::ListDeleteItem(wxListItem *item)
{
    m_dontRemoveOnDeselect=true;
    DeleteItem(*item);
    m_dontRemoveOnDeselect=false;
}

void CslListCtrlServer::RemoveServer(CslListServer *server,CslServerInfo *info,wxInt32 selId)
{
    wxInt32 i,l,c;

    if ((c=ListFindItem(info))!=wxNOT_FOUND)
    {
        wxListItem item;
        item.SetId(c);
        ListDeleteItem(&item);
    }

    if (!server)
    {
        l=m_servers.GetCount();
        for (i=0;i<l;i++)
            if (m_servers.Item(i)->GetPtr()==info)
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

wxUint32 CslListCtrlServer::ListUpdate(vector<CslServerInfo*> *servers)
{
    bool sort=false;
    wxUint32 c=0;

    loopv(*servers)
    {
        CslServerInfo *info=servers->at(i);
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

    if (!sort || !g_cslSettings->m_autoSortColumns)
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
}

wxUint32 CslListCtrlServer::ListSearch(wxString search)
{
    wxUint32 i,l,c=0;

    m_searchString=search.Lower();

    Freeze();

    l=m_servers.GetCount();
    for (i=0;i<l;i++)
    {
        CslServerInfo *info=m_servers.Item(i)->GetPtr();
        if (ListUpdateServer(info))
            c++;
    }

    Thaw();

    return c;
}

wxUint32 CslListCtrlServer::ListFilter(wxUint32 filterFlags)
{
    wxUint32 i,l,c=0;

    m_filterFlags=filterFlags;
    Freeze();

    l=m_servers.GetCount();
    for (i=0;i<l;i++)
    {
        CslServerInfo *info=m_servers.Item(i)->GetPtr();
        if (ListUpdateServer(info))
            c++;
    }

    ListSort(-1);
    Thaw();

    return c;
}

void CslListCtrlServer::ConnectToServer(CslServerInfo *info)
{
    if (CslConnectionState::IsPlaying())
    {
        wxMessageBox(_("You are currently playing, so quit the game and try again."),
                     _("Warning"),wxOK|wxICON_INFORMATION,this);
        return;
    }

    bool sbUseParam=false;
    wxString cmd,path,opts,out;
    wxString srvtitle=info->GetBestDescription();

    if (info->m_type==CSL_GAME_AC)
    {
        wxUint32 l=srvtitle.Len();
        srvtitle=A2U(StripColours(U2A(srvtitle),&l,2));
    }

    switch (info->m_type)
    {
        case CSL_GAME_SB:
            cmd=g_cslSettings->m_clientBinSB;
            path=g_cslSettings->m_configPathSB;
            opts=g_cslSettings->m_clientOptsSB;
            break;
        case CSL_GAME_AC:
            cmd=g_cslSettings->m_clientBinAC;
            path=g_cslSettings->m_configPathAC;
            opts=g_cslSettings->m_clientOptsAC;
            break;
        case CSL_GAME_BF:
            cmd=g_cslSettings->m_clientBinBF;
            path=g_cslSettings->m_configPathBF;
            opts=g_cslSettings->m_clientOptsBF;
            break;
        case CSL_GAME_CB:
            cmd=g_cslSettings->m_clientBinCB;
            path=g_cslSettings->m_configPathCB;
            opts=g_cslSettings->m_clientOptsCB;
            break;
        default:
            return;
    }

    if (cmd.IsEmpty())
    {
        wxMessageBox(wxString::Format(_("There was no executable for game %s specified yet!" \
                                        "\nPlease check your settings."),GetGameStr(info->m_type)),
                     _("Error"),wxICON_ERROR,this);
        return;
    }
    if (path.IsEmpty() || !::wxDirExists(path))
    {
        wxMessageBox(_("Invalid game path was specified!\nPlease check your settings."),
                     _("Error"),wxICON_ERROR,this);
        return;
    }

    switch (info->m_type)
    {
        case CSL_GAME_SB:
        case CSL_GAME_BF:
        {
            if (info->m_mm==MM_PRIVATE)
            {
                if (wxMessageBox(wxString::Format(_("The server \"%s\" is in private mode, so it's\n" \
                                                    "probably not possible to connect.\n\nProceed anyway?"),
                                                  srvtitle.c_str()),
                                 _("Question"),wxYES_NO|wxICON_QUESTION,this) != wxYES)
                    return;
                sbUseParam=true;
            }
            else if (info->m_mm!=MM_LOCKED &&
                     (info->m_map.IsEmpty() ||
                      info->m_map.CmpNoCase(CSL_DEFAULT_INJECT_FIL_SB)==0 ||
                      info->m_timeRemain==0))
                sbUseParam=true;

            if (sbUseParam)
            {
#ifdef __WXMSW__
                opts+=wxT(" -x\"connect ")+info->m_host+wxT("\"");
#else
                opts+=wxT(" -xconnect\\ ")+info->m_host;
#endif
            }
            else
                opts=opts+wxT(" -l")+CSL_DEFAULT_INJECT_FIL_SB;

            break;
        }

        case CSL_GAME_AC:
        case CSL_GAME_CB:
            break;

        default:
            break;
    }

    cmd+=wxT(" ")+opts;

    CslConnectionState::Reset();

    if (info->m_players>0 && info->m_players==info->m_playersMax)
    {
        wxInt32 time=g_cslSettings->m_waitServerFull;
        CslDlgConnectWait *dlg=new CslDlgConnectWait(this,&time);
        if (dlg->ShowModal()==wxID_OK)
        {
            m_timerCount=0;
            CslConnectionState::CreateWaitingState(info,this,time);
        }
        return;
    }

    if (!sbUseParam)
    {
        switch (CslGame::ConnectPrepare(info,path,&out))
        {
            case CSL_ERROR_NONE:
                break;

            case CSL_ERROR_GAME_UNKNOWN:
                return;

            case CSL_ERROR_FILE_OPERATION:
                return;

            case CSL_ERROR_FILE_DONT_EXIST:
                wxMessageBox(out+_("\nPlease check your installation!"),_("Error"),wxICON_ERROR,this);
                return;

            default:
                return;
        }
    }

    CslDlgOutput::Reset(srvtitle);

    info->Lock();
    ::wxSetWorkingDirectory(path);

    CslProcess *process=new CslProcess(this,info,cmd);
    if (!(::wxExecute(cmd,wxEXEC_ASYNC,process)>0))
    {
        wxMessageBox(_("Failed to start: ")+cmd
                     ,_("Error"),wxICON_ERROR,this);
        info->Lock(false);
        return;
    }

    m_timerCount=0;
    CslConnectionState::CreatePlayingState(info,this,out);

    info->m_connectedTimes++;
    info->m_playLast=wxDateTime::Now().GetTicks();
    m_listInfo->UpdateInfo(info);
    ListUpdateServer(info);
}

void CslListCtrlServer::ToggleSortArrow()
{
    wxListItem item;
    wxInt32 img=-1;

    if (g_cslSettings->m_autoSortColumns)
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
            g_cslSettings->m_autoSortColumns ? img=CSL_LIST_IMG_SORT_ASC : img=CSL_LIST_IMG_SORT_ASC_LIGHT;
            m_sortHelper.m_sortMode=CSL_SORT_ASC;
        }
        else
        {
            g_cslSettings->m_autoSortColumns ? img=CSL_LIST_IMG_SORT_DSC : img=CSL_LIST_IMG_SORT_DSC_LIGHT;
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

// since WX > 2.8.4 the items get deselected when sorting !?
#if wxVERSION_NUMBER > 2804
    CslServerInfo **selected=new CslServerInfo*[GetItemCount()];
    wxInt32 c=0,i,j;

    for (i=0;i<GetItemCount();i++)
        if (GetItemState(i,wxLIST_STATE_SELECTED) & wxLIST_STATE_SELECTED)
            selected[c++]=(CslServerInfo*)((CslListServer*)GetItemData(i))->GetPtr();
#endif

    SortItems(ListSortCompareFunc,(long)&m_sortHelper);

#if wxVERSION_NUMBER > 2804
    for (i=0;i<c;i++)
    {
        j=ListFindItem(selected[i]);
        if (j==wxNOT_FOUND)
            continue;
        SetItemState(j,wxLIST_STATE_SELECTED,wxLIST_STATE_SELECTED);
    }

    delete[] selected;
#endif
}

int wxCALLBACK CslListCtrlServer::ListSortCompareFunc(long item1,long item2,long data)
{
    CslServerInfo *info1=(CslServerInfo*)item1;
    CslServerInfo *info2=(CslServerInfo*)item2;

    bool ping1Ok=PingOk(info1);
    bool ping2Ok=PingOk(info2);

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
            vs1=info1->m_host;
            vs2=info2->m_host;
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

        case SORT_DESC:
            type=CSL_LIST_SORT_STRING;
            vs1=info1->m_desc;
            vs2=info2->m_desc;
            break;

        case SORT_PING:
            type=CSL_LIST_SORT_UINT;
            vui1=info1->m_ping;
            vui2=info2->m_ping;
            break;

        case SORT_VER:
            type=CSL_LIST_SORT_INT;
            vi1=info1->m_protocol;
            vi2=info2->m_protocol;
            break;

        case SORT_MODE:
            type=CSL_LIST_SORT_STRING;
            vs1=info1->m_gameMode;
            vs2=info2->m_gameMode;
            break;

        case SORT_MAPS:
            type=CSL_LIST_SORT_STRING;
            vs1=info1->m_map;
            vs2=info2->m_map;
            break;

        case SORT_TIME:
            type=CSL_LIST_SORT_UINT;
            vui1=info1->m_timeRemain;
            vui2=info2->m_timeRemain;
            break;

        case SORT_PLAY:
            type=CSL_LIST_SORT_INT;
            vi1=info1->m_players;
            vi2=info2->m_players;
            break;

        case SORT_MM:
            type=CSL_LIST_SORT_INT;
            vi1=info1->m_mm;
            vi2=info2->m_mm;
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
