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

#ifndef CSLLISTCTRLSERVER_H
#define CSLLISTCTRLSERVER_H

/**
 @author Glen Masgai <mimosius@gmx.de>
*/

#include <wx/wxprec.h>
#ifdef __BORLANDC__
#pragma hdrstop
#endif
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/imaglist.h>
#include <wx/process.h>
#include "CslEngine.h"
#include "CslListCtrlInfo.h"
#include "CslStatusBar.h"


enum { CSL_LIST_MASTER = 0, CSL_LIST_FAVOURITE };

enum { CSL_SORT_ASC = 0, CSL_SORT_DSC };
enum
{
    SORT_HOST = 0, SORT_DESC, SORT_PING, SORT_MODE,
    SORT_MAPS, SORT_TIME, SORT_PLAY, SORT_MM, SORT_UNKNOWN
};

class CslListCtrlServer;


class CslConnectionState
{
    private:
        static bool m_playing;
        static wxInt32 m_waitTime;
        static wxInt32 m_timerTicks;
        static CslListCtrlServer *m_waitList;
        static CslServerInfo *m_waitInfo;

    public:
        static void Reset()
        {
            if (m_waitInfo) m_waitInfo->SetWaiting(false);
            m_playing=false;
            m_waitTime=0;
            m_waitList=NULL,m_waitInfo=NULL;
            CslStatusBar::SetText(wxT(""),1);
        }

        static void CreateWaitingState(CslServerInfo *info,wxInt32 time,CslListCtrlServer *list)
        {
            m_waitInfo=info;
            m_waitTime=time;
            m_waitList=list;
            info->SetWaiting(true);
        }
        static bool DecTime()
        {
            m_waitTime--;
            if (m_waitTime==0)
            {
                Reset();
                return false;
            }
            return true;
        }

        static void SetPlaying(bool val)
        {
            m_playing=val;
        }

        static bool IsPlaying()
        {
            return m_playing;
        }
        static bool IsWaiting()
        {
            return m_waitTime>0;
        }
        static wxInt32 GetWaitTime()
        {
            return m_waitTime;
        }
        static CslListCtrlServer* GetList()
        {
            return m_waitList;
        }
        static CslServerInfo* GetInfo()
        {
            return m_waitInfo;
        }
};


class CslSortHelper
{
    public:
        CslSortHelper() :
                m_sortMode(CSL_SORT_ASC),
                m_sortType(SORT_PING) {}

        wxInt32 m_sortMode;
        wxInt32 m_sortType;
};

class CslListServer : public CslServerInfo
{
    public:
        CslListServer(CslServerInfo *info) :
                CslServerInfo(), m_info(info),m_imgId(-1) {}

        void Reset()
        {
            m_desc.Empty();
            m_gameMode.Empty();
            m_map.Empty();
            m_ping=-1;
            m_timeRemain=-2;
            m_players=-1;
            m_playersMax=-1;
            m_mm=-1;
            m_imgId=-1;
        }

        CslServerInfo* GetPtr()
        {
            return m_info;
        }
        bool HasStats()
        {
            return m_playLast>0 || m_playTimeLastGame>0 ||
                   m_playTimeTotal>0 || m_connectedTimes>0;
        }
        wxInt32 ImgId()
        {
            return m_imgId;
        }
        void ImgId(wxInt32 imgId)
        {
            m_imgId=imgId;
        }

    protected:
        CslServerInfo *m_info;
        wxInt32 m_imgId;
};

WX_DEFINE_ARRAY_PTR(CslListServer*,t_aCslServerInfo);


class CslListCtrlServer : public wxListCtrl
{
    public:
        CslListCtrlServer(wxWindow* parent,wxWindowID id,const wxPoint& pos=wxDefaultPosition,
                          const wxSize& size=wxDefaultSize,long style=wxLC_ICON,
                          const wxValidator& validator=wxDefaultValidator,
                          const wxString& name=wxListCtrlNameStr);

        ~CslListCtrlServer();

        void ListInit(CslEngine *engine,CslListCtrlInfo *listInfo,CslListCtrlServer *listMaster,
                      CslListCtrlServer *istFavourites,wxUint32 filterFlags);
        wxUint32 ListUpdate(vector<CslServerInfo*> *servers);
        void ListClear();
        void ToggleSortArrow();
        wxUint32 ListSearch(wxString search);
        wxUint32 ListFilter(wxUint32 filterFlags);
        void ClearStartConfig(wxString path);
        void ListAdjustSize(wxSize size,bool init=false);
        void SetMasterSelected(bool selected)
        {
            m_masterSelected=selected;
        }

    private:
        wxInt32 m_id;
        CslGame *m_game;
        CslEngine *m_engine;

        bool m_masterSelected;
        wxUint32 m_timerCount;

        CslListCtrlInfo *m_listInfo;
        CslListCtrlServer *m_listMaster;
        CslListCtrlServer *m_listFavourites;
        bool m_dontUpdateInfo;  // don't update info list on ctrl+a
        bool m_dontRemoveOnDeselect;

        t_aCslServerInfo m_selected;
        t_aCslServerInfo m_servers;
        wxString m_searchString;
        wxUint32 m_filterFlags;

        wxImageList m_imageList;
        wxColour m_stdColourListText;
        wxColour m_stdColourListItem;

        CslSortHelper m_sortHelper;

        void OnShow(wxShowEvent& event);
        void OnSize(wxSizeEvent& event);
        void OnKeyDown(wxKeyEvent &event);
        void OnColumnLeftClick(wxListEvent& event);
        void OnItemActivated(wxListEvent& event);
        void OnItemSelected(wxListEvent& event);
        void OnItemDeselected(wxListEvent& event);
        void OnContextMenu(wxContextMenuEvent& event);
        void OnMenu(wxCommandEvent& event);
        void OnEndProcess(wxCommandEvent& event);
        void OnTimer(wxTimerEvent& event);

        static int wxCALLBACK ListSortCompareFunc(long item1,long item2,long data);

        DECLARE_EVENT_TABLE()

    protected:
        static bool PingOk(CslServerInfo *info);
        wxInt32 ListFindItem(CslServerInfo *info);
        void ListDeleteItem(wxListItem *item);
        bool ListUpdateServer(CslServerInfo *info);
        void RemoveServer(CslListServer *server,CslServerInfo *info,wxInt32 id);
        void ListSort(wxInt32 column);
        void ConnectToServer(CslServerInfo *info);
};

#endif // CSLLISTCTRLSERVER_H
