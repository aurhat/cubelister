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

#ifndef CSLPROTOCOLINPUT_H
#define CSLPROTOCOLINPUT_H

#include <wx/protocol/file.h>
#include <wx/protocol/ftp.h>
#include <wx/protocol/http.h>
#include <wx/uri.h>
#include <wx/zstream.h>

#define CSL_PROTO_INPUT_CHUNK_SIZE  (512 * 1024)

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EXPORTED_EVENT_TYPE(CSL_DLL_TOOLS, wxCSL_EVT_PROTO_INPUT, wxID_ANY)
END_DECLARE_EVENT_TYPES()

class CslProtocolInputEvent;

typedef void (wxEvtHandler::*CslProtocolInputEventFunction)(CslProtocolInputEvent&);

#define CslProtocolInputEventHandler(fn) \
    (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(CslProtocolInputEventFunction, &fn)

#define CSL_EVT_PROTO_INPUT(id, fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
                               wxCSL_EVT_PROTO_INPUT, id, wxID_ANY, \
                               CslProtocolInputEventHandler(fn), \
                               (wxObject*)NULL \
                             ),

class CSL_DLL_TOOLS CslProtocolInputEvent : public wxEvent
{
    friend class CslProtocolInput;

    public:
        enum
        {
            ERROR_INPUT  = 1<<0,
            ERROR_OUTPUT = 1<<1,
            TERMINATE    = 1<<2,
        };

        CslProtocolInputEvent(wxInt32 id = wxID_ANY,
                              void *clientData = NULL) :
                wxEvent(id, wxCSL_EVT_PROTO_INPUT),
                m_status(0),
                m_statusCode(0),
                m_totalSize(0),
                m_bytesRead(0),
                m_buffer(0),
                m_clientData(clientData)
        { }

        bool IsError() { return IsInputError() || IsOutputError(); }
        bool IsInputError() { return (m_status & ERROR_INPUT) != 0; }
        bool IsOutputError() { return (m_status & ERROR_OUTPUT) != 0; }
        bool IsTerminate() { return (m_status & CslProtocolInputEvent::TERMINATE) != 0; }

        wxUint32 GetStatus() const { return m_status; }
        wxInt32 GetStatusCode() const { return m_statusCode; }
        size_t GetTotalSize() const { return m_totalSize; }
        size_t GetBytesRead() const { return m_bytesRead; }
        CslFileProperties& GetFileProperties() { return m_fileProperties; }
        CslMemoryBuffer& GetBuffer() { return m_buffer; }
        void* GetClientData() { return m_clientData; }

    protected:
        virtual wxEvent* Clone() const
            { return new CslProtocolInputEvent(*this); }

        wxUint32 m_status;
        wxInt32 m_statusCode;
        size_t m_totalSize;
        size_t m_bytesRead;
        CslFileProperties m_fileProperties;
        CslMemoryBuffer m_buffer;
        void *m_clientData;

    private:
        DECLARE_DYNAMIC_CLASS_NO_ASSIGN(CslProtocolInputEvent)
};


class CSL_DLL_TOOLS CslTcpProto : public wxProtocol
{
    public:
        CslTcpProto() { }
        virtual ~CslTcpProto()
        {
#if !wxCHECK_VERSION(2, 9, 0)
            m_lastError = wxPROTO_NOERR;
#endif
        }

#if !wxCHECK_VERSION(2, 9, 0)
        wxProtocolError GetError() { return m_lastError; }
#endif
        bool Abort() { return true; }
        wxString GetContentType() const { return wxEmptyString; }

        wxInputStream* GetInputStream(const wxString& path);

    protected:
#if !wxCHECK_VERSION(2, 9, 0)
        wxProtocolError m_lastError;
#endif
        DECLARE_PROTOCOL(CslTcpProto);
        DECLARE_DYNAMIC_CLASS_NO_COPY(CslTcpProto)
};


class CSL_DLL_TOOLS CslArchiveProto : public wxProtocol
{
    public:
        CslArchiveProto()
        {
            m_isArchive = false;
#if !wxCHECK_VERSION(2, 9, 0)
            m_lastError = wxPROTO_NOERR;
#endif
        }
        virtual ~CslArchiveProto() { };

#if !wxCHECK_VERSION(2, 9, 0)
        wxProtocolError GetError() { return m_lastError; }
#endif
        bool Abort() { return true; }
        wxString GetContentType() const { return wxEmptyString; }

