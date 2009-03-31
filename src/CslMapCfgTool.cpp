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

#include "Csl.h"
#include "CslPanelMap.h"
#include "CslMapCfgTool.h"


enum
{
    COMBO_VERSION     = wxID_HIGHEST + 1,
    COMBO_BASE,
    BUTTON_LOAD_IMAGE,

    PANEL_MAP,
    PANEL_ZOOM
};


BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE(wxCSL_EVT_PAINT,wxID_ANY)
END_DECLARE_EVENT_TYPES()

#define CSL_EVT_PAINT(id,fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
                               wxCSL_EVT_PAINT,id,wxID_ANY, \
                               (wxObjectEventFunction)(wxEventFunction) \
                               wxStaticCastEvent(wxCommandEventFunction,&fn), \
                               (wxObject*)NULL \
                             ),

DEFINE_EVENT_TYPE(wxCSL_EVT_PAINT)


BEGIN_EVENT_TABLE(CslMapCfgTool, wxDialog)
    EVT_CLOSE(CslMapCfgTool::OnClose)
    EVT_BUTTON(wxID_ANY, CslMapCfgTool::OnCommandEvent)
    EVT_CHOICE(wxID_ANY, CslMapCfgTool::OnCommandEvent)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(CslMapToolPanelMap, CslPanelMap)
    CSL_EVT_PAINT(wxID_ANY,CslMapToolPanelMap::OnPaintOverlay)
    EVT_PAINT(CslMapToolPanelMap::OnPaint)
    EVT_MOTION(CslMapToolPanelMap::OnMouseMove)
    EVT_LEFT_DOWN(CslMapToolPanelMap::OnMouseLeftDown)
    EVT_LEFT_UP(CslMapToolPanelMap::OnMouseLeftUp)
END_EVENT_TABLE()


void CslMapToolPanelMap::OnPaintOverlay(wxCommandEvent& event)
{
    wxClientDC dc(this);
    PrepareDC(dc);

    if (m_centre.x<0)
        m_centre=wxPoint(63,63);

    wxPen pen(*wxWHITE,2);
    pen.SetCap(wxCAP_BUTT);
    dc.SetPen(pen);
    dc.DrawLine(m_centre.x,m_centre.y-10,m_centre.x,m_centre.y-3); // N
    dc.DrawLine(m_centre.x,m_centre.y+10,m_centre.x,m_centre.y+3); // S
    dc.DrawLine(m_centre.x-10,m_centre.y,m_centre.x-3,m_centre.y); // W
    dc.DrawLine(m_centre.x+10,m_centre.y,m_centre.x+3,m_centre.y); // E
}

void CslMapToolPanelMap::OnPaint(wxPaintEvent& event)
{
    event.Skip();

    if (m_id!=PANEL_ZOOM)
        return;

    wxCommandEvent evt(wxCSL_EVT_PAINT);
    wxPostEvent(this,evt);
}

void CslMapToolPanelMap::OnMouseMove(wxMouseEvent &event)
{
    event.Skip();

    if (m_id!=PANEL_MAP || !(IsOk() && m_panelZoom))
        return;

    const wxPoint& origin=GetBitmapOrigin();
    wxInt32 w=m_bitmap.GetWidth();
    wxInt32 h=m_bitmap.GetHeight();
    wxPoint pos=event.GetPosition()-origin;
    wxPoint centre(63,63);
    wxInt32 i;

    if (pos.x-32<0)
    {
        centre.x=pos.x<=0 ? 1 : 2*pos.x;
        pos.x=32;
    }
    else if (pos.x+32>=w)
    {
        i=w-pos.x;
        centre.x=i<0 ? 127 : 127-2*i;
        pos.x=w-32;
    }
    if (pos.y-32<0)
    {
        centre.y=pos.y<=0 ? 1 : 2*pos.y;
        pos.y=32;
    }
    else if (pos.y+32>=h)
    {
        i=h-pos.y;
        centre.y=i<0 ? 127 : 127-2*i;
        pos.y=h-32;
    }

    wxImage zoom=m_bitmap.GetSubBitmap(wxRect(pos.x-32,pos.y-32,64,64)).ConvertToImage();
    zoom.Rescale(128,128);

    m_panelZoom->SetCentre(centre);
    m_panelZoom->SetMap(wxBitmap(zoom));

    if (m_mouseLeftDown)
        SetBasePosition(event.GetPosition());
}

