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

#include "Csl.h"
#include "CslProtocolInput.h"
#include <wx/archive.h>
#include <wx/sckstrm.h>
#include <wx/tarstrm.h>
#include <wx/wfstream.h>
#include <wx/zipstrm.h>

DEFINE_EVENT_TYPE(wxCSL_EVT_PROTO_INPUT)


IMPLEMENT_DYNAMIC_CLASS(CslArchiveProto, wxProtocol)
IMPLEMENT_DYNAMIC_CLASS(CslTcpProto, wxProtocol)

IMPLEMENT_DYNAMIC_CLASS(CslProtocolInputEvent, wxEvent)
IMPLEMENT_DYNAMIC_CLASS(CslProtocolInput, wxObject)

IMPLEMENT_PROTOCOL(CslTcpProto, wxT("tcp"), wxT("10000"), true)
IMPLEMENT_PROTOCOL(CslArchiveProto, wxT("archive"), NULL, false)

wxInputStream* CslTcpProto::GetInputStream(const wxString& path)
{
    if (!path.IsEmpty())
    {
        CslCharBuffer buf = CslWX2MB(path);

        wxSocketBase::Write((void*)buf.data(), path.size());

        if (wxSocketBase::LastCount()!=path.size())
        {
            m_lastError = wxPROTO_NETERR;
            return NULL;
        }
    }

    wxSocketInputStream *stream = new wxSocketInputStream(*this);

    if (stream->IsOk())
    {
        m_lastError = wxPROTO_NOERR;
        return stream;
    }

    m_lastError = wxPROTO_NETERR;
    delete stream;

    return NULL;
}


template<class T>
wxInputStream* CslArchiveProto::GetFactoryStream(wxString& path, wxInputStream *iStream)
{
    const T *factory = T::Find(path, wxSTREAM_FILEEXT);

    if (!factory)
    {
        CSL_DO_DEBUG
        (
            wxString list;
            factory = T::GetFirst();

            while (factory)
            {
                list << wxT(" ") << factory->GetProtocol();
                factory = factory->GetNext();
            }

            if (list.IsEmpty())
                list << wxT(" none");

            CSL_LOG_DEBUG("%s handler for '%s' not found - available are:%s\n",
                          U2C(CLASSINFO(T)->GetClassName()), U2C(path.AfterLast('.')), U2C(list));
        );

        return NULL;
    }

    wxInputStream *stream = factory->NewStream(iStream ? iStream : new wxFileInputStream(path));

    if (stream->IsOk())
    {
        path = factory->PopExtension(path);
        return stream;
    }

    delete stream;
    return NULL;
}

wxInputStream* CslArchiveProto::GetInputStream(const wxString& path)
{
    wxString archiveName = path;
    wxInputStream *stream, *astream;

    stream = GetFactoryStream<wxFilterClassFactory>(archiveName);

    if ((astream = GetFactoryStream<wxArchiveClassFactory>(archiveName, stream)))
    {
        stream = astream;
        m_isArchive = true;
    }

    if (stream)
    {
        wxFileName fn(archiveName);
        m_archiveName = fn.GetFullName();

    }
    else
        m_lastError = wxPROTO_NOHNDLR;

    return stream;
}


CslProtocolInput::~CslProtocolInput()
{
    DoTerminate();

    if (wxThread::IsRunning())
        wxThread::Wait();

    wxDELETE(m_inputStream);
    wxDELETE(m_outputStream);
    wxDELETE(m_protocol);

}

bool CslProtocolInput::Create(wxEvtHandler *handler, wxInt32 id,
                              const wxURI& uri,
                              const wxFileName& output,
                              const wxDateTime& modifiedSince)
{
    if (wxThread::IsRunning())
    {
        m_protocolError = wxPROTO_STREAMING;
        return false;
    }

    m_terminate = false;

    m_maxRead = (size_t)-1;
    m_totalRead = 0;
    m_totalSize = 0;
    m_chunkSize = CSL_PROTO_INPUT_CHUNK_SIZE;

    m_clientData = NULL;

    m_handler = handler;
    m_id = id;

    m_statusCode = 0;

    m_inputStream = NULL;
    m_outputStream = NULL;

    m_protocol = NULL;
    m_protocolError = wxPROTO_NOERR;

    m_inputURI = uri;
    SetOutput(output);

    m_modifiedSince = modifiedSince;

    return m_protocolError==wxPROTO_NOERR;
}

