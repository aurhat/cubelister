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

#ifndef CSLIRCENGINE_H
#define CSLIRCENGINE_H

#include <libircclient.h>
#include <libirc_rfcnumeric.h>

/**
 @author Glen Masgai <mimosius@users.sourceforge.net>
*/


extern wxColour IrcColours[16];

class CslIrcEvent;
class CslIrcChannel;
class CslIrcServer;
class CslIrcNetwork;
class CslIrcContext;
class CslIrcSession;
class CslIrcEngine;

WX_DEFINE_ARRAY(CslIrcChannel*,CslIrcChannels);
WX_DEFINE_ARRAY(CslIrcServer*,CslIrcServers);
WX_DEFINE_ARRAY(CslIrcNetwork*,CslIrcNetworks);
WX_DEFINE_ARRAY(CslIrcContext*,CslIrcContexts);
WX_DEFINE_ARRAY(CslIrcSession*,CslIrcSessions);


BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE(wxCSL_EVT_IRC,wxID_ANY)
END_DECLARE_EVENT_TYPES()

typedef void (wxEvtHandler::*CslIrcEventEventFunction)(CslIrcEvent&);

#define CSL_EVT_IRC(id,fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
                               wxCSL_EVT_IRC,id,wxID_ANY, \
                               (wxObjectEventFunction)(wxEventFunction) \
                               wxStaticCastEvent(CslIrcEventEventFunction,&fn), \
                               (wxObject*)NULL \
                             ),


extern CslIrcNetworks g_CslIrcNetworks;

WX_DEFINE_ARRAY_PTR(char*,CharPointerArray);

class CslIrcEvent : public wxEvent
{
    public:
        enum
        {
            ERR, CONNECT, NOTICE, JOIN, JOINED, NAMES, NICK, KICK,
            QUIT, PART, CHANMSG, PRIVMSG, ACTION, MODE, TOPIC, NUMERIC
        };

        CslIrcEvent(wxEvtHandler *target,wxInt32 type=-1,
                    const wxString& channel=wxEmptyString) :
                wxEvent(wxID_ANY,wxCSL_EVT_IRC),
                Target(target),Type(type),Channel(channel) {}

        ~CslIrcEvent()
        {
            char *data;
            for (wxUint32 i=0;i<CharData.GetCount();i++)
            {
                if ((data=CharData.Item(i)))
                    free(data);
            }
            CharData.Empty();
        }

        virtual wxEvent* Clone() const
        {
            return new CslIrcEvent(*this);
        }

        void AddCharData(const char *data)
        {
            CharData.Add(strdup(data ? data:""));
        }

        wxEvtHandler *Target;
        wxInt32 Type;
        wxString Channel;
        wxArrayInt Ints;
        wxArrayString Strings;
        CharPointerArray CharData;

    private:
        CslIrcEvent(const CslIrcEvent& event) : wxEvent(wxID_ANY,wxCSL_EVT_IRC)
        {
            Target=event.Target;
            Type=event.Type;
            Channel=event.Channel;
            Ints=event.Ints;
            Strings=event.Strings;

            for (wxUint32 i=0;i<event.CharData.GetCount();i++)
                AddCharData(event.CharData.Item(i));
        }
};


class CslIrcUser
{
    public:
        enum { STATUS_USER, STATUS_VOICE = 0x01, STATUS_OP=0x02 };

        CslIrcUser(const wxString& nick=wxEmptyString,wxUint32 status=STATUS_USER) :
                Nick(nick),Status(status) {}

        wxString Nick;
        wxUint32 Status;
};


class CslIrcChannel
{
    public:
        CslIrcChannel(const wxString& name=wxEmptyString,
                      const wxString& password=wxEmptyString) :
                Connected(false),Name(name),Password(password) {}

        bool operator==(const CslIrcChannel& channel)
        {
            return Name.CmpNoCase(channel.Name)==0;
        }
        bool operator==(const wxString& name)
        {
            return Name.CmpNoCase(name)==0;
        }

        bool Connected;
        wxString Name,Password;
        wxString Topic;
        CslCharEncoding Encoding;
};


