/***************************************************************************
 *   Copyright (C) 2007-2014 by Glen Masgai                                *
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

#ifndef CSLFRAME_H
#define CSLFRAME_H

#include "CslDlgExtended.h"
#include "CslDlgSettings.h"
#include "CslDlgOutput.h"
#include "CslGameConnection.h"
#include "CslMenu.h"
#include "CslPanelCountryView.h"
#include "CslPanelPlayerSearch.h"
#include "CslPanelServerView.h"
#include "CslPanelSearch.h"
#include "CslPanelTraffic.h"
#include "CslServerBrowser.h"
#include "CslServerInfo.h"
#include "CslStatusBar.h"
#include "CslIPC.h"
#include "CslTTS.h"


class CslTaskBarIcon;

class CslFrame: public wxFrame, public CslPluginMgr, public CslPluginHost
{
    friend class CslTaskBarIcon;

    public:
        CslFrame();
        ~CslFrame();

        wxAuiManager& GetAuiManager()
        {
            return m_aui;
        }

        CslArrayCslServerView& GetServerViews()
        {
            return m_serverViews;
        }

        void CreateServerView(CslServerInfo *info, wxUint32 view,
                              const wxString& name = wxEmptyString);

        // CslPluginHost implementations
        wxWindow* GetMainWindow()
            { return this; }
        wxEvtHandler* GetEvtHandler()
            { return this; }
        wxString GetHomeDir(wxPathFormat format = wxPATH_NATIVE);

        wxInt32 GetFreeId();
        wxInt32 GetFreeIds(wxInt32 count, wxInt32 ids[]);

        wxMenu* GetPluginMenu();
        wxMenuBar* GetMainMenuBar()
            { return GetMenuBar(); }

        CslEngine* GetCslEngine()
            { return m_engine;  }
        CslGame* GetSelectedGame()
            { return TreeGamesGetSelectedGame(); }

    private:
        void CreateMainMenu();
        void CreateControls();
        void SetProperties();
        void DoLayout();
        void SetAuiPaneSettings();
        wxAuiPaneInfo& AuiResizeFixedPane(wxAuiPaneInfo& pane, wxWindow *window);

        void CountryViewSetCaption(CslServerInfo *info);
        wxString ServerViewGetCaption(CslServerInfo *info, bool selected);
        void ServerBrowserSetCaption(wxInt32 id, const wxString& addon = wxEmptyString);

        void ToggleShow(wxInt32 mode = 0);
#ifndef __WXMAC__
        void ToggleTrayIcon();
#endif
        void ToggleAuiPane(wxInt32 id, bool force = false);

        void SetTotalPlaytime(CslGame *game);

        bool TreeGamesScrollToSelection();
        wxTreeItemId TreeGamesFindItem(wxTreeItemId parent, CslGame *game, CslMaster *master);
        void TreeGamesAddMaster(wxTreeItemId parent, CslMaster *master, bool focus = false);
        void TreeGamesRemoveMaster();
        void TreeGamesAddGame(CslGame *game, wxInt32 img ,bool focus = false);
        CslGame* TreeGamesGetSelectedGame(wxTreeItemId *item = NULL);
        CslMaster* TreeGamesGetSelectedMaster(wxTreeItemId *item = NULL);

        wxString CheckVersion(wxInputStream& input);

        void InitProtocolInputs();
        void DeinitProtocolInputs();
        void CheckProtocolInputs();

        void UpdateMaster();
        void ConnectToServer(CslServerInfo *info = NULL,
                             wxInt32 pass = CslGameConnection::NO_PASS);

        wxUint32 LoadLocators();
        void SaveLocators();

        void OnPong(CslPongEvent& event);
        void OnMasterUpdate(CslMasterUpdateEvent& event);
        void OnTimer(wxTimerEvent& event);
        void OnListItemSelected(wxListEvent& event);
        void OnListItemActivated(wxListEvent& event);
        void OnTreeSelChanged(wxTreeEvent& event);
        void OnTreeRightClick(wxTreeEvent& event);
        void OnCommandEvent(wxCommandEvent& event);
        void OnKeypress(wxKeyEvent& event);
        void OnSize(wxSizeEvent& event);
#ifndef __WXMAC__
        void OnIconize(wxIconizeEvent& event);
        void OnTrayIcon(wxTaskBarIconEvent& event);
#endif
        void OnShow(wxShowEvent& event);
        void OnClose(wxCloseEvent& event);
        void OnAuiPaneClose(wxAuiManagerEvent& event);
        void OnToolTip(CslToolTipEvent& event);
        void OnCslProtocolInput(CslProtocolInputEvent& event);
        void OnEndProcess(wxCommandEvent& event);
        void OnIPC(CslIpcEvent& event);

        DECLARE_EVENT_TABLE()

        wxNotebook *notebook_views;
        CslListCtrlServer *list_ctrl_master, *list_ctrl_favourites;
        CslPanelServerView *player_info;
        CslListCtrlInfo *list_ctrl_info;
        CslListCtrlPlayerSearch *list_ctrl_player_search;
        CslPanelSearch *pane_search;
        CslPanelCountry *pane_country;
        CslPanelTraffic *pane_traffic;
        wxTreeCtrl *tree_ctrl_games;
        wxMenu *menuMaster;
        wxAuiManager m_aui;
#ifndef __WXMAC__
        CslTaskBarIcon *m_taskbar_icon;
#endif
        bool m_maximised;

        wxString m_aui_default_layout;

        wxImageList m_imgListTree;
        wxTreeItemId m_treeGamesRoot;

        wxTimer m_timer;
        bool m_timerInit;
        wxInt32 m_timerCount, m_timerUpdate;

        CslEngine *m_engine;
        CslIpcServer *m_ipcServer;

        CslProtocolInput *m_protocolInput;
        wxArrayPtrVoid m_protocolInputCookies;

        time_t m_lastVersionCheck;
        time_t m_lastVersionDate;

        CslDlgOutput *m_outputDlg;
        CslDlgExtended *m_extendedDlg;

        wxString m_toolTipTitle;
        wxString m_toolTipTextLeft;
        wxString m_toolTipTextRight;

        CslServerInfo *m_oldSelectedInfo;

        CslArrayCslServerView m_serverViews;
        CslArrayCslServerInfo m_countryServers;
        CslArrayCslServerInfo m_searchedServers;

        wxString m_searchString;
        wxInt32 m_searchResultPlayer, m_searchResultServer;

};

#endif //CSLFRAME_H
