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

#include <wx/tokenzr.h>
#include "CslIRCEngine.h"
#include "engine/cube_tools.h"
#include "engine/CslVersion.h"

#define IRC_MAX_RENAME_TRIES  5

CslIrcNetworks g_CslIrcNetworks;

DEFINE_EVENT_TYPE(wxCSL_EVT_IRC)


wxColour IrcColours[16] =
{
    wxColour(255,255,255),
    wxColour(0,    0,  0),
    wxColour(0,    0,127),
    wxColour(0,  128,  0),
    wxColour(255,  0,  0),
    wxColour(127,  0,  0),
    wxColour(160,  0,160),
    wxColour(255,128,  0),
    wxColour(255,240,  0),
    wxColour(0,  250,  0),
    wxColour(0,  128,128),
    wxColour(0,  255,255),
    wxColour(0,   80,255),
    wxColour(255,  0,255),
    wxColour(127,127,127),
    wxColour(210,210,210)
};


void irc_auto_rename_nick(irc_session_t *session)
{
	static int tries=0;
    CslIrcContext *context=(CslIrcContext*)irc_get_ctx(session);

    if (++tries<2)
    {
        //context->Server->Network->Nick+=wxT("'");
		context->Server->Network->Nick=context->Server->Network->AltNick;
        irc_cmd_nick(session,U2A(context->Server->Network->AltNick));
    }
    else
        irc_cmd_quit(session,NULL);
}

void irc_notify_player_list(irc_session_t *session,const char *channel,const char *players)
{
    CslIrcContext *context=(CslIrcContext*)irc_get_ctx(session);

    wxString c=A2U(channel);
    wxString p=A2U(players);
    wxStringTokenizer tkz(p,wxT(" \t"));

    CslIrcEvent evt(context->Target,CslIrcEvent::NAMES,c);

    while (tkz.HasMoreTokens())
        evt.Strings.Add(tkz.GetNextToken());

    wxPostEvent(context->EvtHandler,evt);
}

void event_connect(irc_session_t *session,const char* WXUNUSED(event),const char *origin,
                   const char** WXUNUSED(params),unsigned int WXUNUSED(count))
{
    CslIrcContext *context=(CslIrcContext*)irc_get_ctx(session);

    if (!context || !context->EvtHandler)
        return;

    CslIrcEvent evt(context->Target,CslIrcEvent::CONNECT);
    wxPostEvent(context->EvtHandler,evt);
}

void event_join(irc_session_t *session,const char* WXUNUSED(event),const char *origin,
                const char **params,unsigned int WXUNUSED(count))
{
    CslIrcContext *context=(CslIrcContext*)irc_get_ctx(session);
    CslIrcEvent evt(context->Target);

    evt.Channel=A2U(params[0]);
    evt.Strings.Add(A2U(origin));
    evt.Type=context->Server->Network->Nick==A2U(origin) ?
             CslIrcEvent::JOIN : CslIrcEvent::JOINED;

    wxPostEvent(context->EvtHandler,evt);
}

void event_nick(irc_session_t *session, const char* WXUNUSED(event),
                const char *origin,const char **params,unsigned int count)
{
    CslIrcContext *context=(CslIrcContext*)irc_get_ctx(session);

    if (count<1)
        return;

    CslIrcEvent evt(context->Target,CslIrcEvent::NICK,A2U(origin));

    if (context->Server->Network->Nick==A2U(origin))
    {
        context->Server->Network->Nick=A2U(params[0]);
        evt.Ints.Add(1);
    }
    else
        evt.Ints.Add(0);

    evt.Strings.Add(A2U(params[0]));

    wxPostEvent(context->EvtHandler,evt);
}

void event_kick(irc_session_t *session,const char* WXUNUSED(event),
                const char *origin,const char **params,unsigned count)
{
    if (count<2)
        return;

    CslIrcContext *context=(CslIrcContext*)irc_get_ctx(session);
    CslIrcEvent evt(context->Target,CslIrcEvent::KICK,A2U(params[0]));

    evt.Strings.Add(A2U(origin));

    if (count>1)
    {
        evt.Strings.Add(A2U(params[1]));
        evt.Strings.Add(A2U(params[2]));
    }

    wxPostEvent(context->EvtHandler,evt);
}

