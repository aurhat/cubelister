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
#include "engine/CslEngine.h"
#include "engine/CslTools.h"
#include "CslListCtrlInfo.h"
#include "CslStatusBar.h"


enum { CSL_HIGHLIGHT_SEARCH_SERVER = 1<<0, CSL_HIGHLIGHT_SEARCH_PLAYER = 1<<1 };


class CslListCtrlServer;


class CslConnectionState
{
    public:
        static void Reset()
        {
            if (m_activeInfo) m_activeInfo->SetWaiting(false);
            m_playing=false;
            m_waitTime=0;
            m_activeList=NULL;
            m_activeInfo=NULL;
            CslStatusBar::SetText(1,wxEmptyString);
        }

        static void CreateWaitingState(CslServerInfo *info,CslListCtrlServer *list,
                                       const wxInt32 time)
        {
            m_activeInfo=info;
            m_waitTime=time;
            m_activeList=list;
            info->SetWaiting(true);
        }

        static bool CountDown()
        {
            if (--m_waitTime==0)
            {
                Reset();
                return false;
            }
            return true;
        }

        static void CreatePlayingState(CslServerInfo *info,CslListCtrlServer *list)
        {
            m_activeInfo=info;
            m_activeList=list;
            m_playing=true;
        }

        static bool IsPlaying() { return m_playing; }
        static bool IsWaiting() { return m_waitTime>0; }
        static wxInt32 GetWaitTime() { return m_waitTime; }
        static CslListCtrlServer* GetList() { return m_activeList; }
        static CslServerInfo* GetInfo() { return m_activeInfo; }

    private:
        static bool m_playing;
        static wxInt32 m_waitTime;
        static CslListCtrlServer *m_activeList;
        static CslServerInfo *m_activeInfo;
};

class CslListServerData
{
    public:
        CslListServerData(CslServerInfo *info) :
                Info(info)
                {
                    Reset();
                }

        CslListServerData& operator=(const CslServerInfo& info)
        {
            Host=info.Host;
            Description=info.GetBestDescription();
            GameMode=info.GameMode;
            Map=info.Map;
            Protocol=info.Protocol;
            Ping=info.Ping;
            TimeRemain=info.TimeRemain;
            Players=info.Players;
            PlayersMax=info.PlayersMax;
            MM=info.MM;
            return *this;
        }

        void Reset()
        {
            Host.Empty();
            Description.Empty();
            GameMode.Empty();
            Map.Empty();
            Protocol=-1;
            Ping=-1;
            TimeRemain=-2;
            Players=-1;
            PlayersMax=-1;
            MM=-1;
            ImgId=-1;
            HighLight=0;
        }

        bool SetHighlight(const wxInt32 type,const bool value)
        {
            if (value)
                HighLight|=type;
            else
                HighLight&=~type;

            return HighLight!=0;
        }

        CslServerInfo *Info;
        wxString Host;
        wxString Description;
        wxString GameMode;
        wxString Map;
        wxInt32 Protocol;
        wxInt32 Ping;
        wxInt32 TimeRemain;
        wxInt32 Players;
        wxInt32 PlayersMax;
        wxInt32 MM;
        wxInt32 ImgId;
        wxUint32 HighLight;
};

WX_DEFINE_ARRAY_PTR(CslListServerData*,t_aCslListServerData);


class CslListCtrlServer : public wxListCtrl
{
    public:
        enum { CSL_LIST_MASTER, CSL_LIST_FAVOURITE };

        CslListCtrlServer(wxWindow* parent,wxWindowID id,const wxPoint& pos=wxDefaultPosition,
                          const wxSize& size=wxDefaultSize,long style=wxLC_ICON,
                          const wxValidator& validator=wxDefaultValidator,
                          const wxString& name=wxListCtrlNameStr);
        ~CslListCtrlServer();

        void ListInit(CslEngine *engine,
                      CslListCtrlInfo *listInfo,
                      CslListCtrlServer *sibling
                      /*,CslDlgExtended *extendedDlg*/);
        wxUint32 ListUpdate(vector<CslServerInfo*>& servers);
        void ListClear();
        void ListSort(wxInt32 column=-1);
        void ToggleSortArrow();
        wxUint32 ListSearch(const wxString& search);
        wxUint32 ListFilter();
        void ListAdjustSize(const wxSize& size,bool init=false);
        void SetMasterSelected(bool selected) { m_masterSelected=selected; }
        void Highlight(wxInt32 type,bool highlight,CslServerInfo *info=NULL,wxListItem *listitem=NULL);

    private:
        wxInt32 m_id;
        CslGame *m_game;
        CslEngine *m_engine;

        bool m_masterSelected;
        wxUint32 m_timerCount;

        CslListCtrlInfo *m_listInfo;
        CslListCtrlServer *m_sibling;

        bool m_dontUpdateInfo;  // don't update info list on ctrl+a
        bool m_dontRemoveOnDeselect;

        t_aCslListServerData m_selected;
        t_aCslListServerData m_servers;
        wxString m_searchString;
        wxUint32 *m_filterFlags;
        wxInt32 m_filterVersion;

        wxImageList m_imageList;
        wxColour m_stdColourListText;
        wxColour m_stdColourListItem;

        CslListSortHelper m_sortHelper;

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

        void ListCreateGameBitmaps();
        wxInt32 ListFindItem(CslServerInfo *info,wxListItem& item);
        void ListDeleteItem(wxListItem *item);
        bool ListSearchItemMatches(CslServerInfo *info);
        bool ListFilterItemMatches(CslServerInfo *info);
    public:
        bool ListUpdateServer(CslServerInfo *info);
        void RemoveServer(CslListServerData *server,CslServerInfo *info,wxInt32 id);
        void ConnectToServer(CslServerInfo *info,wxUint32 mode=CslGame::CSL_CONNECT_DEFAULT);
        void ListRemoveServers();
        void ListDeleteServers();
};

#endif // CSLLISTCTRLSERVER_H