wxThreadError CslProtocolInput::Run()
{
    wxThreadError err = wxTHREAD_NO_ERROR;

    if ((err = wxThread::Create())==wxTHREAD_NO_ERROR)
        err = wxThread::Run();

    return err;
}

void CslProtocolInput::DoTerminate()
{
    wxCriticalSectionLocker lock(m_critSection);

    m_terminate = true;
}

wxProtocolError CslProtocolInput::GetError()
{
    wxCriticalSectionLocker lock(m_critSection);

    return m_protocolError;
}

wxFileName CslProtocolInput::GetOutput()
{
    wxCriticalSectionLocker lock(m_critSection);

    return m_outputPath;
}


size_t CslProtocolInput::GetReadSize()
{
    wxCriticalSectionLocker lock(m_critSection);

    return m_totalRead;
}

size_t CslProtocolInput::GetTotalSize()
{
    wxCriticalSectionLocker lock(m_critSection);

    return m_totalSize;
}

wxString CslProtocolInput::GetHeader(const wxString& header)
{
    wxHTTP *http = wxDynamicCast(m_protocol, wxHTTP);

    if (!http)
        return wxEmptyString;

    return http->GetHeader(header);
}

bool CslProtocolInput::SetOutput(const wxFileName& fileName)
{
    if (wxThread::IsRunning())
        return false;

    m_outputPath = fileName;

    return true;
}

void CslProtocolInput::SetMaxRead(size_t size)
{
    wxCriticalSectionLocker lock(m_critSection);

    m_maxRead = size ? size : (size_t)-1;
}

void CslProtocolInput::SetChunkSize(size_t size)
{
    wxCriticalSectionLocker lock(m_critSection);

    m_chunkSize = size ? size : CSL_PROTO_INPUT_CHUNK_SIZE;
}

void CslProtocolInput::SetClientData(void *data)
{
    if (wxThread::IsRunning())
        return;

    m_clientData = data;
}

bool CslProtocolInput::SetHeader(const wxString& header, const wxString& data)
{
    wxHTTP *http = wxDynamicCast(m_protocol, wxHTTP);

    if (!http)
        return false;

    http->SetHeader(header, data);

    return true;
}

void CslProtocolInput::SetTotalSize()
{
    wxCriticalSectionLocker lock(m_critSection);

    m_totalSize = m_inputStream ? m_inputStream->GetSize() : 0;

    if (m_totalSize==(size_t)-1)
        m_totalSize = 0;
}

void CslProtocolInput::SetError(wxProtocolError error)
{
    wxCriticalSectionLocker lock(m_critSection);

    m_protocolError = error;
}

bool CslProtocolInput::CreateOutputStream(const wxFileName& fileName)
{
    const wxString dstPath = fileName.GetPathWithSep();
    const wxString dstFile = fileName.GetFullName();

    if (dstFile.IsEmpty())
        return NULL;

    if (!wxDir::Exists(dstPath) && !wxFileName::Mkdir(dstPath, 0700, wxPATH_MKDIR_FULL))
        return NULL;

    m_outputStream = new wxFileOutputStream(dstPath + dstFile);

    if (!m_outputStream->IsOk())
        wxDELETE(m_outputStream);

    return m_outputStream != NULL;
}

wxThread::ExitCode CslProtocolInput::Entry()
{
    CSL_DEF_DEBUG(CslStopWatch watch(false));

    wxString scheme = m_inputURI.GetScheme();

    CslProtocolInputEvent event(m_id, m_clientData);
    event.SetEventObject(m_handler);

    if (scheme==wxT("http"))
    {
        m_protocol = new wxHTTP;
        HandleProto((wxHTTP&)(*m_protocol), event);
    }
    else if (scheme==wxT("ftp"))
    {
        m_protocol = new wxFTP;
        HandleProto((wxFTP&)(*m_protocol), event);
    }
    else if (scheme==wxT("tcp"))
    {
        m_protocol = new CslTcpProto;
        HandleProto((CslTcpProto&)(*m_protocol), event);
    }
    else if (scheme==wxT("file"))
    {
        m_protocol = new wxFileProto;
        HandleProto((wxFileProto&)(*m_protocol), event);
    }
    else if (scheme==wxT("archive"))
    {
        m_protocol = new CslArchiveProto;
        HandleProto((CslArchiveProto&)(*m_protocol), event);
    }
    else
    {
        SetError(wxPROTO_NOHNDLR);

        wxASSERT_MSG(m_protocol!=NULL, wxT("unknown protocol"));
    }

    CSL_LOG_DEBUG("%s - processing time: %ld ms\n",
                  U2C(m_inputURI.BuildURI()), watch.Time());

    event.m_buffer = wxMemoryBuffer(0);
    event.m_status |= CslProtocolInputEvent::TERMINATE;

    ::wxPostEvent(m_handler, event);

    return (wxThread::ExitCode)0;
}