void event_quit(irc_session_t *session,const char* WXUNUSED(event),
                const char *origin,const char **params,unsigned int count)
{
    CslIrcContext *context=(CslIrcContext*)irc_get_ctx(session);
    CslIrcEvent evt(context->Target,CslIrcEvent::QUIT,A2U(origin));
    if (count>0)
        evt.Strings.Add(A2U(params[0]));
    wxPostEvent(context->EvtHandler,evt);
}

void event_part(irc_session_t *session,const char* WXUNUSED(event),
                const char *origin,const char **params,unsigned int count)
{
    CslIrcContext *context=(CslIrcContext*)irc_get_ctx(session);
    CslIrcEvent evt(context->Target,CslIrcEvent::PART,A2U(params[0]));
    evt.Strings.Add(A2U(origin));
    if (count>1)
        evt.Strings.Add(A2U(params[1]));
    wxPostEvent(context->EvtHandler,evt);
}

void event_chanmsg(irc_session_t *session,const char* WXUNUSED(event),
                   const char *origin,const char **params,unsigned int count)
{
    if (count<2 || !*params[1])
        return;

    CslIrcContext *context=(CslIrcContext*)irc_get_ctx(session);
    CslIrcEvent evt(context->Target,CslIrcEvent::CHANMSG,A2U(params[0]));
    evt.Strings.Add(A2U(origin));
    evt.Strings.Add(A2U(params[1]));
    wxPostEvent(context->EvtHandler,evt);
}

void event_privmsg(irc_session_t *session,const char* WXUNUSED(event),
                   const char *origin,const char **params,unsigned int count)
{
    if (count<2 || !*params[1])
        return;

    CslIrcContext *context=(CslIrcContext*)irc_get_ctx(session);
    CslIrcEvent evt(context->Target,CslIrcEvent::PRIVMSG,A2U(origin));
    evt.Strings.Add(A2U(params[1]));
    wxPostEvent(context->EvtHandler,evt);
}

void event_notice(irc_session_t *session,const char* WXUNUSED(event),
                  const char *origin,const char **params,unsigned int count)
{
    if (count<2)
        return;

    CslIrcContext *context=(CslIrcContext*)irc_get_ctx(session);
    CslIrcEvent evt(context->Target,CslIrcEvent::NOTICE,A2U(params[0]));
    evt.Strings.Add(A2U(origin));
    evt.Strings.Add(A2U(params[1]));
    wxPostEvent(context->EvtHandler,evt);
}

void event_ctcp_request(irc_session_t *session,const char* WXUNUSED(event),const char *origin,
                        const char **params,unsigned int WXUNUSED(count))
{
    char nick[128];

    irc_target_get_nick(origin,nick,sizeof(nick));

    if (!strcasecmp(params[0],"VERSION"))
    {
        wxString s;
        s << wxT("VERSION ") << CSL_NAME_STR << wxT(" ") << CSL_VERSION_LONG_STR << CSL_WEBADDRFULL_STR;
        irc_cmd_ctcp_reply(session,nick,U2A(s));
    }
}

void event_ctcp_action(irc_session_t *session,const char* WXUNUSED(event),const char *origin,
                       const char **params,unsigned int count)
{
    if (count<2) //we don't have the patched version of libircclient
        return;

    CslIrcContext *context=(CslIrcContext*)irc_get_ctx(session);
    CslIrcEvent evt(context->Target,CslIrcEvent::ACTION,A2U(params[0]));
    evt.Strings.Add(A2U(origin));
    evt.Strings.Add(A2U(params[1]));
    wxPostEvent(context->EvtHandler,evt);
}

