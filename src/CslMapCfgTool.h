/***************************************************************************
 *   Copyright (C) 2007-2013 by Glen Masgai                                *
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

#ifndef CSLMAPCFGTOOL_H
#define CSLMAPCFGTOOL_H

/**
 @author Glen Masgai <mimosius@users.sourceforge.net>
*/


class CslMapToolPanelMap : public CslPanelMap
{
    public:
        CslMapToolPanelMap(wxWindow *parent,wxInt32 id) :
                CslPanelMap(parent,id),
                m_id(id),m_panelZoom(NULL),m_basesPtr(NULL),m_activeBase(-1),
                m_centre(wxPoint(-1,-1)),m_mouseLeftDown(false) {}

        void Init(CslMapToolPanelMap *panel,CslArrayCslBaseInfo *basesPtr)
        {
            m_panelZoom=panel;
            m_basesPtr=basesPtr;
        }
        void SetCentre(const wxPoint& centre) { m_centre=centre; }
        void SetActiveBase(wxInt32 id) { m_activeBase=id; }

    private:
        wxInt32 m_id;

        CslMapToolPanelMap *m_panelZoom;
        CslArrayCslBaseInfo *m_basesPtr;
        wxInt32 m_activeBase;

        wxPoint m_centre;
        bool m_mouseLeftDown;

        void OnPaintOverlay(wxCommandEvent& event);
        void OnPaint(wxPaintEvent& event);
        void OnMouseMove(wxMouseEvent& event);
        void OnMouseLeftDown(wxMouseEvent& event);
        void OnMouseLeftUp(wxMouseEvent& event);

        DECLARE_EVENT_TABLE()

    protected:
        void SetBasePosition(const wxPoint& point);
};


class CslMapCfgTool: public wxDialog
{
    public:
        // begin wxGlade: CslMapCfgTool::ids
        // end wxGlade

        CslMapCfgTool(wxWindow* parent,int id,const wxString& title,
                      const wxPoint& pos=wxDefaultPosition,
                      const wxSize& size=wxDefaultSize,
                      long style=wxDEFAULT_DIALOG_STYLE);
        ~CslMapCfgTool() { Reset(); }

    private:
        // begin wxGlade: CslMapCfgTool::methods
        void set_properties();
        void do_layout();
        // end wxGlade

        wxString m_fileName;
        wxString m_pngPath,m_cfgPath;

        wxInt32 m_lastInfo;
        CslMapInfo m_mapInfo;

        void OnClose(wxCloseEvent& event);
        void OnCommandEvent(wxCommandEvent& event);

        DECLARE_EVENT_TABLE()

    protected:
        // begin wxGlade: CslMapCfgTool::attributes
        wxStaticBox* sizer_control_staticbox;
        wxStaticBox* sizer_map_staticbox;
        CslMapToolPanelMap* panel_bitmap;
        wxChoice* choice_version;
        wxButton* button_version_add;
        wxButton* button_version_del;
        wxStaticLine* static_line_4;
        wxChoice* choice_base;
        wxButton* button_base_add;
        wxButton* button_base_del;
        wxStaticLine* static_line_1;
        wxTextCtrl* text_ctrl_map_name;
        wxTextCtrl* text_ctrl_author;
        wxStaticLine* static_line_2;
        CslMapToolPanelMap* panel_zoom;
        wxStaticLine* static_line_3;
        wxButton* button_load_image;
        wxButton* button_load;
        wxButton* button_save;
        wxButton* button_clear;
        wxButton* button_close;
        // end wxGlade

        bool LoadImage();
        void LoadConfig();
        void SaveConfig();
        void AddVersion();
        void DelVersion();
        void AddBase();
        void DelBase();
        void SetCurrentBase(wxInt32 Id);
        void ChoiceSetVersion(wxInt32 id);
        void ChoiceFillBases();
        void Reset(const bool resetMap=true);
}; // wxGlade: end class


class CslMapCfgToolApp: public wxApp
{
    public:
        bool OnInit();
};

#endif // CSLMAPCFGTOOL_H
