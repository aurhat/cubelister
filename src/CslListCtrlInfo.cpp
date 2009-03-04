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

#include "CslListCtrlInfo.h"
#include "engine/CslTools.h"
#include "CslSettings.h"
#include "CslFlags.h"
#include "CslGeoIP.h"
#include "img/info_18_12.xpm"

BEGIN_EVENT_TABLE(CslListCtrlInfo,CslListCtrl)
    EVT_SIZE(CslListCtrlInfo::OnSize)
END_EVENT_TABLE()


CslListCtrlInfo::CslListCtrlInfo(wxWindow* parent,wxWindowID id,const wxPoint& pos,
                                 const wxSize& size,long style,
                                 const wxValidator& validator, const wxString& name) :
        CslListCtrl(parent,id,pos,size,style,validator,name)
{
    FlickerFree(false);

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

    wxInt32 i=0;
    wxListItem item;

    item.SetMask(wxLIST_MASK_TEXT);
    item.SetText(wxT(""));
    InsertColumn(0,item);
    InsertColumn(1,item);

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
        SetItemBackgroundColour(item,g_cslSettings->colInfoStripe);
    }

    AdjustSize(GetClientSize());
}

void CslListCtrlInfo::OnSize(wxSizeEvent& event)
{
#ifndef __WXMSW__
    if (event.GetEventObject())
    {
        AdjustSize(event.GetSize());

        wxSizeEvent evt;
        wxPostEvent(this,evt);

        event.Skip();
    }
    else
    {
#ifdef __WXGTK__
        ((wxScrolledWindow*)m_mainWin)->SetScrollbars(0,0,0,0);
#else
        ((wxScrolledWindow*)m_genericImpl->m_mainWin)->SetScrollbars(0,0,0,0);
#endif //__WXGTK__
    }
#else
    AdjustSize(event.GetSize());
    event.Skip();
#endif //__WXMSW__
}

void CslListCtrlInfo::AdjustSize(wxSize size)
{
    SetColumnWidth(0,(wxInt32)(size.x*0.35));
    SetColumnWidth(1,(wxInt32)(size.x*0.65));
}

void CslListCtrlInfo::GetToolTipText(wxInt32 WXUNUSED(row),CslToolTipEvent& event)
{
    wxInt32 i;
    wxString attr,val;
    wxListItem item;

    item.SetMask(wxLIST_MASK_TEXT);

    for (i=0;i<GetItemCount();i++)
    {
        item.SetId(i);
        item.SetColumn(1);
        GetItem(item);
        val=item.GetText();

        if (val.IsEmpty())
            continue;

        item.SetColumn(0);
        GetItem(item);
        attr=item.GetText();

        event.Text.Add(attr);
        event.Text.Add(val);
    }

    event.Title=_("Server information");
}

void CslListCtrlInfo::UpdateInfo(CslServerInfo *info)
{
    if (!info)
        return;

    wxDateTime dt;
    wxString s;
    wxInt32 i,ic=0;
    const char *country;
    const char **flag=NULL;
    char *host=strdup(U2A(info->Addr.IPAddress().c_str()));

    s=wxString::Format(wxT("%s:%d"),info->Host.c_str(),info->GamePort);
    if (IsIP(info->Host))
    {
        if (!info->Domain.IsEmpty())
            s+=wxT(" (")+info->Domain+wxT(")");
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

    SetItem(ic++,1,info->GetGame().GetName());

    if (info->Protocol>=0)
    {
        s=wxString::Format(wxT("%d"),info->Protocol);
        if (info->ExtInfoStatus!=CSL_EXT_STATUS_FALSE)
            s+=wxString::Format(wxT(" / %d"),info->ExtInfoVersion);
    }
    else
        s=_("unknown");
    SetItem(ic++,1,s);

    if (info->ExtInfoStatus!=CSL_EXT_STATUS_FALSE)
    {
        if (info->ExtInfoStatus==CSL_EXT_STATUS_OK)
            s=FormatSeconds(info->Uptime);
        else
            s=_("This version of extended info is not supported.");
    }
    else
        s=_("Extended info not supported.");
    SetItem(ic++,1,s);

    if (info->LastSeen)
    {
        dt.Set((time_t)info->LastSeen);
        s=dt.Format();
    }
    else
        s=_("unknown");
    SetItem(ic++,1,s);

    if (info->PlayedLast)
    {
        dt.Set((time_t)info->PlayedLast);
        s=dt.Format();
    }
    else
        s=_("unknown");
    SetItem(ic++,1,s);

    if (info->PlayTimeLast)
        s=FormatSeconds(info->PlayTimeLast);
    else
        s=wxT("0");
    SetItem(ic++,1,s);

    if (info->PlayTimeTotal)
        s=FormatSeconds(info->PlayTimeTotal);
    else
        s=wxT("0");
    SetItem(ic++,1,s);

    SetItem(ic++,1,wxString::Format(wxT("%d"),info->ConnectedTimes));
}
