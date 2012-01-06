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

#include <Csl.h>
// GetSystemIPAddresses()
#ifdef __WXMSW__
#else
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdio.h>
#include <arpa/inet.h>
#endif // __WXMSW__

#ifdef __DEBUG__
void Debug_Printf(const char *file, int line, const char *func, const char *fmt, ...)
{
#ifdef __WXMSW__
    const char *filename=strstr(file, "\\src\\");
#else
    const char *filename=strstr(file, "/src/");
#endif
    filename=filename ? filename+1:file;
    va_list ArgList;
    va_start(ArgList, fmt);
    fprintf(stdout, "%u:%s:%d:%s(): ", GetTicks(), filename, line, func);
    vfprintf(stdout, fmt, ArgList);
    fflush(stdout);
    va_end(ArgList);
}
#endif

wxString& CmdlineEscapeQuotes(wxString& str)
{
#ifdef __WXMSW__
    str.Replace(wxT("\""), wxT("\\\""));
#endif

    return str;
}

wxString& CmdlineEscapeSpaces(wxString& str)
{
#ifndef __WXMSW__
    str.Replace(wxT(" "), wxT("\\ "));
#endif

    return str;
}

char* FixString(char *src, wxInt32 coloursize, bool space, bool newline, bool tab)
{
    uchar c;
    char *buf=src, *dst=src;

    for (c=*buf; c; c=*(++buf))
    {
        if (c=='\f')
            for (wxInt32 l=coloursize; l && *++buf; l--);
        else if (iscubeprint(c) ||
                 (space   && c==' ') ||
                 (newline && (c=='\r' || c=='\n')) ||
                 (tab     && (c=='\t' || c=='\v')))
            *dst++=c;
    }

    *dst=0;

    return src;
}

wxString& FixFilename(wxString& name)
{
    wxUint32 i, j;
    static const wxString exclude=wxT("\\/:*?\"<>| ");

    for (i=0; i<name.Length(); i++)
        for (j=0; j<exclude.Length(); j++)
            if (name.GetChar(i)==exclude.GetChar(j))
                name.SetChar(i, wxT('_'));

    return name;
}

// MIT bit count
wxInt32 BitCount32(wxUint32 value)
{
    register wxUint32 tmp;
    tmp=value-((value>>1)&033333333333)-((value>>2)&011111111111);
    return ((tmp+(tmp>>3))&030707070707)%63;
}

bool IsIP(const wxString& s)
{
    CslIPV4Addr addr(s);
    return addr.GetNetmask()==32;
}

#define CSLIPV4ADDRESSESLOCALLEN  5
static const CslIPV4Addr CslIPV4AddressesLocal[CSLIPV4ADDRESSESLOCALLEN] =
{
    CslIPV4Addr("0.0.0.0/8"),
    CslIPV4Addr("10.0.0.0/8"),
    CslIPV4Addr("127.0.0.0/8"),
    CslIPV4Addr("172.16.0.0/12"),
    CslIPV4Addr("192.168.0.0/16")
};

bool IsLocalIP(const CslIPV4Addr& addr, vector<CslIPV4Addr*> *ifaces)
{
    if (!addr.IsOk())
        return false;

    if (ifaces)
    {
        loopv(*ifaces)
        {
            const CslIPV4Addr& iface=*(*ifaces)[i];

            if (iface.IsInRange(addr))
                return true;
        }

        return false;
    }

    for (wxUint32 i=0; i<CSLIPV4ADDRESSESLOCALLEN; i++)
    {
        if (CslIPV4AddressesLocal[i].IsInRange(addr))
            return true;
    }

    return false;
}

