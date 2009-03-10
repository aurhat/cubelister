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

#include <wx/wxprec.h>
#ifdef __BORLANDC__
#pragma hdrstop
#endif
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/platinfo.h>
#include <wx/regex.h>
#include "CslVersion.h"
#include "CslTools.h"

wxString g_basePath;

#ifdef __WXDEBUG__
void Debug_Printf(const char *file,int line,const char *func,const char *fmt,...)
{
#ifdef __WXMSW__
    const char *filename=strstr(file,"\\src\\");
#else
    const char *filename=strstr(file,"/src/");
#endif
    filename=filename ? filename+1:file;
    va_list ArgList;
    va_start(ArgList,fmt);
    fprintf(stdout,"%u:%s:%d:%s(): ",GetTicks(),filename,line,func);
    vfprintf(stdout,fmt,ArgList);
    fflush(stdout);
    va_end(ArgList);
}
#endif

void FixString(char *src,wxUint32 *len,wxUint32 count,bool keepnl)
{
    char *dst=src;
    wxUint32 c,l=0;

    for (c=*src;c && *len>0;c=*(++src))
    {
        if (c=='\f')
        {
            src+=count;
            *len-=count+1;
        }
        else if (isprint(c) || (keepnl && (c=='\r' || c=='\n')))
        {
            *dst++=c;
            (*len)--;
            l++;
        }
    }

    *dst=0;
    *len=l;
}

void FixFilename(wxString& name)
{
    wxUint32 i,j;
    wxString exclude=wxT("\\/:*?\"<>| ");

    for (i=0;i<name.Length();i++)
        for (j=0;j<exclude.Length();j++)
            if (name.GetChar(i)==exclude.GetChar(j))
                name.SetChar(i,wxT('_'));
}

bool IsIP(const wxString& s)
{
    const wxChar* dot=wxT(".");
    wxString digit=wxT("0*[0-9]{1,3}");
    wxString exp=wxT("^")+digit+dot+digit+dot+digit+dot+digit+wxT("$");
    wxRegEx regex;

    regex.Compile(exp);
    return regex.Matches(s);
}

bool IsLocalIP(const wxString& s)
{
    if (s.Left(2)==wxT("0.") ||
        s.Left(3)==wxT("10.") ||
        s.Left(3)==wxT("127") ||
        s.Left(7)==wxT("192.168") ||
        s.Left(7)==wxT("169.254"))
        return true;

    if (s.Left(3)==wxT("172"))
    {
        wxString m=s.Mid(4);
        wxInt32 pos=m.Find(wxT("."));

        if (pos<0)
           return false;

        long l;
        m.Left(pos).ToLong(&l);

        if (l>=16 && l<=31)
            return true;
    }

    return false;
}

wxUint32 IP2Int(const wxString& s)
{
    long unsigned int ul;
    wxUint32 i=0,ip=0,l=(wxUint32)s.Len(),mult=0x1000000;
    wxString m;

    for (;i<=l;i++)
    {
        if (i<l && s.Mid(i,1).IsNumber())
            m+=s.Mid(i,1);
        else
        {
            m.ToULong(&ul,10);
            if (ul>255)
                return 0;
            ip+=ul*mult;
            mult>>=8;
            m.Empty();
        }
    }

    return ip;
}

wxString FormatBytes(wxUint64 size)
{
    if (size>0x100000)
    {
#ifdef USE_FLOAT_PRECISION
        float s=(float)size/0x100000;
        return wxString::Format(wxT("%.2f MiB"),s);
#else
        wxUint64 m=size/0x100000;
        wxUint64 k=(size%0x100000)*100/0x100000;
        return wxString::Format(wxT("%lli.%-2.2lli MiB"),m,k);
#endif

    }
    else if (size>0x400)
        return wxString::Format(wxT("%lli KiB"),size/0x400);
    else
        return wxString::Format(wxT("%lli B"),size);
}

