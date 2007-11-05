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

#include "CslListCtrlInfo.h"
#include "CslSettings.h"
#include "CslFlags.h"
#include "CslTools.h"
#include "CslGeoIP.h"
#include "img/info_18_12.xpm"

BEGIN_EVENT_TABLE(CslListCtrlInfo, wxListCtrl)
    EVT_SIZE(CslListCtrlInfo::OnSize)
END_EVENT_TABLE()


CslListCtrlInfo::CslListCtrlInfo(wxWindow* parent,wxWindowID id,const wxPoint& pos,
                                 const wxSize& size,long style,
                                 const wxValidator& validator, const wxString& name)
        : wxListCtrl(parent,id,pos,size,style,validator,name)
{
#ifdef __WXMSW__
    m_imgList.Create(23,12,true);
    m_imgList.Add(AdjustIconSize(info_18_12_xpm,wxNullIcon,wxSize(23,12),wxPoint(2,0)));
    m_imgList.Add(AdjustIconSize(unknown_xpm,wxNullIcon,wxSize(23,12),wxPoint(5,0)));
#else
    m_imgList.Create(18,12,true);
    m_imgList.Add(wxBitmap(info_18_12_xpm));
    m_imgList.Add(wxBitmap(unknown_xpm));
#endif

    SetImageList(&m_imgList,wxIMAGE_LIST_SMALL);

    wxListItem item;
    item.SetMask(wxLIST_MASK_TEXT);

    item.SetText(wxT(""));
    InsertColumn(0,item);
    InsertColumn(1,item);

    wxInt32 i=0;
    InsertItem(i++,_("Address"),0);
    InsertItem(i++,_("Location"),0);
    InsertItem(i++,_("Game"),0);
    InsertItem(i++,_("Protocol version"),0);
    InsertItem(i++,_("Uptime"),0);
    InsertItem(i++,_("Last seen"),0);
    InsertItem(i++,_("Last played"),0);
    InsertItem(i++,_("Last play time"),0);
    InsertItem(i++,_("Total play time"),0);
    InsertItem(i,_("Connects"),0);

    for (;i>=0;i-=2)
    {
        item.SetId(i);
        SetItemBackgroundColour(item,g_cslSettings->m_colInfoStripe);
    }

    AdjustSize(GetClientSize());
}

void CslListCtrlInfo::OnSize(wxSizeEvent& event)
{
    AdjustSize(event.GetSize());
    event.Skip();
}

void CslListCtrlInfo::AdjustSize(wxSize size)
{
#if 0 //#ifndef __WXMSW__
    wxInt32 w=size.x-4;
#else
    wxInt32 w=size.x;
#endif // __WXMSW__

    SetColumnWidth(0,(wxInt32)(w*0.35));
    SetColumnWidth(1,(wxInt32)(w*0.64));
}

void CslListCtrlInfo::UpdateInfo(CslServerInfo *info)
{
    if (!info)
    {
        wxASSERT_MSG(info,wxT("invalid info"));
        return;
    }
#define CSL_UNKNOWN_STR _("unknown")

    wxDateTime dt;
    wxString s;
    wxInt32 i,ic=0;
    const char *country;
    const char **flag=NULL;
    char *host=strdup(U2A(info->m_addr.IPAddress().c_str()));

    s=info->m_host;
    if (IsIP(info->m_host))
    {
        wxString h=info->m_domain;
        if (!h.IsEmpty())
            s+=wxT(" (")+h+wxT(")");
    }
    SetItem(ic++,1,s);

    if (CslGeoIP::IsOk())
    {
        country=CslGeoIP::GetCountryCodeByAddr(host);
        if (country)
        {
            i=sizeof(codes)/sizeof(codes[0])-1;
            for (;i>=0;i--)
            {
                if (!strcasecmp(country,codes[i]))
                {
                    flag=flags[i];
                    break;
                }
            }
            s=A2U(country);
            country=CslGeoIP::GetCountryNameByAddr(host);
            if (country)
                s+=wxT(" (")+A2U(country)+wxT(")");
        }
        else
            s=wxEmptyString;
    }
    else
        s=_("GeoIP database not found.");

    free(host);

    if (!flag)
        flag=unknown_xpm;
#ifdef __WXMSW__
    m_imgList.Replace(1,AdjustIconSize(flag,wxNullIcon,wxSize(23,12),wxPoint(5,0)));
#else
    m_imgList.Replace(1,wxBitmap(flag));
#endif
    SetItem(ic++,1,s,1);

    switch (info->m_type)
    {
        case CSL_GAME_SB:
            s=CSL_DEFAULT_NAME_SB;
            break;
        case CSL_GAME_AC:
            s=CSL_DEFAULT_NAME_AC;
            break;
        case CSL_GAME_CB:
            s=CSL_DEFAULT_NAME_CB;
            break;
        default:
            s=CSL_UNKNOWN_STR;
    }
    SetItem(ic++,1,s);

    if (info->m_protocol<0)
        s=CSL_UNKNOWN_STR;
    else
        s=s.Format(wxT("%d"),info->m_protocol);
    SetItem(ic++,1,s);

    if (info->m_extInfoStatus!=CSL_EXT_STATUS_FALSE)
    {
        if (info->m_extInfoStatus==CSL_EXT_STATUS_OK)
            s=FormatSeconds(info->m_uptime);
        else
            s=_("This version of extended info is not supported.");
    }
    else
        s=_("Extended info not supported.");
    SetItem(ic++,1,s);

    if (info->m_lastSeen)
    {
        dt.Set((time_t)info->m_lastSeen);
        s=dt.Format();
    }
    else
        s=CSL_UNKNOWN_STR;
    SetItem(ic++,1,s);

    if (info->m_playLast)
    {
        dt.Set((time_t)info->m_playLast);
        s=dt.Format();
    }
    else
        s=CSL_UNKNOWN_STR;
    SetItem(ic++,1,s);

    if (info->m_playTimeLastGame)
        s=FormatSeconds(info->m_playTimeLastGame);
    else
        s=wxT("0");
    SetItem(ic++,1,s);

    if (info->m_playTimeTotal)
        s=FormatSeconds(info->m_playTimeTotal);
    else
        s=wxT("0");
    SetItem(ic++,1,s);

    SetItem(ic++,1,wxString::Format(wxT("%d"),info->m_connectedTimes));
}