void event_mode(irc_session_t *session,const char* WXUNUSED(event),const char *origin,
                const char **params,unsigned int count)
{
    if (count<3)
        return;

    CslIrcContext *context=(CslIrcContext*)irc_get_ctx(session);
    CslIrcEvent evt(context->Target,CslIrcEvent::MODE,A2U(params[0]));
    evt.Strings.Add(A2U(origin));
    evt.Strings.Add(A2U(params[2]));
    evt.Strings.Add(A2U(params[1]));
    wxPostEvent(context->EvtHandler,evt);
}

void event_topic(irc_session_t *session,const char* WXUNUSED(event),const char *origin,
                 const char **params,unsigned int count)
{
    if (count<2)
        return;

    CslIrcContext *context=(CslIrcContext*)irc_get_ctx(session);
    CslIrcEvent evt(context->Target,CslIrcEvent::TOPIC,A2U(params[0]));
    evt.Strings.Add(A2U(origin));
    evt.Strings.Add(A2U(params[1]));
    wxPostEvent(context->EvtHandler,evt);
}

void event_numeric(irc_session_t *session,unsigned int event,const char *origin,
                   const char **params, unsigned count)
{
    CslIrcContext *context=(CslIrcContext*)irc_get_ctx(session);

    switch (event)
    {
        case LIBIRC_RFC_ERR_NICKNAMEINUSE:
        case LIBIRC_RFC_ERR_NICKCOLLISION:
            irc_auto_rename_nick(session);
            return;
        case LIBIRC_RFC_RPL_TOPIC: //topic notification on join
            if (count>2)
            {
                CslIrcEvent evt(context->Target,CslIrcEvent::TOPIC,A2U(params[1]));
				evt.Ints.Add(event);
                evt.Strings.Add(A2U(params[2]));
                wxPostEvent(context->EvtHandler,evt);
            }
            return;
        case LIBIRC_RFC_RPL_NAMREPLY:
            if (count>3)
            {
                irc_notify_player_list(session,params[2],params[3]);
                return;
            }
            break;
        case LIBIRC_RFC_RPL_ENDOFNAMES:
            return;

        case LIBIRC_RFC_ERR_CANNOTSENDTOCHAN:
            if (count>2)
            {
                CslIrcEvent evt(context->Target,CslIrcEvent::NUMERIC,A2U(params[1]));
				evt.Ints.Add(event);
                evt.Strings.Add(A2U(params[2]));
                wxPostEvent(context->EvtHandler,evt);
                return;
            }
            break;
    }

    wxUint32 i;
    wxString s,p;

    s << event << wxT(": ");

    for (i=0;i<count;i++)
    {
        p=A2U(params[i]);
        if (!p.CmpNoCase(context->Server->Network->Nick))
            continue;

        s << p << wxT(" ");
    }

    CslIrcEvent evt(context->Target,CslIrcEvent::NUMERIC,s);
    evt.Ints.Add(event);
    wxPostEvent(context->EvtHandler,evt);
}


#include <errno.h>

CslIrcThread::CslIrcThread() :
        wxThread(wxTHREAD_JOINABLE),m_condition(NULL),m_terminate(false),m_ok(false)
{
    if (!(m_ok=Create()==wxTHREAD_NO_ERROR))
        return;

    m_condition=new wxCondition(m_mutex);
}

CslIrcThread::~CslIrcThread()
{
    if (m_condition)
        delete m_condition;
}