void GetSystemIPAddresses(vector<CslIPV4Addr*>& ifaces)
{
#ifndef __WXMSW__
    int len, sock;
    char buf[1024];
    struct ifreq *ifr;
    struct ifconf ifc;

    if ((sock=socket(AF_INET, SOCK_DGRAM, 0))<0)
    {
        LOG_DEBUG("socket() failed.");
        return;
    }

    ifc.ifc_len=sizeof(buf);
    ifc.ifc_buf=buf;

    if (ioctl(sock, SIOCGIFCONF, &ifc)<0)
    {
        LOG_DEBUG("ioctl(SIOCGIFCONF) failed.");
        return;
    }

    ifr=ifc.ifc_req;
    len=ifc.ifc_len/sizeof(struct ifreq);

    wxUint32 ip, mask;

    for (int i=0; i<len; i++)
    {
        struct ifreq *item=&ifr[i];

        ip=(wxUint32)((sockaddr_in*)&item->ifr_addr)->sin_addr.s_addr;

        if (ioctl(sock, SIOCGIFNETMASK, item)<0)
        {
            LOG_DEBUG("ioctl(SIOCGIFNETMASK) failed.");
            continue;
        }
        mask=(wxUint32)((sockaddr_in*)&item->ifr_netmask)->sin_addr.s_addr;

        if (ioctl(sock, SIOCGIFBRDADDR, item)<0)
        {
            LOG_DEBUG("ioctl(SIOCGIFBRDADDR) failed.");
            continue;
        }

        ifaces.add(new CslIPV4Addr(ip, mask));

        LOG_DEBUG("%s / %s\n", U2A(NtoA(ip)), U2A(NtoA(mask)));
    }
#endif // __WXMSW__
}

wxUint32 AtoN(const wxString& s)
{
#if 0
    wxString m;
    long unsigned int ul;
    wxUint32 i=0, ip=0, l=(wxUint32)s.Len(), mult=0x1000000;

    for (; i<=l; i++)
    {
        if (i<l && s.Mid(i, 1).IsNumber())
            m+=s.Mid(i, 1);
        else
        {
            m.ToULong(&ul, 10);
            if (ul>255)
                return 0;
            ip+=ul*mult;
            mult>>=8;
            m.Empty();
        }
    }

    return ip;
#else
#ifdef __WXMSW__
    wxUint32 ip=0;

    if ((ip=inet_addr(U2A(s)))!=(wxUint32)-1)
        return ip;
#else
    in_addr in;

    if (inet_aton(U2A(s), &in))
        return in.s_addr;
#endif // __WXMSW__
    return 0;
#endif
}

wxString NtoA(wxUint32 ip)
{
#if 0
    return wxString::Format(wxT("%d.%d.%d.%d"), ip&0xff, ip>>8&0xff, ip>>16&0xff, ip>>24);
#else
    in_addr in;
    in.s_addr=ip;

    return wxString(A2U(inet_ntoa(in)));
#endif
}

wxString FormatBytes(wxUint64 size)
{
    if (size>0x100000)
    {
#ifdef USE_FLOAT_PRECISION
        float s=(float)size/0x100000;
        return wxString::Format(wxT("%.2f MiB"), s);
#else
        wxUint64 m=size/0x100000;
        wxUint64 k=(size%0x100000)*100/0x100000;
        return wxString::Format(wxT("%lli.%-2.2lli MiB"), m, k);
#endif

    }
    else if (size>0x400)
        return wxString::Format(wxT("%lli KiB"), size/0x400);
    else
        return wxString::Format(wxT("%lli B"), size);
}