wxString FormatSeconds(wxUint32 time,bool space,bool full)
{
    wxString s;
    wxUint32 dy,hr,mn,sc;
    wxUint32 rest=time;

    dy=rest/86400;
    rest%=86400;
    hr=rest/3600;
    rest%=3600;
    mn=rest/60;
    sc=rest%60;

    if (dy)
    {
        s+=s.Format(wxT("%d"),dy);
        if (space)
            s+=wxT(" ");
        if (full)
        {
            if (dy==1)
                s+=_("day");
            else
                s+=_("days");
        }
        else
            s+=_("d");
        s+=wxT(" ");
    }
    if (hr)
    {
        s+=s.Format(wxT("%d"),hr);
        if (space)
            s+=wxT(" ");
        if (full)
        {
            if (hr==1)
                s+=_("hour");
            else
                s+=_("hours");
        }
        else
            s+=_("h");
        s+=wxT(" ");
    }
    if (mn)
    {
        s+=s.Format(wxT("%d"),mn);
        if (space)
            s+=wxT(" ");
        if (full)
        {
            if (mn==1)
                s+=_("minute");
            else
                s+=_("minutes");
        }
        else
            s+=_("min");
        s+=wxT(" ");
    }
    if (sc)
    {
        s+=s.Format(wxT("%d"),sc);
        if (space)
            s+=wxT(" ");
        if (full)
        {
            if (sc==1)
                s+=_("second");
            else
                s+=_("seconds");
        }
        else
            s+=_("sec");
    }

    return s;
}


#define CSL_USE_WIN32_TICK

#ifdef __WXMSW__
#ifndef CSL_USE_WIN32_TICK
#include <time.h>
#if !defined(_WINSOCK2API_) && !defined(_WINSOCKAPI_)
struct timeval
{
    long tv_sec;
    long tv_usec;
};
#endif
int gettimeofday(struct timeval* tv,void *dummy)
{
    union
    {
        long long ns100;
        FILETIME ft;
    } now;

    GetSystemTimeAsFileTime(&now.ft);
    tv->tv_usec = (long)((now.ns100 / 10LL) % 1000000LL);
    tv->tv_sec = (long)((now.ns100 - 116444736000000000LL) / 10000000LL);
    return 0;
}
#endif //USE_WIN32_TICK
#else
#include <sys/time.h>
#endif //__WXMSW__

wxUint32 GetTicks()
{
    static wxUint32 initTicks=0;
    wxUint32 ticks;
#if defined(__WXMSW__) && defined(CSL_USE_WIN32_TICK)
    ticks=GetTickCount();
#else
    struct timeval tv;
    gettimeofday(&tv,NULL);
    wxUint32 r=tv.tv_usec/1000;
    wxUint64 v=tv.tv_sec*1000;
    ticks=v+r-initTicks;
#endif
    if (!initTicks)
    {
        initTicks=ticks;
        return 0;
    }
    return ticks;
}

wxBitmap AdjustIconSize(const char **data,const wxIcon& icon,
                        const wxSize& size,const wxPoint& origin)
{
    wxBitmap bitmap;
    if (data)
        bitmap=wxBitmap(data);
    else
        bitmap=wxBitmap(icon);
    wxImage image=bitmap.ConvertToImage();
    image.Resize(size,origin);
    return wxBitmap(image);
}

bool BitmapFromWindow(wxWindow *window,wxBitmap& bitmap)
{
    bool ret;
    wxMemoryDC mdc;
    wxClientDC cdc(window);
    const wxSize& size=window->GetClientSize();

    window->Raise();
    wxTheApp->Yield();

    bitmap.Create(size.x,size.y);
    mdc.SelectObject(bitmap);
    ret=mdc.Blit(0,0,size.x,size.y,&cdc,0,0);
    mdc.SelectObject(wxNullBitmap);

    return ret;
}

void RegisterEventsRecursively(wxInt32 id,wxWindow *parent,wxEvtHandler *handler,
                               wxEventType type,wxObjectEventFunction function)
{
    if (parent)
    {
        parent->Connect(id,type,function,NULL,handler);

        wxWindowListNode *node=parent->GetChildren().GetFirst();

        while (node)
        {
            wxWindow* child=node->GetData();
            RegisterEventsRecursively(id,child,handler,type,function);
            node=node->GetNext();
        }
    }
}

wxString GetHttpAgent()
{
    wxPlatformInfo pinfo;
    wxOperatingSystemId id=pinfo.GetOperatingSystemId();
    wxString agent=wxT("Csl/");
    agent+=CSL_VERSION_LONG_STR;
    agent+=wxT("(")+pinfo.GetOperatingSystemFamilyName(id)+wxT(")");
    return agent;
}

wxInt32 WriteTextFile(const wxString& filename,const wxString& data,const wxFile::OpenMode mode)
{
    wxFile file(filename,mode);

    if (!file.IsOpened())
        return CSL_ERROR_FILE_OPERATION;

    size_t l=data.Length();
    size_t w=file.Write(U2A(data),l);
    file.Close();

    return l!=w ? CSL_ERROR_FILE_OPERATION:CSL_ERROR_NONE;
}