class CslIrcServer
{
    public:
        CslIrcServer(CslIrcNetwork *network,const wxString& address,wxUint16 port) :
                Network(NULL),Address(address),Port(port) {}

        CslIrcNetwork *Network;
        wxString Address;
        wxUint16 Port;
};


class CslIrcNetwork
{
    public:
        CslIrcNetwork(const wxString& name,const wxString& nick,const wxString& altnick) :
                Name(name),Nick(nick),AltNick(altnick) {}
        ~CslIrcNetwork() { WX_CLEAR_ARRAY(Servers); WX_CLEAR_ARRAY(AutoChannels); }

        wxString Name;
        wxString Nick,AltNick;
        CslIrcServers Servers;
        CslIrcChannels AutoChannels;

        void AddServer(CslIrcServer *server) { server->Network=this; Servers.Add(server); }
        void AddAutoChannel(CslIrcChannel *channel) { AutoChannels.Add(channel); }
};


class CslIrcContext
{
    public:
        CslIrcContext() {};
        CslIrcContext(CslIrcServer *server,wxEvtHandler *target) :
                Session(NULL),Server(server),Target(target),
                Disconnecting(false) {}

        bool operator==(const CslIrcContext& context) const
        {
            return Session==context.Session;
        }

        irc_session_t *Session;
        CslIrcServer *Server;
        wxEvtHandler *EvtHandler,*Target;
        bool Disconnecting;
};


class CslIrcThread : public wxThread
{
    public:
        CslIrcThread();
        ~CslIrcThread();

        bool Connect(const CslIrcContext& context);
        bool Disconnect(const CslIrcContext& context);
        void Terminate();
        bool IsOk() const { return m_ok; }

    protected:
        CslIrcContexts m_contexts;
        wxMutex m_mutex;
        wxCondition *m_condition;
        wxCriticalSection m_section;
        bool m_terminate,m_ok;

        CslIrcContext* FindContext(const CslIrcContext& context);
        bool RemoveContext(CslIrcContext *context);
        void LibIrcError(CslIrcContext *context,wxInt32 error);

        virtual wxThread::ExitCode Entry();
};


class CslIrcSession : public wxEvtHandler
{
        friend class CslIrcEngine;

    public:
        enum { STATE_CONNECTING, STATE_CONNECTED, STATE_DISCONNECTED };

        bool Connect(const CslIrcContext& context);
        bool Disconnect(const wxString& message=wxEmptyString);

        bool SendTextMessage(const wxString& channel,const wxString& text);
        bool JoinChannel(const wxString& name,const wxString& password);
        bool LeaveChannel(const wxString& name,const wxString& reason=wxEmptyString);
        bool ChangeNick(const wxString& nick);
        bool SendCtcpAction(const wxString& channel,const wxString& text);
        bool SendRawCommand(const wxString& command);

        wxInt32 GetState() const { return m_state; }
        wxString GetNick() { return m_context.Server->Network->Nick; }
        CslIrcChannels GetChannels() { return m_channels; }
        const CslIrcContext& GetContext() const { return m_context; }

        CslIrcChannel* FindChannel(const wxString& name);
        CslIrcChannel* FindChannel(const CslIrcChannel& channel);
        bool ConvertToChannelEncoding(const wxString& name,const wxString& text,
                                      wxCharBuffer& buffer);

    private:
        wxInt32 m_state;
        CslIrcChannels m_channels;
        CslIrcContext m_context;

        void OnIrcEvent(CslIrcEvent& event);

        DECLARE_EVENT_TABLE()

    protected:
        CslIrcSession();
        ~CslIrcSession();

        bool InitContext(CslIrcContext *context);
};


class CslIrcEngine : public wxEvtHandler
{
    public:
        CslIrcEngine();
        ~CslIrcEngine();

        bool InitEngine();
        void DeinitEngine();

        CslIrcSession* CreateSession();
        bool DestroySession(CslIrcSession *session);

        static CslIrcThread* GetThread() { return m_thread; }

    private:
        CslIrcSessions m_sessions;
        static CslIrcThread *m_thread;
};

#endif //CSLIRCENGINE_H
