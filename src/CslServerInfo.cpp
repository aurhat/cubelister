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

#include "Csl.h"
#include "CslEngine.h"
#include "CslApp.h"
#include "CslGame.h"
#include "CslGeoIP.h"
#include "CslServerInfo.h"
#include "CslSettings.h"

BEGIN_EVENT_TABLE(CslListCtrlInfo,CslListCtrl)
    EVT_SIZE(CslListCtrlInfo::OnSize)
    CSL_EVT_DNS_RESOLVE(CslListCtrlInfo::OnResolve)
END_EVENT_TABLE()


CslListCtrlInfo::CslListCtrlInfo(wxWindow* parent,wxWindowID id,const wxPoint& pos,
                                 const wxSize& size,long style,
                                 const wxValidator& validator, const wxString& name) :
        CslListCtrl(parent,id,pos,size,style,validator,name),
        m_info(NULL), m_resolving(false)
{
    wxInt32 i=0;
    wxListItem item;

    ListAddColumn(wxEmptyString, wxLIST_FORMAT_LEFT, 0.35f, true, true);
    ListAddColumn(wxEmptyString, wxLIST_FORMAT_LEFT, 0.65f, true, true);

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
    InsertItem(i++, _("Connects"), CSL_LIST_IMG_INFO);

    SetClientSize(GetBestSize());

    for (i -= 1; i>=0; i -= 2)
    {
        item.SetId(i);
        SetItemBackgroundColour(item, CslGetSettings().ColInfoStripe);
    }

    CslEngine *engine = ::wxGetApp().GetCslEngine();

    engine->Connect(CslPongEvent::PONG, wxCSL_EVT_PONG,
                    CslPongEventHandler(CslListCtrlInfo::OnPong),
                    NULL, this);
}

CslListCtrlInfo::~CslListCtrlInfo()
{
    CslEngine *engine = ::wxGetApp().GetCslEngine();

    engine->Disconnect(CslPongEvent::PONG, wxCSL_EVT_PONG,
                       CslPongEventHandler(CslListCtrlInfo::OnPong),
                       NULL, this);
}

void CslListCtrlInfo::UpdateServer(CslServerInfo *info, bool noresolve)
{
    m_info = info;

    if (!info)
    {
        Reset();
        return;
    }

    wxString s;
    wxDateTime dt;
    wxInt32 i, ic = 0;
    const CslIPV4Addr& addr = info->Address();

    static const wxString GET_IP     = _("Getting IP...");
    static const wxString GET_DOMAIN = _("Getting domain...");
    static const wxString NO_IP      = _("No IP");
    static const wxString NO_DOMAIN  = _("No domain");

    if (!noresolve)
        Resolve();

    wxWindowUpdateLocker lock(this);

    if (IsIPV4(addr))
    {
        s = addr.GetIPString() << wxString::Format(wxT(":%d"), info->GamePort);
        s << wxT(" (")
          << (info->Domain.IsEmpty() ? (m_resolving ? GET_DOMAIN : NO_DOMAIN) : info->Domain)
          << wxT(")");
    }
    else
    {
        s = wxString::Format(wxT("%s:%d"), info->Host.c_str(), info->GamePort);
        s << wxT(" (") << (m_resolving ? GET_IP : NO_IP) << wxT(")");
    }

    SetItem(ic++, 1, s);

    if (!CslGeoIP::IsOk())
        s = _("GeoIP database not loaded.");
    else if (IsLocalIPV4(addr))
        s = _("local network");
    else
    {
        s = CslGeoIP::GetCountryNameByIPnum(addr.GetIP());
        wxString city = CslGeoIP::GetCityNameByIPnum(addr.GetIP());

        if (!s.IsEmpty())
        {
            if (!city.IsEmpty())
                s<<wxT(" (")<<city<<wxT(")");
        }
        else
            s = _("Unknown location.");
    }
    SetItem(ic++, 1, s, GetCountryFlag(::wxGetApp().GetCslEngine(), addr.GetIP()));

    SetItem(ic++, 1, info->GetGame().GetName());

    if (info->Protocol>=0)
    {
        s = wxString::Format(wxT("%d"),info->Protocol);
        if (info->ExtInfoStatus!=CSL_EXT_STATUS_FALSE)
            s += wxString::Format(wxT(" / %d"),info->ExtInfoVersion);
    }
    else
        s = _("unknown");
    SetItem(ic++, 1, s);

    if (info->ExtInfoStatus!=CSL_EXT_STATUS_FALSE)
    {
        if (info->ExtInfoStatus==CSL_EXT_STATUS_OK)
            s = FormatSeconds(info->Uptime);
        else if (info->Protocol<CSL_EX_VERSION_MIN || info->Protocol>=CSL_EX_VERSION_MAX)
            s = _("This version of extended info is not supported.");
        else
            s = _("Broken protocol.");
    }
    else
        s = _("Extended info not supported.");
    SetItem(ic++, 1, s);

    if (!info->InfoText.IsEmpty())
    {
        wxInt32 l = i = info->InfoText.Length();

        if (l>50)
        {
            if ((i = info->InfoText.find_first_of(wxT(" \t\v\r\n"), 50))==wxNOT_FOUND)
                i = 50;
        }

        s = info->InfoText.Mid(0, i)+(i<l ? wxT(" ...") : wxT(""));
        s.Replace(wxT("\r\n"), wxT(" "));
        s.Replace(wxT("\n"), wxT(" "));
    }
    else
        s = _("Server has no info message.");
    SetItem(ic++, 1, s);

    if (info->LastSeen)
    {
        dt.Set((time_t)info->LastSeen);
        s = dt.Format();
    }
    else
        s = _("unknown");
    SetItem(ic++, 1, s);

    if (info->PlayedLast)
    {
        dt.Set((time_t)info->PlayedLast);
        s = dt.Format();
    }
    else
        s = _("unknown");
    SetItem(ic++, 1, s);

    if (info->PlayTimeLast)
        s = FormatSeconds(info->PlayTimeLast);
    else
        s = wxT("0");
    SetItem(ic++, 1, s);

    if (info->PlayTimeTotal)
        s = FormatSeconds(info->PlayTimeTotal);
    else
        s = wxT("0");
    SetItem(ic++, 1, s);

    SetItem(ic++, 1, wxString::Format(wxT("%d"), info->ConnectedTimes));
}