void CslProtocolInput::HandleProto(wxHTTP& http, CslProtocolInputEvent& event)
{
    wxIPV4address addr;
    addr.Hostname(m_inputURI.GetServer());
    addr.Service(m_inputURI.HasPort() ? (wxUint16)wxAtoi(m_inputURI.GetPort()) : 80);

    if (m_modifiedSince.IsValid())
    {
        wxString rfc2822 = ::ToRfc2822(m_modifiedSince);
        http.SetHeader(wxT("If-Modified-Since"), rfc2822);
    }

    http.SetHeader(wxT("User-Agent"), ::GetHttpAgent());
    http.SetTimeout(10);
    http.Notify(false);
    http.Connect(addr, true);

    wxString srcPath(m_inputURI.GetPath());

    m_inputStream = http.GetInputStream(srcPath);
    m_statusCode = http.GetResponse();

    ::wxYieldIfNeeded();

    SetTotalSize();
    SetError(http.GetError());

    event.m_statusCode = m_statusCode;
    event.m_totalSize = m_totalSize;
    event.m_fileProperties.Size = m_totalSize;
    event.m_fileProperties.Name = m_outputPath;
    event.m_fileProperties.Time.Modify = ::FromRfc2822(http.GetHeader(wxT("Last-Modified")));

    if (!m_outputPath.IsOk() || m_outputPath.IsDir())
        event.m_fileProperties.Name.SetFullName(wxFileName(srcPath).GetFullName());

    m_critSection.Enter();
    m_outputPath = event.m_fileProperties.Name;
    m_critSection.Leave();

#if 0
    if (evt.Output.Time.Modify.IsValid())
        CSL_LOG_DEBUG("modified: %s\n", U2C(evt.Output.Time.Modify.FromTimezone(wxDateTime::UTC).Format()));
#endif

    if (m_protocolError!=wxPROTO_NOERR)
    {
        event.m_status |= CslProtocolInputEvent::ERROR_INPUT;
        goto error;
    }

    CSL_LOG_DEBUG("%s - code: %d - size: %lu\n",
                  U2C(m_inputURI.BuildURI()),
                  m_statusCode, m_totalSize);

    if (m_statusCode==200)
    {
        if (m_outputPath.IsOk() && !CreateOutputStream(m_outputPath))
        {
            SetError(wxPROTO_INVVAL);
            event.m_status |= CslProtocolInputEvent::ERROR_OUTPUT;
            goto error;
        }

        ProcessInput(event);

        if (m_outputPath.IsOk() && event.m_fileProperties.Time.Modify.IsValid())
        {
            wxDELETE(m_outputStream);
            event.m_fileProperties.Set();
    }
    else
        goto error;

    return;

error:
    ::wxPostEvent(m_handler, event);
}

void CslProtocolInput::HandleProto(wxFTP& ftp, CslProtocolInputEvent& event)
{
    //TODO
}

void CslProtocolInput::HandleProto(CslTcpProto& tcp, CslProtocolInputEvent& event)
{
    wxIPV4address addr;
    addr.Hostname(m_inputURI.GetServer());
    addr.Service((wxUint16)wxAtoi(m_inputURI.GetPort()));

    tcp.Notify(false);
    tcp.SetTimeout(10);
    tcp.SetFlags(wxSOCKET_BLOCK|wxSOCKET_WAITALL);
    tcp.Connect(addr, true);

    wxString srcPath(m_inputURI.Unescape(m_inputURI.GetPath()).AfterFirst('/'));

    m_inputStream = tcp.GetInputStream(srcPath);

    event.m_fileProperties.Name = m_outputPath;

    if (tcp.GetError()!=wxPROTO_NOERR)
    {
        SetError(tcp.GetError());
        event.m_status |= CslProtocolInputEvent::ERROR_INPUT;
        goto error;
    }

    CSL_LOG_DEBUG("%s\n", U2C(m_inputURI.BuildURI()));

    if (m_outputPath.IsOk() && !CreateOutputStream(m_outputPath))
    {
        SetError(wxPROTO_INVVAL);
        event.m_status |= CslProtocolInputEvent::ERROR_OUTPUT;
        goto error;
    }

    ProcessInput(event);

    return;

error:
    if (event.IsError())
        ::wxPostEvent(m_handler, event);
}

void CslProtocolInput::HandleProto(wxFileProto& file, CslProtocolInputEvent& event)
{
    //TODO
}

void CslProtocolInput::HandleProto(CslArchiveProto& archive, CslProtocolInputEvent& event)
{
    wxString dstPath(m_outputPath.GetPathWithSep());
    wxString srcPath(m_inputURI.Unescape(m_inputURI.GetPath()));

    m_inputStream = archive.GetInputStream(srcPath);

    if (archive.GetError()!=wxPROTO_NOERR)
    {
        SetError(archive.GetError());

        event.m_fileProperties.Name = m_outputPath;
        event.m_status |= CslProtocolInputEvent::ERROR_INPUT;
        ::wxPostEvent(m_handler, event);

        return;
    }

    if (archive.IsArchive())
    {
        wxArrayPtrVoid filespecs;
        wxArchiveEntry *entry;
        wxArchiveInputStream& input = (wxArchiveInputStream&)(*m_inputStream);

        CSL_LOG_DEBUG("%s\n", U2C(m_inputURI.BuildUnescapedURI()));

        while (!m_terminate && (entry = input.GetNextEntry()))
        {
            wxTarEntry *tar;
            wxZipEntry *zip;

            wxString name = dstPath + entry->GetName();

            CslFileProperties *fp = new CslFileProperties;

            fp->Size = entry->GetSize();
            fp->Time.Modify = entry->GetDateTime();

            if (entry->IsDir())
            {
                fp->Name.AssignDir(name);
                fp->Type = wxTAR_DIRTYPE;
            }
            else
                fp->Name.Assign(name);

            if ((tar = wxDynamicCast(entry, wxTarEntry)))
            {
                fp->Type = tar->GetTypeFlag();
                fp->Mode = tar->GetMode();
                fp->Time.Access = tar->GetAccessTime();
                fp->Time.Create = tar->GetCreateTime();
            }
            else if ((zip = wxDynamicCast(entry, wxZipEntry)))
            {
                fp->Mode = zip->GetMode();
                fp->Type = zip->IsText() ? -1 : wxTAR_REGTYPE;
            }

            fp->Time.MakeUTC();

            event.m_bytesRead = m_totalRead;
            event.m_fileProperties = *fp;

            switch (fp->Type)
            {
                case wxTAR_REGTYPE:
                    if (!dstPath.IsEmpty() && !CreateOutputStream(fp->Name))
                    {
                        SetError(wxPROTO_INVVAL);
                        event.m_status |= CslProtocolInputEvent::ERROR_OUTPUT;
                        ::wxPostEvent(m_handler, event);
                    }
                    else
                    {
                        ProcessInput(event);
                        wxDELETE(m_outputStream);
                    }
                    break;
                case wxTAR_LNKTYPE:
                case wxTAR_SYMTYPE:
                case wxTAR_CHRTYPE:
                case wxTAR_BLKTYPE:
                    ::wxPostEvent(m_handler, event);
                    break;
                case wxTAR_DIRTYPE:
                    if (!dstPath.IsEmpty() &&
                        !wxFileName::DirExists(name) &&
                        !wxFileName::Mkdir(name, 0700, wxPATH_MKDIR_FULL))
                    {
                        SetError(wxPROTO_INVVAL);
                        event.m_status |= CslProtocolInputEvent::ERROR_OUTPUT;
                    }
                    ::wxPostEvent(m_handler, event);
                    break;
                case wxTAR_FIFOTYPE:
                case wxTAR_CONTTYPE:
                    ::wxPostEvent(m_handler, event);
                    break;
                default:
                    break;
            }

            wxDELETE(entry);

            if (m_protocolError!=wxPROTO_NOERR)
            {
                wxDELETE(fp);
                break;
            }

            if (!dstPath.IsEmpty())
                filespecs.push_back(fp);
        }

        loopvrev(filespecs)
        {
            CslFileProperties *fp = (CslFileProperties*)filespecs[i];
            fp->Set();
            delete fp;
        }
    }
    else
    {
        CSL_LOG_DEBUG("%s\n", U2C(m_inputURI.BuildURI()));

        bool createOutput = m_outputPath.IsOk();

        if (m_outputPath.IsDir())
        {
            m_critSection.Enter();
            m_outputPath.SetFullName(archive.GetArchiveName());
            m_critSection.Leave();
        }

        event.m_fileProperties.Name = m_outputPath;
        event.m_fileProperties.Time.Get(wxFileName(srcPath));

        if (createOutput && !CreateOutputStream(m_outputPath))
        {
            SetError(wxPROTO_INVVAL);

            event.m_status |= CslProtocolInputEvent::ERROR_OUTPUT;
            ::wxPostEvent(m_handler, event);

            return;
        }

        if (ProcessInput(event) && createOutput)
        {
            wxDELETE(m_outputStream);
            event.m_fileProperties.Set();
    }
}

inline size_t CslProtocolInput::DoRead(void *buf, size_t count, bool& doRead)
{
#ifdef __WXGTK__
    bool connected = m_protocol->IsConnected();

    if (connected)
    {
#if !wxCHECK_VERSION(2, 9, 0)
        if (!m_protocol->Wait())
        {
            doRead = false;
            return 0;
        }
#endif
        wxMutexGuiEnter();
    }
#endif

    size_t read = m_inputStream->Read(buf, count).LastRead();

#ifdef __WXGTK__
    if (connected)
        wxMutexGuiLeave();
#endif

    if (!read || !m_inputStream->IsOk())
    {
        doRead = false;

        switch (m_inputStream->GetLastError())
        {
            case wxSTREAM_EOF:
                break;
#if !wxCHECK_VERSION(2, 9, 0)
                // wx 2.8 returns wxSTREAM_READ_ERROR if the socket was closed by the remote host
                // this may happen if all bytes were read. wx 2.9 handles it properly in this case
            case wxSTREAM_READ_ERROR:
                if (((wxSocketBase*)m_protocol)->Error() &&
                    ((wxSocketBase*)m_protocol)->IsDisconnected())
                    break;
#endif
            default:
                SetError(wxPROTO_NETERR);
                break;
        }
    }

    return read;
}


bool CslProtocolInput::ProcessInput(CslProtocolInputEvent& event)
{
    bool doRead = !m_terminate;

    while (doRead)
    {
        size_t left, read;

        {
            wxCriticalSectionLocker lock(m_critSection);

            if (m_totalSize)
                read = min(m_totalSize, m_maxRead);
            else
                read = m_maxRead;

            if (!(left = min(read-m_totalRead, m_chunkSize)))
                break;
        }

        read = 0;

        event.m_buffer = wxMemoryBuffer(left);

        char *buf = (char*)event.m_buffer.GetData();

        while (doRead && left)
        {
            size_t last = DoRead(buf + read, min(left, (size_t)1024), doRead);

            if (m_protocolError!=wxPROTO_NOERR)
            {
                doRead = false;
                event.m_status |= CslProtocolInputEvent::ERROR_INPUT;
                break;
            }

            left -= last;
            read += last;
        }

        m_critSection.Enter();
        event.m_bytesRead = (m_totalRead += read);
        m_critSection.Leave();

        event.m_buffer.UngetWriteBuf(read);

        if (m_outputStream && read)
        {
            m_outputStream->Write(buf, read);

            if (m_outputStream->LastWrite()!=read)
            {
                doRead = false;
                event.m_status |= CslProtocolInputEvent::ERROR_OUTPUT;

                SetError(wxPROTO_NETERR);
            }
        }

        if (doRead)
        {
            if (m_totalSize && m_totalSize==m_totalRead)
                doRead = false;
            else
                doRead = !m_terminate;

        }

        ::wxPostEvent(m_handler, event);
    }

    return m_protocolError==wxPROTO_NOERR;
}
