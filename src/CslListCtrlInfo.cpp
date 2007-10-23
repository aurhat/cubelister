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
#include "CslTools.h"
#include "CslFlags.h"
#ifdef __WXMSW__
#include "img/info_23_12.xpm"
#else
#include "img/info_18_12.xpm"
#endif

BEGIN_EVENT_TABLE(CslListCtrlInfo, wxListCtrl)
    EVT_SIZE(CslListCtrlInfo::OnSize)
END_EVENT_TABLE()

#ifdef _MSC_VER
#define strcasecmp _stricmp
#else
#endif

#if 0
void ShiftrFlags(int offset)
{
    int j=sizeof(flags)/sizeof(flags[0])-1;

    for (;j>=0;j--)
    {
        char **xpm=flags[j];
        int bs=strlen(xpm[0]);
        int w=0,h=0,c=0,a=0;
        int sc=0;
        int i=0,k;
        char b;
        char *buf;
        char cc[5]={0};

        while (i<bs)
        {
            b=xpm[0][i];
            i++;
            if (sc==4) { break; }
            if (b==' ') { sc++; continue; }
            if (sc==0) { w=w*10+(b-'0'); }
            if (sc==1) { h=h*10+(b-'0'); }
            if (sc==2) { c=c*10+(b-'0'); }
            if (sc==3) { a=a*10+(b-'0'); }
        }

        for (i=1;i<=c;i++)
        {
            if (!strstr(xpm[i],"None"))
                continue;

            k=0;
            bs=strlen(xpm[i]);
            while (k<bs)
            {
                b=xpm[i][k];
                cc[k]=b;
                if (b==' ') break;
                k++;
            }
            break;
        }
        if (k==0 && k<a)
            memset(cc+1,cc[0],3);

        buf=(char*)malloc(bs+1);
        sprintf(buf,"%d %d %d %d",w+offset,h,c,a);
        xpm[0]=buf;

        for (i=1;i<=h;i++)
        {
            buf=(char*)malloc((w+offset)*a);
            memcpy(buf+offset*a,xpm[c+i],w*a);
            if (a==1) memset(buf,cc[0],offset);
            else
            {
                for (k=0;k<offset;k++)
                    memcpy(buf+a*k,cc,a);
            }
            xpm[c+i]=buf;
            //printf("%s\n",xpm[c+i]);
        }
    }
}
#endif

CslListCtrlInfo::CslListCtrlInfo(wxWindow* parent,wxWindowID id,const wxPoint& pos,
                                 const wxSize& size,long style,
                                 const wxValidator& validator, const wxString& name)
        : wxListCtrl(parent,id,pos,size,style,validator,name)
{
    wxString path=wxT(DATADIR)+wxString(wxT("/GeoIP.dat"));
    m_geoIP=GeoIP_open(U2A(path),GEOIP_MEMORY_CACHE);
#ifdef __WXGTK__
    if (!m_geoIP)
    {
        path=::g_basePath+wxT("/data/GeoIP.dat");
        m_geoIP=GeoIP_open(U2A(path),GEOIP_MEMORY_CACHE);
    }
#endif

#ifdef __WXMSW__
    m_imgList.Create(23,12,true);
    m_imgList.Add(wxBitmap(info_23_12_xpm));
    //ShiftrFlags(5);
#else
    m_imgList.Create(18,12,true);
    m_imgList.Add(wxBitmap(info_18_12_xpm));
#endif

    m_imgList.Add(wxBitmap(unknown_xpm));
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

CslListCtrlInfo::~CslListCtrlInfo()
{
    if (m_geoIP)
        GeoIP_delete(m_geoIP);
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

    if (m_geoIP)
    {
        country=GeoIP_country_code_by_addr(m_geoIP,host);
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
            country=GeoIP_country_name_by_addr(m_geoIP,host);
            if (country)
                s+=wxT(" (")+A2U(country)+wxT(")");
        }
        else
            s=wxEmptyString;
    }
    else
        s=_("GeoIP database not found.");

    free(host);

    if (flag)
        m_imgList.Replace(1,wxBitmap(flag));
    else
        m_imgList.Replace(1,wxBitmap(unknown_xpm));
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

    if (info->m_exInfo!=CSL_EXINFO_FALSE)
    {
        if (info->m_exInfo==CSL_EXINFO_OK)
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
