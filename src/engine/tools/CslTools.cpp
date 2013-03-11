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

#include <Csl.h>
// GetSystemIPAddresses()
#ifndef __WXMSW__
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdio.h>
#include <arpa/inet.h>
#endif // __WXMSW__

DEFINE_EVENT_TYPE(wxCSL_EVT_THREAD_TERMINATE)

wxMutex g_csl_printf_mutex;

void Debug_Printf(const char *file, int line, const char *func, const char *fmt, ...)
{
    const char *filename = strstr(file, CSL_PATHDIV_MB "src" CSL_PATHDIV_MB);
    filename=filename ? filename+1 : file;
    va_list ArgList;
    va_start(ArgList, fmt);
    g_csl_printf_mutex.Lock();
    fprintf(stdout, "%u:%s:%d:%s(): ", GetTicks(), filename, line, func);
    vfprintf(stdout, fmt, ArgList);
    fflush(stdout);
    g_csl_printf_mutex.Unlock();
    va_end(ArgList);
}

wxString& CmdlineEscapeQuotes(wxString& str)
{
#ifndef __WXMSW__
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

char* FilterCubeString(char *src, wxInt32 coloursize, bool space, bool newline, bool tab)
{
    uchar c;
    char *buf=src, *dst=src;

    for (c=*buf; c; c=*(++buf))
    {
        if (c=='\f') for (wxInt32 l=coloursize; l && *++buf; l--);
        else if (iscubeprint(c) ||
                 (space   &&  c==' ') ||
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

#define CSLIPV4ADDRESSESLOCALLEN  5
static const CslIPV4Addr CslArrayCslIPV4AddrLocal[CSLIPV4ADDRESSESLOCALLEN] =
{
    CslIPV4Addr("0.0.0.0/8"),
    CslIPV4Addr("10.0.0.0/8"),
    CslIPV4Addr("127.0.0.0/8"),
    CslIPV4Addr("172.16.0.0/12"),
    CslIPV4Addr("192.168.0.0/16")
};

bool IsLocalIPV4(const CslIPV4Addr& addr, CslArrayCslIPV4Addr *addresses)
{
    if (!addr.IsOk())
        return false;

    if (addresses)
    {
        loopv(*addresses)
        {
            const CslIPV4Addr& sysaddr=*(*addresses)[i];

            if (sysaddr.IsInRange(addr))
                return true;
        }

        return false;
    }

    for (wxUint32 i=0; i<CSLIPV4ADDRESSESLOCALLEN; i++)
    {
        if (CslArrayCslIPV4AddrLocal[i].IsInRange(addr))
            return true;
    }

    return false;
}

void GetSystemIPV4Addresses(CslArrayCslIPV4Addr& addresses)
{
    wxUint32 ip, mask;

#ifdef __WXMSW__
    SOCKET sock=WSASocket(AF_INET, SOCK_DGRAM, 0, 0, 0, 0);

    if (sock==INVALID_SOCKET)
    {
        CSL_LOG_DEBUG("WSASocket() failed (WSA error: %d).\n", WSAGetLastError());
        return;
    }

    unsigned long len;
    INTERFACE_INFO ifi[24];

    if (WSAIoctl(sock, SIO_GET_INTERFACE_LIST, 0, 0, &ifi,
                 sizeof(ifi), &len, 0, 0)==SOCKET_ERROR)
    {
        CSL_LOG_DEBUG("WSAIoctl() failed (WSA error: %d).\n", WSAGetLastError());
        goto cleanup;
    }

    len/=sizeof(INTERFACE_INFO);

    for (unsigned long i=0;i<len; ++i)
    {
        ip=(wxUint32)((sockaddr_in*)&(ifi[i].iiAddress))->sin_addr.S_un.S_addr;
        mask=(wxUint32)((sockaddr_in*)&(ifi[i].iiNetmask))->sin_addr.S_un.S_addr;

        CslIPV4Addr *addr = new CslIPV4Addr(ip, 0, mask);
        addresses.Add(addr);

        CSL_LOG_DEBUG("%s\n", U2C(addr->Format(wxT("%i / %m"))));
    }

cleanup:
    if (closesocket(sock)==SOCKET_ERROR)
        CSL_LOG_DEBUG("closesocket() failed (WSA error: %d).\n", WSAGetLastError());
#else
    int len, sock;
    char buf[4096];
    struct ifreq *ifr;
    struct ifconf ifc;

    if ((sock=socket(AF_INET, SOCK_DGRAM, 0))<0)
    {
        CSL_LOG_DEBUG("socket() failed.");
        return;
    }

    ifc.ifc_len=sizeof(buf);
    ifc.ifc_buf=buf;

    if (ioctl(sock, SIOCGIFCONF, &ifc)<0)
    {
        CSL_LOG_DEBUG("ioctl(SIOCGIFCONF) failed.\n");
        goto cleanup;
    }

    ifr=ifc.ifc_req;
    len=ifc.ifc_len/sizeof(struct ifreq);

    for (int i=0; i<len; i++)
    {
        struct ifreq *item=&ifr[i];

        ip=(wxUint32)((sockaddr_in*)&item->ifr_addr)->sin_addr.s_addr;

        if (ioctl(sock, SIOCGIFNETMASK, item)<0)
        {
            CSL_LOG_DEBUG("ioctl(SIOCGIFNETMASK) failed.\n");
            continue;
        }
        mask=(wxUint32)((sockaddr_in*)&item->ifr_netmask)->sin_addr.s_addr;

        if (ioctl(sock, SIOCGIFBRDADDR, item)<0)
        {
            CSL_LOG_DEBUG("ioctl(SIOCGIFBRDADDR) failed.\n");
            continue;
        }

        CslIPV4Addr *addr = new CslIPV4Addr(ip, 0, mask);
        addresses.Add(addr);

        CSL_LOG_DEBUG("%s\n", U2C(addr->Format(wxT("%i / %m"))));
    }

cleanup:
    if (close(sock)<0)
        CSL_LOG_DEBUG("close() failed.\n");
#endif // __WXMSW__
}

void FreeSystemIPV4Addresses(CslArrayCslIPV4Addr& addresses)
{
    WX_CLEAR_ARRAY(addresses);
}

wxUint32 AtoN(const wxString& s)
{
#ifdef __WXMSW__
    wxUint32 ip=0;

    if ((ip=inet_addr(U2C(s)))!=(wxUint32)-1)
        return ip;
#else
    in_addr in;

    if (inet_aton(U2C(s), &in))
        return in.s_addr;
#endif // __WXMSW__
    return 0;
}

wxString NtoA(wxUint32 ip)
{
    in_addr in;
    in.s_addr=ip;

    return C2U(inet_ntoa(in));
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
int gettimeofday(struct timeval* tv, void *WXUNUSED(dummy))
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
    return ::wxDirExists(path) ? wxDir::GetAllFiles(path, &results, filespec) : 0;
}

wxInt32 WriteTextFile(const wxString& filename, const wxString& data, const wxFile::OpenMode mode)
{
    wxFile file(filename, mode);

    if (!file.IsOpened())
        return CSL_ERROR_FILE_OPERATION;

    size_t l=data.Length();
    size_t w=file.Write(U2C(data), l);
    file.Close();

    return l!=w ? CSL_ERROR_FILE_OPERATION:CSL_ERROR_NONE;
}