void CslListCtrlInfo::Reset()
{
    loopi(GetItemCount())
        SetItem(i, 1, wxT(""), -1);
}

void CslListCtrlInfo::Resolve()
{
    if (m_info->Domain.IsEmpty() || !m_info->Pingable())
    {
        m_resolving = true;

        ::wxGetApp().GetCslEngine()->DNSResolve(m_info->Host, m_info, this);
    }
}

void CslListCtrlInfo::GetToolTipText(wxInt32 WXUNUSED(row), CslToolTipEvent& event)
{
    wxString attr, val;
    wxListItem item;

    item.SetMask(wxLIST_MASK_TEXT);

    loopi(GetItemCount())
    {
        item.SetId(i);
        item.SetColumn(1);
        GetItem(item);
        val = item.GetText();

        if (val.IsEmpty())
            continue;

        item.SetColumn(0);
        GetItem(item);
        attr = item.GetText();

        event.Text.Add(attr);
        event.Text.Add(val);
    }

    event.Title = _("Server information");

    if (m_info && !m_resolving && (!m_info->Domain.IsEmpty() || m_info->Pingable()))
    {
        wxString url = wxT("http://") +
                       (m_info->Domain.IsEmpty() ? m_info->Address().GetIPString() : m_info->Domain);

        wxHyperlinkCtrl *link = new wxHyperlinkCtrl(event.Parent, wxID_ANY, _("Go to servers website"), url);

        wxFont font = link->GetFont();
        font.SetWeight(wxFONTWEIGHT_BOLD);

        link->SetFont(font);
        link->SetBackgroundColour(CSL_SYSCOLOUR(wxSYS_COLOUR_INFOBK));

        event.UserWindow = link;
    }
}

wxSize CslListCtrlInfo::DoGetBestSize() const
{
    wxRect rect;
    wxSize size;

    GetItemRect(0, rect);
    size = wxSize(240, rect.GetHeight() * GetItemCount());
#if !defined(__WXMSW__) &&  !wxCHECK_VERSION(2, 9, 0)
     size.y += 4;
#endif

    return size;
}

void CslListCtrlInfo::OnSize(wxSizeEvent& event)
{
    if (event.GetEventObject())
    {
#if !defined(__WXMSW__) && !wxCHECK_VERSION(2, 9, 0)
        // post another event to remove the scrollbars
        wxSizeEvent evt;
        AddPendingEvent(evt);
#else
        ScrollList(0, 0);
#endif
        event.Skip();
    }
    else
    {
#if !defined(__WXMSW__) && !wxCHECK_VERSION(2, 9, 0)
        // remove the scrollbars
        wxWindow *wnd = GetChildWindowByClassInfo(this, CLASSINFO(wxScrolledWindow));

        if (wnd)
            ((wxScrolledWindow*)wnd)->SetScrollbars(0, 0, 0, 0);
#endif
    }
}

void CslListCtrlInfo::OnPong(CslPongEvent& event)
{
    if (event.GetServerInfo()==m_info)
        UpdateServer(m_info, true);

    event.Skip();
}

void CslListCtrlInfo::OnResolve(CslDNSResolveEvent& event)
{
       if (event.GetClientData()==m_info)
    {
        m_resolving = false;
        m_info->Domain = event.GetDomain();
        UpdateServer(m_info, true);
    }
}
