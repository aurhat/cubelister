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
#include "CslTTS.h"

#if defined(__WXMSW__)
    #include <sapi.h>
    #include <comdef.h> // _com_error
    #ifdef _MSC_VER
        #pragma comment(lib, "sapi.lib")
    #endif
#elif defined(__WXMAC__)
    #include <Carbon/Carbon.h>
#elif defined(HAVE_CONFIG_H)
    #include <config.h>
    #if HAVE_LIBSPEECHD_H
        #include <libspeechd.h>
    #elif HAVE_LIBSPEECHD_08_H
        #define HAVE_LIBSPEECHD_H 1
        #include <speech-dispatcher/libspeechd.h>
    #endif
#endif //__WXMSW__

#if defined(__WXMSW__)
ISpVoice *g_csl_tts;
#elif defined(__WXMAC__)
SpeechChannel g_csl_tts;
#elif defined(HAVE_LIBSPEECHD_H)
SPDConnection *g_csl_tts;
#endif //__WXMSW__


IMPLEMENT_DYNAMIC_CLASS(CslTTSSettings, wxObject);
IMPLEMENT_DYNAMIC_CLASS(CslTTSMessage, wxObject);

const CslTTSSettings CslTTSEmptySettings = CslTTSSettings(false, -1);


CslTTS::CslTTS()
{
    m_ok = false;
    g_csl_tts = NULL;
}

CslTTS& CslTTS::GetInstance()
{
    static CslTTS self;

    return self;
}

bool CslTTS::Init(const CslTTSSettings& settings)
{
    CslTTS& self = GetInstance();

    if (self.m_ok)
        return false;

#if defined(__WXMSW__)
    HRESULT hr;

    if ((hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL,
                               IID_ISpVoice, (void**)&g_csl_tts))!=S_OK ||
        (hr = g_csl_tts->SetNotifyCallbackFunction(CslTTS::OnProcessed, 0, 0))!=S_OK ||
        (hr = g_csl_tts->SetInterest(SPFEI(SPEI_END_INPUT_STREAM), SPFEI(SPEI_END_INPUT_STREAM)))!=S_OK)
    {
        g_csl_tts = NULL;
        CSL_LOG_DEBUG("Failed to initialise TTS (%s)\n", _com_error(hr).ErrorMessage());
    }
    else
    {
        self.m_ok = true;
        self.Connect(wxEVT_IDLE, wxIdleEventHandler(CslTTS::OnIdle), NULL, &self);
    }
#elif defined(__WXMAC__)
    if (NewSpeechChannel(NULL, &g_csl_tts)==noErr)
    {
        if (SetSpeechInfo(g_csl_tts, soSpeechDoneCallBack, (void*)c)==noErr)
        {
            self.m_ok = true;
            self.Connect(wxEVT_IDLE, wxIdleEventHandler(CslTTS::OnIdle), NULL, &self);
        }
        else
            DisposeSpeechChannel(g_csl_tts);
    }
#elif defined(HAVE_LIBSPEECHD_H)
    if ((g_csl_tts = spd_open(__CSL_NAME_SHORT_STR, __CSL_NAME_SHORT_STR,
                              U2C(::wxGetUserName()), SPD_MODE_THREADED)))
    {
        if (spd_set_punctuation(g_csl_tts, SPD_PUNCT_NONE))
            CSL_LOG_DEBUG("Failed to set punctuation mode.\n");
        if (spd_set_spelling(g_csl_tts, SPD_SPELL_ON))
            CSL_LOG_DEBUG("Failed to set spelling mode.\n");

        self.m_ok = true;
    }
    else
        CSL_LOG_DEBUG("Failed to connect to speech-dispatcher.\n");
#endif //__WXMSW__

    SetSettings(settings);

    return self.m_ok;
}