wxThread::ExitCode CslIrcThread::Entry()
{
    int maxfd,error;
    struct timeval timeout;
    fd_set readSet,writeSet;
    CslIrcContext *context=NULL;
    wxUint32 count,pos=0;
    wxInt32 termcount=10;

    m_mutex.Lock();

    while (termcount)
    {
        if (m_terminate)
            termcount--;

        m_section.Enter();
        count=m_contexts.GetCount();
        if (count)
            context=m_contexts.Item((pos=pos+1>=count ? 0:pos+1));
        else
            context=NULL;
        m_section.Leave();

        if (!context)
        {
            if (!m_terminate)
                m_condition->Wait();
            continue;
        }

        if (!context->Disconnecting && !irc_is_connected(context->Session))
        {
            if (irc_connect(context->Session,U2A(context->Server->Address),
                            context->Server->Port,NULL,U2A(context->Server->Network->Nick),
                            U2A(CSL_NAME_SHORT_STR),NULL))
            {
                LibIrcError(context,irc_errno(context->Session));
                continue;
            }
        }

        FD_ZERO(&readSet);
        FD_ZERO(&writeSet);
        timeout.tv_sec=0;
        timeout.tv_usec=max(50000,200000/count);
        maxfd=0;

        irc_add_select_descriptors(context->Session,&readSet,&writeSet,&maxfd);

        if (select(maxfd+1,&readSet,&writeSet,NULL,&timeout)<0)
        {
            LOG_DEBUG("select failed: %s\n",strerror(errno));
            if (errno==EINTR)
                continue;
        }

        if (irc_process_select_descriptors(context->Session,&readSet,&writeSet))
            if ((error=irc_errno(context->Session)))
                LibIrcError(context,error);
    }

    m_mutex.Unlock();

    return 0;
}

CslIrcContext* CslIrcThread::FindContext(const CslIrcContext& context)
{
    wxUint32 i;

    for (i=0;i<m_contexts.GetCount();i++)
    {
        if (*m_contexts.Item(i)==context)
            return m_contexts.Item(i);
    }

    return NULL;
}

bool CslIrcThread::Connect(const CslIrcContext& context)
{
    m_section.Enter();

    if (FindContext(context))
        return false;

    CslIrcContext *ctx=new CslIrcContext(context);

    m_contexts.Add(ctx);
    irc_set_ctx(ctx->Session,ctx);

    m_section.Leave();

    if (m_mutex.TryLock()!=wxMUTEX_NO_ERROR)
        return true;
    m_condition->Signal();
    m_mutex.Unlock();

    return true;
}

bool CslIrcThread::Disconnect(const CslIrcContext& context)
{
    wxCriticalSectionLocker enter(m_section);

    CslIrcContext *ctx;

    if (!(ctx=FindContext(context)))
        return false;

    ctx->Disconnecting=true;

    return true;
}

bool CslIrcThread::RemoveContext(CslIrcContext *context)
{
    wxCriticalSectionLocker lock(m_section);

    wxInt32 i;

    if ((i=m_contexts.Index(context))==wxNOT_FOUND)
        return false;

    m_contexts.RemoveAt(i);

    irc_destroy_session(context->Session);

    delete context;

    return true;
}

void CslIrcThread::Terminate()
{
    m_terminate=true;
    wxMutexLocker lock(m_mutex);
    m_condition->Signal();
}

void CslIrcThread::LibIrcError(CslIrcContext *context,wxInt32 error)
{
    LOG_DEBUG("libirc error (%d): \n",error,irc_strerror(errno));

	wxEvtHandler *handler=context->EvtHandler;
	CslIrcEvent evt(context->Target,CslIrcEvent::ERR);
    evt.Strings.Add(A2U(irc_strerror(error)));
    evt.Ints.Add(error);

    switch (error)
    {
        case LIBIRC_ERR_INVAL:
            wxASSERT_MSG(error!=LIBIRC_ERR_INVAL,wxT("Invalid arguments."));
            break;
        case LIBIRC_ERR_RESOLV:
        case LIBIRC_ERR_SOCKET:
        case LIBIRC_ERR_CONNECT:
        case LIBIRC_ERR_CLOSED:
        case LIBIRC_ERR_NOMEM:
            RemoveContext(context);
            break;
        case LIBIRC_ERR_ACCEPT:
        case LIBIRC_ERR_NODCCSEND:
        case LIBIRC_ERR_READ:
        case LIBIRC_ERR_WRITE:
        case LIBIRC_ERR_STATE:
        case LIBIRC_ERR_TIMEOUT:
        case LIBIRC_ERR_OPENFILE:
            break;
        case LIBIRC_ERR_TERMINATED:
            RemoveContext(context);
            break;
    }

    wxPostEvent(handler,evt);
}


