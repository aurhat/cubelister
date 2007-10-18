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

#ifndef CSLDLGEXTENDED_H
#define CSLDLGEXTENDED_H

#include <wx/wxprec.h>
#ifdef __BORLANDC__
#pragma hdrstop
#endif
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/imaglist.h>
// begin wxGlade: ::dependencies
#include <wx/listctrl.h>
#include <wx/statline.h>
// end wxGlade
#include "CslEngine.h"
#include "CslTools.h"


class CslBaseInfo
{
    public:
        CslBaseInfo(const wxInt32 x,const wxInt32 y) :
                m_point(wxPoint(x,y)),m_colour(*wxWHITE) {}

        wxPoint m_point;
        wxColour m_colour;
};

WX_DEFINE_ARRAY_PTR(CslBaseInfo*,t_aBaseInfo);

class CslMapInfo
{
    public:
        CslMapInfo() { Reset(); }
        ~CslMapInfo() { Reset(); }

        bool LoadMapData(const wxString& mapName,const wxString& gameName,
                         const wxUint32 protVersion);
        void Reset(const wxString& mapName=wxEmptyString)
        {
            m_mapName=mapName;
            m_mapNameFull.Empty();
            m_author.Empty();
            m_basesOk=false;
            WX_CLEAR_ARRAY(m_bases);
        }

        void ResetBasesColour()
        {
            for (wxUint32 i=0;i<m_bases.GetCount();i++)
                m_bases.Item(i)->m_colour=*wxWHITE;
        }

        wxString m_mapName,m_mapNameFull,m_author;
        wxBitmap m_bitmap;
        t_aBaseInfo m_bases;
        bool m_basesOk;
};


class CslPanelMap : public wxPanel
{
    public:
        CslPanelMap(wxWindow *parent,wxInt32 id) : wxPanel(parent,id), m_ok(false) {}
        ~CslPanelMap() { WX_CLEAR_ARRAY(m_bases); }

        void SetMap(const wxBitmap& bitmap,const bool refresh)
        {
            m_bitmap=bitmap;
            m_ok=true;

            SetMinSize(wxSize(bitmap.GetWidth(),bitmap.GetHeight()));

            if (refresh)
                Refresh(false);
        }

        void SetOk(const bool ok=true) { m_ok=ok; }

        void UpdateBases(const t_aBaseInfo& bases,const bool hasBases);

    private:
        wxMemoryDC m_memDC;
        wxBitmap m_bitmap;

        t_aBaseInfo m_bases;

        void OnPaint(wxPaintEvent& event);
        void OnErase(wxEraseEvent& event) {}

        DECLARE_EVENT_TABLE();

    protected:
        bool m_ok;

        wxPoint GetBitmapOrigin()
        {
            wxSize cSize=GetClientSize();
            wxSize bSize(m_bitmap.GetWidth(),m_bitmap.GetHeight());
            return wxPoint(cSize.x/2-bSize.x/2,cSize.y/2-bSize.y/2);
        }
};


WX_DEFINE_ARRAY_PTR(wxStaticText*,t_aLabel);

class CslDlgExtended: public wxDialog
{
    public:
        // begin wxGlade: CslDlgExtended::ids
        // end wxGlade

        CslDlgExtended(wxWindow* parent,int id=wxID_ANY,const wxString& title=wxEmptyString,
                       const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize,
                       long style=wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER);
        ~CslDlgExtended();

        void ListInit(CslEngine *engine);
        void DoShow(CslServerInfo *info);
        CslServerInfo* GetInfo() { return m_info; }

    private:
        // begin wxGlade: CslDlgExtended::methods
        void set_properties();
        void do_layout();
        // end wxGlade

        CslEngine *m_engine;
        CslServerInfo *m_info;
        CslMapInfo m_mapInfo;

        wxFont m_labelFont;
        t_aLabel m_teamLabel;

        wxImageList m_imageList;
        CslListSortHelper m_sortHelper;

        wxFlexGridSizer *m_gridSizerMain,*m_gridSizerList,*m_gridSizerInfo;
        wxStaticBoxSizer *m_sizerMap,*m_sizerMapLabel;

        void OnClose(wxCloseEvent& event);
        void OnSize(wxSizeEvent& event);
        void OnColumnLeftClick(wxListEvent& event);
        void OnTimer(wxTimerEvent& event);
        void OnCommandEvent(wxCommandEvent& event);
        void OnPong(wxCommandEvent& event);

        DECLARE_EVENT_TABLE()

    protected:
        // begin wxGlade: CslDlgExtended::attributes
        wxStaticBox* sizer_map_label_staticbox;
        wxStaticBox* sizer_check_staticbox;
        wxStaticBox* sizer_info_staticbox;
        wxStaticBox* sizer_team_score_staticbox;
        wxStaticBox* sizer_map_staticbox;
        wxListCtrl* list_ctrl_extended;
        CslPanelMap* panel_map;
        wxStaticText* label_team1;
        wxStaticText* label_team2;
        wxStaticText* label_team3;
        wxStaticText* label_team4;
        wxStaticText* label_team5;
        wxStaticText* label_team6;
        wxStaticText* label_team7;
        wxStaticText* label_team8;
        wxStaticText* label_server;
        wxStaticText* label_mode;
        wxStaticText* label_remaining;
        wxStaticText* label_records;
        wxCheckBox* checkbox_update;
        wxCheckBox* checkbox_update_end;
        wxCheckBox* checkbox_map;
        wxStaticText* label_map;
        wxStaticText* label_author_prefix;
        wxStaticText* label_author;
        wxButton* button_update;
        wxButton* button_close;
        // end wxGlade

        void UpdateMap();
        void ClearTeamScoreLabel(const wxUint32 start,const wxUint32 end);
        void SetTeamScore();
        void QueryInfo(wxInt32 pid=-1);
        void RecalcMinWidth(const bool forcemin=false);
        void ListAdjustSize(const wxSize& size);
        void ListSort(wxInt32 column);
        void ToggleSortArrow();
        void ShowPanelMap(const bool show, const bool checkbox);

        static int wxCALLBACK ListSortCompareFunc(long item1,long item2,long data);
}; // wxGlade: end class


#endif // CSLDLGEXTENDED_H