        wxInputStream* GetInputStream(const wxString& path);

        bool IsArchive() const { return m_isArchive; }
        wxString GetArchiveName() const { return m_archiveName; }

    protected:
        template<class T>
        wxInputStream* GetFactoryStream(wxString& path, wxInputStream *iStream = NULL);

        bool m_isArchive;
        wxString m_archiveName;

#if !wxCHECK_VERSION(2, 9, 0)
        wxProtocolError m_lastError;
#endif
        DECLARE_PROTOCOL(CslArchiveProto);
        DECLARE_DYNAMIC_CLASS_NO_COPY(CslArchiveProto)
};


class CSL_DLL_TOOLS CslProtocolInput : public wxThread, public wxObject
{
    public:
        CslProtocolInput()
            { Create(NULL, wxID_ANY, wxURI()); }
        CslProtocolInput(wxEvtHandler *handler, wxInt32 id,
                         const wxURI& uri,
                         const wxFileName& output = wxFileName(),
                         const wxDateTime& modifiedSince = wxDefaultDateTime) :
                wxThread(wxTHREAD_JOINABLE),
                m_protocol(NULL),
                m_inputStream(NULL),
                m_outputStream(NULL)
            { Create(handler, id, uri, output, modifiedSince); }

        ~CslProtocolInput();

        bool Create(wxEvtHandler *handler, wxInt32 id,
                    const wxURI& uri,
                    const wxFileName& output = wxFileName(),
                    const wxDateTime& modifiedSince = wxDefaultDateTime);

        wxThreadError Run();
        void DoTerminate();

        bool Error() { return GetError()!=wxPROTO_NOERR; }

        size_t GetReadSize();
        size_t GetTotalSize();
        size_t GetMaxRead() const { return m_maxRead; }
        size_t GetChunkSize() const { return m_chunkSize; }
        void* GetClientData() const { return m_clientData; }
        wxProtocolError GetError();
        wxURI GetInput() const { return m_inputURI; }
        wxFileName GetOutput();
        wxString GetHeader(const wxString& header);

        void SetMaxRead(size_t size);
        void SetChunkSize(size_t size);
        void SetClientData(void *data);
        bool SetOutput(const wxFileName& fileName);
        bool SetHeader(const wxString& header, const wxString& data);

    protected:
        void SetTotalSize();
        void SetError(wxProtocolError error);
        bool CreateOutputStream(const wxFileName& fileName);

        // all threaded
        wxThread::ExitCode Entry();
        void HandleProto(wxHTTP& http, CslProtocolInputEvent& event);
        void HandleProto(wxFTP& ftp, CslProtocolInputEvent& event);
        void HandleProto(CslTcpProto& tcp, CslProtocolInputEvent& event);
        void HandleProto(wxFileProto& file, CslProtocolInputEvent& event);
        void HandleProto(CslArchiveProto& archive, CslProtocolInputEvent& event);
        bool ProcessInput(CslProtocolInputEvent& event);
        size_t DoRead(void *buf, size_t count, bool& doRead);

        wxCriticalSection m_critSection;

        wxEvtHandler *m_handler;
        wxInt32 m_id;

        bool m_terminate;

        void *m_clientData;

        size_t m_maxRead;
        size_t m_totalRead;
        size_t m_totalSize;
        size_t m_chunkSize;

        wxInt32 m_statusCode;

        wxDateTime m_modifiedSince;

        wxProtocol *m_protocol;
        wxProtocolError m_protocolError;

        wxURI m_inputURI;
        wxFileName m_outputPath;
        wxInputStream *m_inputStream;
        wxOutputStream *m_outputStream;

    private:
        DECLARE_DYNAMIC_CLASS_NO_ASSIGN(CslProtocolInput)
};

WX_DEFINE_USER_EXPORTED_ARRAY(CslProtocolInput*, CslArrayCslProtocolInput, class CSL_DLL_TOOLS);

#endif