bool CslTTS::DeInit()
{
    CslTTS& self = GetInstance();

    if (!self.m_ok || !g_csl_tts)
        return false;

#if defined(__WXMSW__)
    g_csl_tts->Release();
#elif defined(__WXMAC__)
    DisposeSpeechChannel(g_csl_tts);
#elif defined(HAVE_LIBSPEECHD_H)
    spd_cancel(g_csl_tts);
    spd_close(g_csl_tts);
#endif //__WXMSW__

    self.m_ok = false;
    g_csl_tts = NULL;

#if defined(__WXMSW__) || defined(__WXMAC__)
    wxCriticalSectionLocker lock(self.m_messagesCritSection);

    WX_CLEAR_ARRAY(self.m_messages);
#endif

    return true;
}

bool CslTTS::Say(const wxString& text, const CslTTSSettings& settings)
{
    CslTTS& self = GetInstance();

    if (!self.m_ok || text.IsEmpty())
        return false;

    if (settings.IsEmpty() && self.m_settings.IsEmpty())
        return false;

    bool ret = false;

#if defined(__WXMSW__) || defined(__WXMAC__)
    ret = self.Process(new CslTTSMessage(text, settings));
#elif defined(HAVE_LIBSPEECHD_H)
    if (!settings.IsEmpty())
        self.ApplySettings(settings);

       ret = spd_say(g_csl_tts, SPD_MESSAGE, U2C(text))>-1;
#endif

    return ret;
}

bool CslTTS::SetVolume(wxInt32 volume)
{
    if (!IsOk())
        return false;

    volume = clamp(volume, 0, 100);

#if defined(__WXMSW__)
    if (g_csl_tts)
        g_csl_tts->SetVolume(volume);
#elif defined(__WXMAC__)
    Fixed vol = FixRatio(volume, 100);
    SetSpeechInfo(g_csl_tts, soVolume, &vol);
#elif defined(HAVE_LIBSPEECHD_H)
    if (g_csl_tts)
        spd_set_volume(g_csl_tts, volume*2-100);
#else
    return false;
#endif //__WXMSW__

    return true;
}

bool CslTTS::ApplySettings(const CslTTSSettings& settings)
{
    CslTTS& self = GetInstance();

    if (!self.m_ok)
        return false;

    if (SetVolume(settings.Volume))
        return true;

    return true;
}

#if defined(__WXMSW__) || defined(__WXMAC__)
bool CslTTS::Process(CslTTSMessage *message)
{
    wxCriticalSectionLocker lock(m_messagesCritSection);

    if (!message)
    {
        if (!m_messages.size())
            return ApplySettings(m_settings);
    }
    else
    {
        m_messages.push_back(message);

        if (m_messages.size()>1)
            return true;
    }

    message = m_messages[0];

    if (!message->Settings.IsEmpty())
        ApplySettings(message->Settings);

    bool ret = false;

#if defined(__WXMSW__)
    ret = g_csl_tts->Speak(message->Text.wc_str(wxConvUTF8), SPF_ASYNC, NULL)==S_OK;
#elif defined(__WXMAC__)
    ret = SpeakText(g_csl_tts, U2C(msg), msg.Length())==noErr;
#endif

    return ret;
}

#if defined(__WXMSW__)
void CslTTS::OnProcessed(WPARAM WXUNUSED(wParam), LPARAM WXUNUSED(lParam))
#elif defined(__WXMAC__)
void CslTTS::OnProcessed(SpeechChannel WXUNUSED(channel), void* WXUNUSED(data))
#endif
{
    CslTTS& self = GetInstance();

    self.m_messagesCritSection.Enter();

    if (self.m_messages.size())
    {
        delete self.m_messages[0];
        self.m_messages.erase(self.m_messages.begin());
    }

    self.m_messagesCritSection.Leave();

    wxIdleEvent evt;
    wxPostEvent(&GetInstance(), evt);
}

void CslTTS::OnIdle(wxIdleEvent& WXUNUSED(event))
{
    Process();
}
#endif