BEGIN_EVENT_TABLE(CslIrcSession,wxEvtHandler)
    CSL_EVT_IRC(wxID_ANY,CslIrcSession::OnIrcEvent)
END_EVENT_TABLE()

CslIrcSession::CslIrcSession() :
        m_state(STATE_DISCONNECTED)
{
}

CslIrcSession::~CslIrcSession()
{
    WX_CLEAR_ARRAY(m_channels);
}

void CslIrcSession::OnIrcEvent(CslIrcEvent& event)
{
    switch (event.Type)
    {
        case CslIrcEvent::ERR:
        {
            switch (event.Ints.Item(0))
            {
                case LIBIRC_ERR_RESOLV:
                case LIBIRC_ERR_SOCKET:
                case LIBIRC_ERR_CONNECT:
                case LIBIRC_ERR_CLOSED:
                case LIBIRC_ERR_NOMEM:
                case LIBIRC_ERR_TERMINATED:
                    for (wxUint32 i=0;i<m_channels.GetCount();i++)
                        m_channels.Item(i)->Connected=false;
                    if (m_state==STATE_DISCONNECTED)
                        return;
					m_state=STATE_DISCONNECTED;
                    break;
            }
            break;
        }

        case CslIrcEvent::CONNECT:
            m_state=STATE_CONNECTED;
            break;

        case CslIrcEvent::JOIN:
            for (wxUint32 i=0;i<m_channels.GetCount();i++)
            {
                if (m_channels.Item(i)->Name==event.Channel)
                {
                    m_channels.Item(i)->Connected=true;
                    break;
                }
            }
            break;

    }

    event.Skip();
}

bool CslIrcSession::InitContext(CslIrcContext *context)
{
    irc_callbacks_t callbacks;

    memset(&callbacks,0,sizeof(callbacks));

    callbacks.event_connect=event_connect;
    callbacks.event_join=event_join;
    callbacks.event_nick=event_nick;
    callbacks.event_quit=event_quit;
    callbacks.event_part=event_part;
    callbacks.event_mode=event_mode;
    callbacks.event_topic=event_topic;
    callbacks.event_kick=event_kick;
    callbacks.event_channel=event_chanmsg;
    callbacks.event_privmsg=event_privmsg;
    callbacks.event_notice=event_notice;
    //callbacks.event_invite
    //callbacks.event_umode
    callbacks.event_ctcp_req=event_ctcp_request;
    //callbacks.event_ctcp_rep;
    callbacks.event_ctcp_action=event_ctcp_action;
    //callbacks.event_unknown
    callbacks.event_numeric=event_numeric;
    //callbacks.event_dcc_chat_req
    //callbacks.event_dcc_send_req

    if (!(context->Session=irc_create_session(&callbacks)))
        return false;

    //nicknames only, strip them from nick!host.
    irc_option_set(context->Session,LIBIRC_OPTION_STRIPNICKS);

    context->EvtHandler=this;

    SetNextHandler(context->Target);

    return true;
}

bool CslIrcSession::Connect(const CslIrcContext& context)
{
    m_context=context;

    if (InitContext(&m_context) && CslIrcEngine::GetThread()->Connect(m_context))
    {
        m_state=STATE_CONNECTING;
        return true;
    }

    return false;
}

bool CslIrcSession::Disconnect(const wxString& message)
{
    if (!CslIrcEngine::GetThread() || m_state==STATE_DISCONNECTED)
        return false;

    wxString s;

    if (message.IsEmpty())
        s << CSL_NAME_STR << wxT(" ") << CSL_VERSION_LONG_STR << CSL_WEBADDRFULL_STR;
    else
        s=message;

    m_state=STATE_DISCONNECTED;

    irc_cmd_quit(m_context.Session,U2A(s));

    CslIrcEngine::GetThread()->Disconnect(m_context);

    return true;
}

