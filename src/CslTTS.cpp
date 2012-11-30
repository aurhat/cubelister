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

#if defined(__WXMSW__)
#include <sapi.h>
#elif defined(__WXMAC__)
#include <Carbon/Carbon.h>
#elif defined(HAVE_CONFIG_H)
#ifdef HAVE_LIBSPEECHD_H
#include <libspeechd.h>
#endif //HAVE_LIBSPEECHD_H
#endif //__WXMSW__

#include "CslTTS.h"
#include "CslSettings.h"

#if defined(__WXMSW__)
    ISpVoice *g_voice;
#elif defined(__WXMAC__)
    SpeechChannel g_channel;
#elif defined(HAVE_LIBSPEECHD_H)
    SPDConnection *g_spd;
#endif //__WXMSW__


CslTTS::CslTTS()
{
    m_ok=false;
    m_volume=0;
#if defined(__WXMSW__)
    g_voice=NULL;
#elif defined(__WXMAC__)
#elif defined(HAVE_LIBSPEECHD_H)
    g_spd=NULL;
#endif //__WXMSW__

}

CslTTS& CslTTS::GetInstance()
{
    static CslTTS self;

    return self;
}

bool CslTTS::Init(const wxString& lang)
{
    CslTTS& self=GetInstance();

    if (self.m_ok)
        return false;

#if defined(__WXMSW__)
    if (CoCreateInstance(CLSID_SpVoice,NULL,CLSCTX_ALL,IID_ISpVoice,(void**)&g_voice)<0)
        g_voice=NULL;
    else
        self.m_ok=true;
#elif defined(__WXMAC__)
    self.m_ok=true;

    if (NewSpeechChannel(NULL,&m_channel)==noErr)
    {
        SetSpeechInfo(m_channel,soSpeechDoneCallBack,(void*)OnProcessed);
        self.Connect(wxEVT_IDLE,wxIdleEventHandler(CslTTS::OnIdle),NULL,&self);
    }
    else
        DeInit();
#elif defined(HAVE_LIBSPEECHD_H)
    if ((g_spd=spd_open(__CSL_NAME_SHORT_STR,NULL,NULL,SPD_MODE_THREADED)))
    {
        if (spd_set_punctuation(g_spd,SPD_PUNCT_NONE))
        {
            LOG_DEBUG("Failed to set punctuation mode.\n");
        }
        if (spd_set_spelling(g_spd,SPD_SPELL_ON))
        {
            LOG_DEBUG("Failed to set spelling mode.\n");
        }
#if 0 //disable for now
        if (!lang.IsEmpty() && spd_set_language(g_spd,U2A(lang)))
        {
            LOG_DEBUG("Failed to set language \"%s\"\n",U2A(lang));
        }
#endif
    }
    else
    {
        LOG_DEBUG("Failed to connect to speech-dispatcher.\n");
        return false;
    }

    self.m_ok=true;
#endif //__WXMSW__

    return self.m_ok;
}

bool CslTTS::DeInit()
{
    CslTTS& self=GetInstance();

    if (!self.m_ok)
        return false;

#if defined(__WXMSW__)
    if (g_voice)
    {
        g_voice->Release();
        g_voice=NULL;
    }
#elif defined(__WXMAC__)
    if (m_channel)
    {
        DisposeSpeechChannel(m_channel);
        m_channel=NULL;
    }
#elif defined(HAVE_LIBSPEECHD_H)
    if (g_spd)
    {
        spd_close(g_spd);
        g_spd=NULL;
    }
#endif //__WXMSW__

    self.m_ok=false;

    return true;
}

bool CslTTS::IsOk()
{
    return GetInstance().m_ok;
}

void CslTTS::Say(const wxString& text)
{
    CslTTS& self=GetInstance();

    if (!self.m_ok || !CslGetSettings().TTS || text.IsEmpty())
        return;

    if (self.m_volume!=CslGetSettings().TTSVolume)
        SetVolume(CslGetSettings().TTSVolume);

#if defined(__WXMSW__)
    if (g_voice)
        g_voice->Speak(text.wc_str(wxConvLocal),SPF_ASYNC,NULL);
#elif defined(__WXMAC__)
    self.Process(text);
#elif defined(HAVE_LIBSPEECHD_H)
    if (g_spd)
    {
        if (spd_say(g_spd,SPD_MESSAGE,U2A(text))<=0)
        {
            if (DeInit())
                Init(self.m_lang);
        }
    }
#endif //__WXMSW__
}

int CslTTS::GetVolume(wxInt32 volume)
{
    CslTTS& self=GetInstance();

    return self.m_ok ? self.m_volume : -1;
}

int CslTTS::SetVolume(wxInt32 volume)
{
    CslTTS& self=GetInstance();

    if (!self.m_ok)
        return -1;

    int old = self.m_volume;

    self.m_volume=volume;

#if defined(__WXMSW__)
    if (g_voice)
        g_voice->SetVolume(volume);
#elif defined(__WXMAC__)
    Fixed vol=FixRatio(self.m_volume,100);
    SetSpeechInfo(m_channel,soVolume,&vol);
#elif defined(HAVE_LIBSPEECHD_H)
    if (g_spd)
        spd_set_volume(g_spd,self.m_volume*2-100);
#endif //__WXMSW__

    return old;
}

#ifdef __WXMAC__
void CslTTS::Process(const wxString& text)
{
    static wxArrayString messages;

    if (text.IsEmpty())
    {
        if (messages.IsEmpty())
            return;
    }
    else
    {
        messages.Add(text);

        if (SpeechBusy())
            return;
    }

    const wxString& msg=messages.Item(0);
    SpeakText(m_channel,U2A(msg),msg.Length());
    messages.RemoveAt(0);
}

void CslTTS::OnProcessed(SpeechChannel channel,void *data)
{
    wxIdleEvent evt;
    wxPostEvent(&GetInstance(),evt);
}

void CslTTS::OnIdle(wxIdleEvent& WXUNUSED(event))
{
    Process();
}
#endif