wxString FormatSeconds(wxUint32 time, bool space, bool full)
{
    wxString s;
    wxUint32 dy, hr, mn, sc;
    wxUint32 rest=time;

    dy=rest/86400;
    rest%=86400;
    hr=rest/3600;
    rest%=3600;
    mn=rest/60;
    sc=rest%60;

    if (dy)
    {
        s<<s.Format(wxT("%d"), dy);
        if (space)
            s<<wxT(" ");
        s<<(full ? (dy==1 ? _("day") : _("days")) : _("d"))<<wxT(" ");
    }
    if (hr)
    {
        s+=s.Format(wxT("%d"), hr);
        if (space)
            s<<wxT(" ");
        s<<(full ? (hr==1 ? _("hour") : _("hours")) : _("h"))<<wxT(" ");
    }
    if (mn)
    {
        s<<s.Format(wxT("%d"), mn);
        if (space)
            s<<wxT(" ");
        s<<(full ? (mn==1 ? _("minute") : _("minutes")) : _("min"))<<wxT(" ");
    }
    if (sc)
    {
        s<<s.Format(wxT("%d"), sc);
        if (space)
            s<<wxT(" ");
        s<<(full ? (sc==1 ? _("second") : _("seconds")) : _("sec"))<<wxT(" ");
    }

    return s.Trim(true);
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
#endif // !defined(_WINSOCK2API_) && !defined(_WINSOCKAPI_)
int gettimeofday(struct timeval* tv, void *dummy)
{
    union
    {
        long long ns100;
        FILETIME ft;
    } now;

    GetSystemTimeAsFileTime(&now.ft);
    tv->tv_usec=(long)((now.ns100/10LL)%1000000LL);
    tv->tv_sec=(long)((now.ns100-116444736000000000LL)/10000000LL);
    return 0;
}
#endif //USE_WIN32_TICK
#else
#include <sys/time.h>
#endif //__WXMSW__

wxUint32 GetTicks()
{
    static wxUint32 init=0;
    wxUint32 ticks;
#if defined(__WXMSW__) && defined(CSL_USE_WIN32_TICK)
    ticks=GetTickCount();
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    wxUint32 r=tv.tv_usec/1000;
    wxUint64 v=tv.tv_sec*1000;
    ticks=v+r-init;
#endif
    if (!init)
    {
        init=ticks;
        return 0;
    }
    return ticks;
}

const wxString& GetHttpAgent()
{
    static wxString agent;

    if (agent.IsEmpty())
    {
        wxPlatformInfo pinfo;
        wxOperatingSystemId sid=pinfo.GetOperatingSystemId();
        agent<<CSL_NAME_SHORT_STR<<wxT(" ")<<CSL_VERSION_STR<<wxT(" ");
        agent<<wxT("(")<<pinfo.GetOperatingSystemFamilyName(sid);
        agent<<wxT("; N; ")<<pinfo.GetOperatingSystemIdName(sid)<<wxT(")");
    }

    return agent;
}

static wxArrayString datadirs;
static wxArrayString plugindirs;

wxString FileName(const wxString& path, bool native)
{
    wxFileName f=wxFileName::FileName(path, native ? wxPATH_NATIVE : wxPATH_UNIX);

    return f.GetFullPath(wxPATH_NATIVE);
}

wxString DirName(const wxString& path, bool native)
{
    wxFileName f=wxFileName::DirName(path, native ? wxPATH_NATIVE : wxPATH_UNIX);

    return f.GetPath(wxPATH_GET_VOLUME|wxPATH_GET_SEPARATOR, wxPATH_NATIVE);
}

void AddDir(const wxString& path, bool native, wxArrayString& array)
{
    wxString pathname=DirName(path, native);

    for (wxInt32 i=0, j=array.GetCount(); i<j; i++)
    {
        if (array.Item(i)==pathname)
            return;
    }

    array.Add(pathname);
}

void RemoveDir(const wxString& path, wxArrayString& array)
{
    for (wxInt32 i=0, j=array.GetCount(); i<j; i++)
    {
        if (array.Item(i)==path)
        {
            array.RemoveAt(i);
            return;
        }
    }
}

wxString FindFile(const wxString& path, wxArrayString& array)
{
    wxString file;
    wxString pathname=FileName(path);

    for (wxInt32 i=0, j=array.GetCount(); i<j; i++)
    {
        file=array.Item(i)+pathname;

        if (wxFile::Exists(file))
            return file;
    }

    return wxEmptyString;
}

void AddPluginDir(const wxString& path, bool native)
{
    AddDir(path, native, plugindirs);
}

void RemovePluginDir(const wxString& path)
{
    RemoveDir(path, plugindirs);
}

wxArrayString GetPluginDirs()
{
    return plugindirs;
}

void AddDataDir(const wxString& path, bool native)
{
    AddDir(path, native, datadirs);
}

void RemoveDataDir(const wxString& path)
{
    RemoveDir(path, datadirs);
}

wxArrayString GetDataDirs()
{
    return datadirs;
}

wxString FindPackageFile(const wxString& filename)
{
    return FindFile(filename, datadirs);
}

wxInt32 FindFiles(const wxString& path, const wxString& filespec, wxArrayString& results)
{
    return wxDir::GetAllFiles(path, &results, filespec);
}

wxInt32 WriteTextFile(const wxString& filename, const wxString& data, const wxFile::OpenMode mode)
{
    wxFile file(filename, mode);

    if (!file.IsOpened())
        return CSL_ERROR_FILE_OPERATION;

    size_t l=data.Length();
    size_t w=file.Write(U2A(data), l);
    file.Close();

    return l!=w ? CSL_ERROR_FILE_OPERATION:CSL_ERROR_NONE;
}