bool CslIrcSession::SendTextMessage(const wxString& channel,const wxString& text)
{
    if (m_state!=STATE_CONNECTED)
        return false;

    return irc_cmd_msg(m_context.Session,U2A(channel),U2A(text))==0;
}

bool CslIrcSession::JoinChannel(const wxString& channel,const wxString& password)
{
    if (m_state!=STATE_CONNECTED)
        return false;

    bool add=true;

    for (wxUint32 i=0;i<m_channels.GetCount();i++)
    {
        CslIrcChannel *chan=m_channels.Item(i);
        if (chan->Name==channel)
        {
            add=false;
            if (chan->Connected)
                return false;
            else
                break;
        }
    }
    if (add)
        m_channels.Add(new CslIrcChannel(channel,password));

    return irc_cmd_join(m_context.Session,U2A(channel),U2A(password))==0;
}

bool CslIrcSession::LeaveChannel(const wxString& channel,const wxString& reason)
{
    if (!CslIrcEngine::GetThread() || m_state!=STATE_CONNECTED)
        return false;

    wxString command;

    for (wxUint32 i=0;i<m_channels.GetCount();i++)
    {
        CslIrcChannel *chan=m_channels.Item(i);
        if (chan->Name==channel)
        {
            m_channels.RemoveAt(i);
            if (chan->Connected)
            {
                command << wxT("PART ") << channel;
                if (!reason.IsEmpty())
                    command << wxT(" ") << reason;
                break;
            }
            else
                return false;
        }
    }

    return SendRawCommand(command);
}

bool CslIrcSession::ChangeNick(const wxString& nick)
{
    if (!CslIrcEngine::GetThread())
        return false;

    if (m_state==STATE_DISCONNECTED)
    {
        m_context.Server->Network->Nick=nick;
        return true;
    }

    return irc_cmd_nick(m_context.Session,U2A(nick))==0;
}

bool CslIrcSession::SendCtcpAction(const wxString& channel,const wxString& text)
{
    if (!CslIrcEngine::GetThread() || m_state!=STATE_CONNECTED)
        return false;

    return irc_cmd_me(m_context.Session,U2A(channel),U2A(text))==0;
}

bool CslIrcSession::SendRawCommand(const wxString& command)
{
    if (!CslIrcEngine::GetThread() || m_state!=STATE_CONNECTED)
        return false;

    return irc_send_raw(m_context.Session,U2A(command))==0;
}


CslIrcThread* CslIrcEngine::m_thread=NULL;

CslIrcEngine::CslIrcEngine()
{
}

CslIrcEngine::~CslIrcEngine()
{
}

bool CslIrcEngine::InitEngine()
{
    if (m_thread)
    {
        wxASSERT_MSG(m_thread==NULL,wxT("Engine already initialised"));
        return false;
    }

    m_thread=new CslIrcThread;

    if (m_thread->IsOk() && m_thread->Run()==wxTHREAD_NO_ERROR)
        return true;

    return false;
}

void CslIrcEngine::DeinitEngine()
{
    wxASSERT_MSG(m_thread!=NULL,wxT("Engine not initialised"));

    wxUint32 i;

    for (i=m_sessions.GetCount();i>0;i--)
        m_sessions.Item(i-1)->Disconnect();

    if (m_thread)
    {
        m_thread->Terminate();
        m_thread->Wait();
        delete m_thread;
    }

    for (i=m_sessions.GetCount();i>0;i--)
        DestroySession(m_sessions.Item(i-1));
}

CslIrcSession* CslIrcEngine::CreateSession()
{
    m_sessions.Add(new CslIrcSession);

    return m_sessions.Last();
}

bool CslIrcEngine::DestroySession(CslIrcSession *session)
{
    bool ret;
    wxInt32 id;

    if ((id=m_sessions.Index(session))==wxNOT_FOUND)
        return false;

    ret=session->Disconnect();
    delete session;

    return ret;
}