void CslMapToolPanelMap::OnMouseLeftDown(wxMouseEvent &event)
{
    event.Skip();

    if (m_id!=PANEL_MAP || !IsOk() || m_activeBase<0)
        return;

    SetBasePosition(event.GetPosition());
    m_mouseLeftDown=true;

}

void CslMapToolPanelMap::OnMouseLeftUp(wxMouseEvent &event)
{
    event.Skip();

    if (m_id!=PANEL_MAP || !IsOk() || m_activeBase<0)
        return;

    m_mouseLeftDown=false;
}

void CslMapToolPanelMap::SetBasePosition(const wxPoint& point)
{
    wxInt32 w=m_bitmap.GetWidth();
    wxInt32 h=m_bitmap.GetHeight();
    wxPoint pos=point-GetBitmapOrigin();

    if (pos.x<0)
        return;
    else if (pos.x>=w)
        return;
    if (pos.y<0)
        return;
    else if (pos.y>=h)
        return;

    m_bases.Item(m_activeBase)->m_point=wxPoint(pos.x,pos.y);
    m_basesPtr->Item(m_activeBase)->m_point=wxPoint(pos.x,pos.y);

    Refresh(false);
}


CslMapCfgTool::CslMapCfgTool(wxWindow* parent,int id,const wxString& title,
                             const wxPoint& pos,const wxSize& size,long style):
        wxDialog(parent, id, title, pos, size, wxDEFAULT_DIALOG_STYLE)
{
    // begin wxGlade: CslMapCfgTool::CslMapCfgTool
    sizer_control_staticbox = new wxStaticBox(this, -1, wxEmptyString);
    sizer_map_staticbox = new wxStaticBox(this, -1, _("Map"));
    panel_bitmap = new CslMapToolPanelMap(this, PANEL_MAP);
    const wxString choice_version_choices[] =
    {
        _("0")
    };
    choice_version = new wxChoice(this, COMBO_VERSION, wxDefaultPosition, wxDefaultSize, 1, choice_version_choices, 0);
    button_version_add = new wxButton(this, wxID_ADD, _("Add"));
    button_version_del = new wxButton(this, wxID_REMOVE, _("Remove"));
    static_line_4 = new wxStaticLine(this, wxID_ANY);
    const wxString choice_base_choices[] =
    {
        _("0")
    };
    choice_base = new wxChoice(this, COMBO_BASE, wxDefaultPosition, wxDefaultSize, 1, choice_base_choices, 0);
    button_base_add = new wxButton(this, wxID_ADD, _("Add"));
    button_base_del = new wxButton(this, wxID_REMOVE, _("Remove"));
    static_line_1 = new wxStaticLine(this, wxID_ANY);
    text_ctrl_map_name = new wxTextCtrl(this, wxID_ANY, wxEmptyString);
    text_ctrl_author = new wxTextCtrl(this, wxID_ANY, wxEmptyString);
    static_line_2 = new wxStaticLine(this, wxID_ANY);
    panel_zoom = new CslMapToolPanelMap(this, PANEL_ZOOM);
    static_line_3 = new wxStaticLine(this, wxID_ANY);
    button_load_image = new wxButton(this, BUTTON_LOAD_IMAGE, _("Load &image"));
    button_load = new wxButton(this, wxID_OPEN, _("&Open"));
    button_save = new wxButton(this, wxID_SAVE, _("&Save"));
    button_clear = new wxButton(this, wxID_CLEAR, _("&Clear"));
    button_close = new wxButton(this, wxID_CLOSE, _("&Close"));

    set_properties();
    do_layout();
    // end wxGlade
}

