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

#ifdef HAVE_CONFIG_H
#include "config.h"
#ifdef HAVE_LIBSPEECHD_H
#include <libspeechd.h>
#endif //HAVE_LIBSPEECHD_H
#endif //HAVE_CONFIG_H

#include "CslTTS.h"
#include "CslSettings.h"
#include "engine/CslTools.h"
#include "engine/CslVersion.h"


CslTTS::CslTTS()
{
    m_ok=false;
    m_volume=0;
#ifdef HAVE_LIBSPEECHD_H
    m_spd=NULL;
#endif //HAVE_LIBSPEECHD_H

}

CslTTS::~CslTTS()
{
    DeInit();
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

#ifdef HAVE_LIBSPEECHD_H
    if ((self.m_spd=spd_open(__CSL_NAME_SHORT_STR,NULL,NULL,SPD_MODE_THREADED)))
    {
        if (spd_set_punctuation(self.m_spd,SPD_PUNCT_NONE))
        {
            LOG_DEBUG("Failed to set punctuation mode.\n");
        }
        if (spd_set_spelling(self.m_spd,SPD_SPELL_ON))
        {
            LOG_DEBUG("Failed to set spelling mode.\n");
        }
        if (!lang.IsEmpty() && spd_set_language(self.m_spd,U2A(lang)))
        {
            LOG_DEBUG("Failed to set language \"%s\"\n",U2A(lang));
        }
    }
    else
    {
        LOG_DEBUG("Failed to connect to speech-dispatcher.\n");
        return false;
    }

    self.m_ok=true;
#endif //HAVE_LIBSPEECHD_H

    return self.m_ok;
}

bool CslTTS::DeInit()
{
    CslTTS& self=GetInstance();

    if (!self.m_ok)
        return false;

#ifdef HAVE_LIBSPEECHD_H
    if (self.m_spd)
    {
        spd_close(self.m_spd);
        self.m_spd=NULL;
    }
#endif //HAVE_LIBSPEECHD_H

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

    if (!self.m_ok || !g_cslSettings->tts)
        return;

    if (self.m_volume!=g_cslSettings->ttsVolume)
        SetVolume(g_cslSettings->ttsVolume);

#ifdef HAVE_LIBSPEECHD_H
    if (self.m_spd)
    {
        if (spd_say(self.m_spd,SPD_MESSAGE,U2A(text))<=0)
        {
            if (DeInit())
                Init(self.m_lang);
        }
    }
#endif //HAVE_LIBSPEECHD_H
}

void CslTTS::SetVolume(wxInt32 volume)
{
    CslTTS& self=GetInstance();

    if (!self.m_ok)
        return;

    self.m_volume=volume;

#ifdef HAVE_LIBSPEECHD_H
    if (self.m_spd)
        spd_set_volume(self.m_spd,self.m_volume*2-100);
#endif //HAVE_LIBSPEECHD_H
}
