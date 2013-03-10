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

#if defined(__WXMSW__)
#include <sapi.h>
    #ifdef _MSC_VER
        #pragma comment(lib, "sapi.lib")
    #endif
#elif defined(__WXMAC__)
    #include <Carbon/Carbon.h>
#elif defined(HAVE_CONFIG_H)
    #ifdef HAVE_LIBSPEECHD_H
        #include <libspeechd.h>
    #endif //HAVE_LIBSPEECHD_H
#endif //__WXMSW__

#include "CslTTS.h"

#if defined(__WXMSW__)
ISpVoice *g_csl_voice;
#elif defined(__WXMAC__)
SpeechChannel g_csl_channel;
#elif defined(HAVE_LIBSPEECHD_H)
SPDConnection *g_csl_spd;
#endif //__WXMSW__


CslTTS::CslTTS()
{
    m_ok = false;
    m_enabled = false;
    m_volume = 0;
#if defined(__WXMSW__)
    g_csl_voice = NULL;
#elif defined(__WXMAC__)
#elif defined(HAVE_LIBSPEECHD_H)
    g_csl_spd = NULL;
#endif //__WXMSW__
}

CslTTS& CslTTS::GetInstance()
{
    static CslTTS self;

    return self;
}

bool CslTTS::Init(const wxString& lang, bool enable, wxInt32 volume)
{
    CslTTS& self = GetInstance();

    if (self.m_ok)
        return false;

#if defined(__WXMSW__)
    if (CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL,
                         IID_ISpVoice, (void**)&g_csl_voice)!=S_OK)
        g_csl_voice = NULL;
    else
        self.m_ok = true;
#elif defined(__WXMAC__)
    if (NewSpeechChannel(NULL, &g_csl_channel)==noErr)
    {
        if (SetSpeechInfo(g_csl_channel, soSpeechDoneCallBack, (void*)OnProcessed)==noErr)
        {
            self.m_ok = true;
            self.Connect(wxEVT_IDLE, wxIdleEventHandler(CslTTS::OnIdle), NULL, &self);
        }
        else
            DisposeSpeechChannel(g_csl_channel);
    }
#elif defined(HAVE_LIBSPEECHD_H)
    if ((g_csl_spd=spd_open(__CSL_NAME_SHORT_STR,NULL,NULL,SPD_MODE_THREADED)))
    {
        if (spd_set_punctuation(g_csl_spd, SPD_PUNCT_NONE))
            CSL_LOG_DEBUG("Failed to set punctuation mode.\n");
        if (spd_set_spelling(g_csl_spd, SPD_SPELL_ON))
            CSL_LOG_DEBUG("Failed to set spelling mode.\n");
#if 0 //disable for now
        if (!lang.IsEmpty() && spd_set_language(g_csl_spd, U2C(lang)))
            CSL_LOG_DEBUG("Failed to set language \"%s\"\n", U2C(lang));
#endif
    }
    else
    {
        CSL_LOG_DEBUG("Failed to connect to speech-dispatcher.\n");
        return false;
    }

    self.m_ok = true;
#endif //__WXMSW__

    Set(enable, volume);

    return self.m_ok;
}

bool CslTTS::DeInit()
{
    CslTTS& self = GetInstance();

    if (!self.m_ok)
        return false;

#if defined(__WXMSW__)
    if (g_csl_voice)
    {
        g_csl_voice->Release();
        g_csl_voice = NULL;
    }
#elif defined(__WXMAC__)
    if (g_csl_channel)
    {
        DisposeSpeechChannel(g_csl_channel);
        g_csl_channel = NULL;
    }
#elif defined(HAVE_LIBSPEECHD_H)
    if (g_csl_spd)
    {
        spd_close(g_csl_spd);
        g_csl_spd = NULL;
    }
#endif //__WXMSW__

    self.m_ok = false;

    return true;
}

bool CslTTS::IsOk()
{
    return GetInstance().m_ok;
}

bool CslTTS::Say(const wxString& text)
{
    CslTTS& self = GetInstance();

    if (!self.m_ok || !self.m_enabled || !self.m_volume || text.IsEmpty())
        return false;

#if defined(__WXMSW__)
    return g_csl_voice->Speak(text.wc_str(wxConvUTF8), SPF_ASYNC,NULL)==S_OK;
#elif defined(__WXMAC__)
    return self.Process(text);
#elif defined(HAVE_LIBSPEECHD_H)
    return spd_say(g_csl_spd,SPD_MESSAGE,U2C(text))>-1;
#endif //__WXMSW__
}

bool CslTTS::Say(const wxString& text, wxInt32 volume)
{
    CslTTS& self = GetInstance();

    if (!self.m_ok)
        return false;

    bool ret = true;
    wxInt32 volumeold;

    volume = clamp(volume, 0, 100);
    volumeold = self.m_volume;

    SetVolume(volume);
    ret = Say(text);
    //SetVolume(volumeold);

    return ret;
}

bool CslTTS::Set(bool enable, wxInt32 volume)
{
    CslTTS& self = GetInstance();

    if (!self.m_ok)
        return false;

    self.m_enabled = enable;
    SetVolume(volume);

    return true;
}

bool CslTTS::Enable(bool v)
{
    CslTTS& self = GetInstance();

    return self.m_ok ? (self.m_enabled = v) : false;
}

bool CslTTS::Enabled()
{
    CslTTS& self = GetInstance();

    return self.m_ok ? self.m_enabled : false;
}

int CslTTS::GetVolume()
{
    CslTTS& self = GetInstance();

    return self.m_ok ? self.m_volume : -1;
}

int CslTTS::SetVolume(wxInt32 volume)
{
    CslTTS& self = GetInstance();

    if (!self.m_ok)
        return -1;

    wxInt32 old = self.m_volume;

    self.m_volume = clamp(volume, 0, 100);

#if defined(__WXMSW__)
    if (g_csl_voice)
        g_csl_voice->SetVolume(volume);
#elif defined(__WXMAC__)
    Fixed vol = FixRatio(self.m_volume, 100);
    SetSpeechInfo(g_csl_channel,soVolume, &vol);
#elif defined(HAVE_LIBSPEECHD_H)
    if (g_csl_spd)
        spd_set_volume(g_csl_spd, self.m_volume*2-100);
#endif //__WXMSW__

    return old;
}

#ifdef __WXMAC__
bool CslTTS::Process(const wxString& text)
{
    static wxArrayString messages;

    if (text.IsEmpty())
    {
        if (messages.IsEmpty())
            return true;
    }
    else
    {
        messages.Add(text);

        if (SpeechBusy())
            return true;
    }

    const wxString& msg = messages.Item(0);
    ret = SpeakText(g_csl_channel, U2C(msg), msg.Length())==noErr;
    messages.RemoveAt(0);

    return ret;
}

void CslTTS::OnProcessed(SpeechChannel channel, void *data)
{
    wxIdleEvent evt;
    wxPostEvent(&GetInstance(), evt);
}

void CslTTS::OnIdle(wxIdleEvent& WXUNUSED(event))
{
    Process();
}
#endif
