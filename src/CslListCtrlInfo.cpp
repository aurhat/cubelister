/***************************************************************************
 *   Copyright (C) 2007-2011 by Glen Masgai                                *
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
#include "CslGame.h"
#include "CslSettings.h"
#include "CslGeoIP.h"
#include "CslListCtrlInfo.h"

BEGIN_EVENT_TABLE(CslListCtrlInfo,CslListCtrl)
    EVT_SIZE(CslListCtrlInfo::OnSize)
END_EVENT_TABLE()


CslListCtrlInfo::CslListCtrlInfo(wxWindow* parent,wxWindowID id,const wxPoint& pos,
                                 const wxSize& size,long style,
                                 const wxValidator& validator, const wxString& name) :
        CslListCtrl(parent,id,pos,size,style,validator,name)
{
    wxInt32 i=0;
    wxListItem item;

    item.SetMask(wxLIST_MASK_TEXT);
    item.SetText(wxT(""));
    InsertColumn(0,item);
    InsertColumn(1,item);

    InsertItem(i++, _("Address"), CSL_LIST_IMG_INFO);
    InsertItem(i++, _("Location"), CSL_LIST_IMG_INFO);
    InsertItem(i++, _("Game"), CSL_LIST_IMG_INFO);
    InsertItem(i++, _("Protocol version"), CSL_LIST_IMG_INFO);
    InsertItem(i++, _("Uptime"), CSL_LIST_IMG_INFO);
    InsertItem(i++, _("Server message"), CSL_LIST_IMG_INFO);
    InsertItem(i++, _("Last seen"), CSL_LIST_IMG_INFO);
    InsertItem(i++, _("Last played"), CSL_LIST_IMG_INFO);
    InsertItem(i++, _("Last play time"), CSL_LIST_IMG_INFO);
    InsertItem(i++, _("Total play time"), CSL_LIST_IMG_INFO);
    InsertItem(i, _("Connects"), CSL_LIST_IMG_INFO);

    for (;i>=0;i-=2)
    {
        item.SetId(i);
        SetItemBackgroundColour(item, CslGetSettings().ColInfoStripe);
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
#if defined(__WXGTK__) || defined(__WXX11__)
        //((wxScrolledWindow*)m_mainWin)->SetScrollbars(0, 0, 0, 0);
        wxWindow *window=GetChildWindowByClassInfo(this, CLASSINFO(wxScrolledWindow));
        if (window)
            ((wxScrolledWindow*)window)->SetScrollbars(0, 0, 0, 0);
#else
        ((wxScrolledWindow*)m_genericImpl->m_mainWin)->SetScrollbars(0,0,0,0);
#endif //defined(__WXGTK__) || defined(__WXX11__)
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

    wxWindowUpdateLocker lock(this);

    wxString s, cn;
    wxDateTime dt;
    wxInt32 i,ic=0;
    CslIPV4Addr addr(U2A(info->Addr.IPAddress()));

    s=wxString::Format(wxT("%s:%d"),info->Host.c_str(),info->GamePort);
    if (IsIP(info->Host))
    {
        if (!info->Domain.IsEmpty())
            s<<wxT(" (")<<info->Domain+wxT(")");
    }
    SetItem(ic++,1,s);

    if (!CslGeoIP::IsOk())
        s=_("GeoIP database not loaded.");
    else if (IsLocalIP(addr))
        s=_("local network");
    else
    {
        s=CslGeoIP::GetCountryNameByIPnum(addr.GetIP());
        wxString city=CslGeoIP::GetCityNameByIPnum(addr.GetIP());

        if (!s.IsEmpty())
        {
            if (!city.IsEmpty())
                s<<wxT(" (")<<city<<wxT(")");
        }
        else
            s=_("Unknown location.");
            }
    SetItem(ic++, 1, s, GetCountryFlag(addr.GetIP()));

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

    if (!info->InfoText.IsEmpty())
    {
        wxInt32 l=i=info->InfoText.Length();
        if (l>50)
        {
            if ((i=info->InfoText.find_first_of(wxT(" \t\v\r\n"),50))==wxNOT_FOUND)
                i=50;
        }
        s=info->InfoText.Mid(0,i)+(i<l ? wxT(" ...") : wxT(""));
        s.Replace(wxT("\r\n"),wxT(" "));
    }
    else
        s=_("Server has no info message.");
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