void CslMapCfgTool::set_properties()
{
    // begin wxGlade: CslMapCfgTool::set_properties
    SetTitle(_("CSL - Map config tool"));
    panel_bitmap->SetMinSize(wxSize(400, 300));
    panel_bitmap->SetBackgroundColour(wxColour(0, 0, 0));
    choice_version->SetSelection(0);
    choice_base->SetSelection(0);
    panel_zoom->SetMinSize(wxSize(128, 128));
    panel_zoom->SetBackgroundColour(wxColour(0, 0, 0));
    button_close->SetDefault();
    // end wxGlade

    panel_bitmap->Init(panel_zoom,&m_mapInfo.m_bases);
    panel_bitmap->SetEraseBackGround();
    panel_zoom->SetEraseBackGround();

    choice_version->Clear();
    Reset();
}

void CslMapCfgTool::do_layout()
{
    // begin wxGlade: CslMapCfgTool::do_layout
    wxFlexGridSizer* grid_sizer_main = new wxFlexGridSizer(2, 1, 0, 0);
    wxFlexGridSizer* grid_sizer_top = new wxFlexGridSizer(1, 2, 0, 0);
    wxStaticBoxSizer* sizer_control = new wxStaticBoxSizer(sizer_control_staticbox, wxHORIZONTAL);
    wxFlexGridSizer* grid_sizer_control = new wxFlexGridSizer(10, 1, 0, 0);
    wxFlexGridSizer* grid_sizer_file = new wxFlexGridSizer(1, 3, 0, 0);
    wxFlexGridSizer* grid_sizer_desc = new wxFlexGridSizer(2, 2, 0, 0);
    wxFlexGridSizer* grid_sizer_base = new wxFlexGridSizer(1, 4, 0, 0);
    wxFlexGridSizer* grid_sizer_version = new wxFlexGridSizer(1, 4, 0, 0);
    wxStaticBoxSizer* sizer_map = new wxStaticBoxSizer(sizer_map_staticbox, wxHORIZONTAL);
    sizer_map->Add(panel_bitmap, 1, wxALL|wxEXPAND, 4);
    grid_sizer_top->Add(sizer_map, 1, wxRIGHT|wxEXPAND, 2);
    wxStaticText* label_version_static = new wxStaticText(this, wxID_ANY, _("Version:"));
    label_version_static->SetMinSize(wxSize(50, -1));
    grid_sizer_version->Add(label_version_static, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_version->Add(choice_version, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_version->Add(button_version_add, 0, wxALL, 4);
    grid_sizer_version->Add(button_version_del, 0, wxALL, 4);
    grid_sizer_control->Add(grid_sizer_version, 1, wxEXPAND, 0);
    grid_sizer_control->Add(static_line_4, 0, wxEXPAND, 0);
    wxStaticText* label_bases_static = new wxStaticText(this, wxID_ANY, _("Bases:"));
    label_bases_static->SetMinSize(wxSize(50, -1));
    grid_sizer_base->Add(label_bases_static, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_base->Add(choice_base, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_base->Add(button_base_add, 0, wxALL, 4);
    grid_sizer_base->Add(button_base_del, 0, wxALL, 4);
    grid_sizer_control->Add(grid_sizer_base, 1, wxEXPAND, 0);
    grid_sizer_control->Add(static_line_1, 0, wxEXPAND, 0);
    wxStaticText* label_map_name_static = new wxStaticText(this, wxID_ANY, _("Map name:"));
    grid_sizer_desc->Add(label_map_name_static, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_desc->Add(text_ctrl_map_name, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    wxStaticText* label_author_static = new wxStaticText(this, wxID_ANY, _("Author:"));
    grid_sizer_desc->Add(label_author_static, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_desc->Add(text_ctrl_author, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_desc->AddGrowableCol(1);
    grid_sizer_control->Add(grid_sizer_desc, 1, wxEXPAND, 0);
    grid_sizer_control->Add(static_line_2, 0, wxEXPAND, 0);
    grid_sizer_control->Add(panel_zoom, 1, wxALL|wxALIGN_CENTER_HORIZONTAL, 4);
    grid_sizer_control->Add(static_line_3, 0, wxEXPAND, 0);
    grid_sizer_file->Add(button_load_image, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_file->Add(button_load, 0, wxALL|wxEXPAND, 4);
    grid_sizer_file->Add(button_save, 0, wxALL|wxEXPAND, 4);
    grid_sizer_control->Add(grid_sizer_file, 1, wxALIGN_CENTER_HORIZONTAL, 0);
    grid_sizer_control->Add(button_clear, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 4);
    grid_sizer_control->AddGrowableCol(0);
    sizer_control->Add(grid_sizer_control, 1, wxALL|wxEXPAND, 4);
    grid_sizer_top->Add(sizer_control, 1, wxLEFT|wxEXPAND, 2);
    grid_sizer_top->AddGrowableRow(0);
    grid_sizer_top->AddGrowableCol(0);
    grid_sizer_main->Add(grid_sizer_top, 1, wxALL|wxEXPAND, 4);
    grid_sizer_main->Add(button_close, 0, wxALL|wxALIGN_RIGHT, 4);
    SetSizer(grid_sizer_main);
    grid_sizer_main->Fit(this);
    grid_sizer_main->AddGrowableRow(0);
    grid_sizer_main->AddGrowableCol(0);
    Layout();
    // end wxGlade
}

void CslMapCfgTool::OnClose(wxCloseEvent& WXUNUSED(event))
{
    Destroy();
}

void CslMapCfgTool::OnCommandEvent(wxCommandEvent& event)
{
    switch (event.GetId())
    {
        case wxID_CLOSE:
            Close();
            break;

        case wxID_OPEN:
            LoadConfig();
            break;

        case wxID_SAVE:
            ChoiceSetVersion(choice_version->GetSelection());
            SaveConfig();
            break;

        case COMBO_VERSION:
            ChoiceSetVersion(event.GetSelection());
            break;

        case COMBO_BASE:
            SetCurrentBase(event.GetSelection());
            break;

        case wxID_ADD:
            if (event.GetEventObject()==button_base_add)
                AddBase();
            else
                AddVersion();
            break;

        case wxID_REMOVE:
            if (event.GetEventObject()==button_base_del)
                DelBase();
            else
                DelVersion();
            break;

        case BUTTON_LOAD_IMAGE:
            LoadImage();
            break;

        case wxID_CLEAR:
            Reset();
            break;
    }
}

bool CslMapCfgTool::LoadImage()
{
    wxString s;

    if (wxDirExists(m_pngPath))
        s=m_pngPath;

    wxFileDialog dlg(this,_("Open image"),s,wxEmptyString,
                     _("PNG files (*.png)|*.png"),wxFD_FILE_MUST_EXIST);
    if (dlg.ShowModal()!=wxID_OK)
        return false;

    s=dlg.GetPath();
    m_pngPath=::wxPathOnly(s);
    m_fileName=dlg.GetFilename().BeforeLast(wxT('.'));

    wxBitmap bitmap;

    if (bitmap.LoadFile(s,wxBITMAP_TYPE_PNG))
    {
        if (bitmap.GetWidth()>400 || bitmap.GetHeight()>300)
        {
            wxMessageBox(_("The image should not be greater than 400x300"),
                         _("Error - Image too large!"),wxICON_ERROR,this);
            return false;
        }
        panel_zoom->SetOk(false,true);
        panel_bitmap->SetMap(bitmap);
        button_base_add->Enable(choice_version->GetCount()>0);

        sizer_map_staticbox->SetLabel(_("Map")+wxString(wxT(" (")+m_fileName+wxT(")")));
        panel_zoom->SetEraseBackGround(false);

        return true;
    }

    return false;
}

void CslMapCfgTool::LoadConfig()
{
    wxString s;

    if (wxDirExists(m_cfgPath))
        s=m_cfgPath;

    wxFileDialog dlg(this,_("Open config"),s,wxEmptyString,
                     _("Config files (*.cfg)|*.cfg"),wxFD_FILE_MUST_EXIST);
    if (dlg.ShowModal()!=wxID_OK)
        return;

    s=dlg.GetPath();
    m_cfgPath=::wxPathOnly(s);

    wxFileInputStream stream(s);
    if (!stream.IsOk())
        return;
    wxFileConfig config(stream);

    Reset(false);

    t_aInt32 versions;
    m_mapInfo.GetMapConfigVersions(config,versions);

    for (wxUint32 i=0;i<versions.GetCount();i++)
    {
        CslMapInfo *info=new CslMapInfo();

        if (!info->LoadMapConfig(config,versions.Item(i)))
        {
            delete info;
            continue;
        }

        choice_version->Append(wxString::Format(wxT("%d"),info->m_version));
        choice_version->SetClientData(choice_version->GetCount()-1,(void*)info);
    }

    if (!choice_version->GetCount())
        return;

    button_version_del->Enable();
    button_save->Enable();
    text_ctrl_author->Enable();
    text_ctrl_map_name->Enable();

    choice_version->SetSelection(0);
    ChoiceSetVersion(0);
}

void CslMapCfgTool::SaveConfig()
{
    wxString path=m_cfgPath;

    wxFileDialog dlg(this,_("Save config"),wxEmptyString,
                     m_fileName+wxT(".cfg"),_("Config files (*.cfg)|*.cfg"),
#if wxCHECK_VERSION(2,9,0)
                     wxFD_SAVE|wxFD_OVERWRITE_PROMPT
#else
                     wxSAVE|wxOVERWRITE_PROMPT
#endif
                    );
    // wxGTK: hmm, doesn't work in the ctor?!
    if (path.IsEmpty())
        path=m_pngPath;
    if (wxDirExists(path))
        dlg.SetPath(path+wxT("/")+m_fileName);
    if (dlg.ShowModal()!=wxID_OK)
        return;

    path=dlg.GetPath();
    m_cfgPath=::wxPathOnly(path);

    wxFileConfig config;
    wxUint32 i,j;

    for (i=0;i<choice_version->GetCount();i++)
    {
        CslMapInfo *info=(CslMapInfo*)choice_version->GetClientData(i);

        config.SetPath(wxString::Format(wxT("/%d"),info->m_version));
        config.Write(wxT("Mapname"),info->m_mapNameFull);
        config.Write(wxT("Author"),info->m_author);

        for (j=0;j<info->m_bases.GetCount();j++)
        {
            config.Write(wxString::Format(wxT("x%d"),j),info->m_bases.Item(j)->m_point.x);
            config.Write(wxString::Format(wxT("y%d"),j),info->m_bases.Item(j)->m_point.y);
        }
    }

    if (!i)
        return;

    wxFileOutputStream stream(path);
    config.Save(stream);
}

void CslMapCfgTool::AddVersion()
{
    CslMapInfo *info;
    wxUint32 i,c,h;
    wxInt32 version=::wxGetNumberFromUser(_("Enter new version number"),_("Version: "),
                                          _("New version number"),256,0,9999,this);
    if (version==-1)
        return;

    c=choice_version->GetCount();
    h=c;
    for (i=0;i<c;i++)
    {
        info=(CslMapInfo*)choice_version->GetClientData(i);
        if (info->m_version==version)
        {
            wxMessageBox(_("This version already exits!"),
                         _("Error"),wxICON_ERROR,this);
            return;
        }
        if (version<info->m_version)
            h=i;
    }

    info=new CslMapInfo(wxEmptyString,version);

    choice_version->Insert(wxString::Format(wxT("%d"),info->m_version),h);
    choice_version->SetClientData(h,(void*)info);
    choice_version->SetSelection(h);
    ChoiceSetVersion(h);

    button_base_add->Enable(panel_bitmap->IsOk());
    button_version_del->Enable();
    button_save->Enable();
    text_ctrl_author->Enable();
    text_ctrl_map_name->Enable();
}

void CslMapCfgTool::DelVersion()
{
    wxInt32 i=choice_version->GetSelection();

    CslMapInfo *info=(CslMapInfo*)choice_version->GetClientData(i);
    delete info;

    choice_version->Delete(i);

    wxInt32 c=choice_version->GetCount();
    if (!c)
    {
        Reset(false);
        /*button_version_del->Enable(false);
        button_save->Enable(false);
        text_ctrl_author->Enable(false);
        text_ctrl_map_name->Enable(false);*/
    }
    else
        choice_version->SetSelection(0);

    m_lastInfo=-1;
    ChoiceSetVersion(c>0 ? 0 : -1);
}

void CslMapCfgTool::AddBase()
{
    wxInt32 i=m_mapInfo.m_bases.GetCount();
    CslBaseInfo *base=new CslBaseInfo(4,4);

    m_mapInfo.m_bases.Add(base);
    choice_base->Append(wxString::Format(wxT("%d"),i));
    choice_base->SetSelection(i);
    SetCurrentBase(i);

    button_base_del->Enable();
}

void CslMapCfgTool::DelBase()
{
    wxInt32 i=choice_base->GetSelection();

    delete m_mapInfo.m_bases.Item(i);
    m_mapInfo.m_bases.RemoveAt(i);
    choice_base->Clear();
    ChoiceFillBases();

    button_base_del->Enable(choice_base->GetCount()>0);
}

void CslMapCfgTool::SetCurrentBase(const wxInt32 id)
{
    m_mapInfo.ResetBasesColour();

    if (id>-1)
        m_mapInfo.m_bases.Item(id)->m_colour=*wxRED;
    panel_bitmap->SetActiveBase(id);
    panel_bitmap->UpdateBases(m_mapInfo.m_bases,true);
}

void CslMapCfgTool::ChoiceSetVersion(const wxInt32 id)
{
    m_mapInfo.m_mapNameFull=text_ctrl_map_name->GetValue();
    m_mapInfo.m_author=text_ctrl_author->GetValue();

    if (m_lastInfo>-1)
    {
        CslMapInfo *info=(CslMapInfo*)choice_version->GetClientData(m_lastInfo);
        *info=m_mapInfo;
    }
    m_lastInfo=id;

    if (id>-1)
    {
        m_mapInfo=*(CslMapInfo*)choice_version->GetClientData(id);

        text_ctrl_map_name->SetValue(m_mapInfo.m_mapNameFull);
        text_ctrl_author->SetValue(m_mapInfo.m_author);
    }
    else
    {
        text_ctrl_map_name->Clear();
        text_ctrl_author->Clear();
    }
    ChoiceFillBases();
}

void CslMapCfgTool::ChoiceFillBases()
{
    wxInt32 i,c;

    choice_base->Clear();

    c=m_mapInfo.m_bases.GetCount();
    for (i=0;i<c;i++)
        choice_base->Append(wxString::Format(wxT("%d"),i));
    if (i)
        choice_base->SetSelection(0);

    SetCurrentBase(i>0 ? 0 : -1);
    button_base_del->Enable(i>0);
}

void CslMapCfgTool::Reset(const bool resetMap)
{
    wxUint32 i,c=choice_version->GetCount();

    for (i=0;i<c;i++)
    {
        CslMapInfo *info=(CslMapInfo*)choice_version->GetClientData(i);
        delete info;
    }

    m_lastInfo=-1;
    button_save->Enable(false);
    button_version_del->Enable(false);
    button_base_add->Enable(false);
    button_base_del->Enable(false);
    text_ctrl_author->Clear();
    text_ctrl_map_name->Clear();
    text_ctrl_author->Enable(false);
    text_ctrl_map_name->Enable(false);
    choice_version->Clear();
    choice_base->Clear();
    sizer_map_staticbox->SetLabel(_("Map"));
    if (!resetMap)
        return;
    m_mapInfo.Reset();
    panel_bitmap->Reset();
    panel_zoom->SetEraseBackGround();
    panel_zoom->SetCentre(wxPoint(-1,-1));
    panel_zoom->Reset();
}


IMPLEMENT_APP(CslMapCfgToolApp)

bool CslMapCfgToolApp::OnInit()
{
    wxInitAllImageHandlers();
    CslMapCfgTool* dlg_cslmapcfgtool=new CslMapCfgTool(NULL,wxID_ANY,wxEmptyString);
    SetTopWindow(dlg_cslmapcfgtool);
    dlg_cslmapcfgtool->Show();
    return true;
}
