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

#ifndef CSLTTS_H
#define CSLTTS_H

/**
    @author Glen Masgai <mimosius@gmx.de>
*/

#define CSL_TTS_DEFAULT_VOLUME  50

#ifdef __WXMAC__
class CslTTS : public wxEvtHandler
#else
class CSL_DLL_GUITOOLS CslTTS
#endif
{
    private:
        CslTTS();
        CslTTS(const CslTTS& tts) {}

        static CslTTS& GetInstance();

#if defined (__WXMAC__)
        bool Process(const wxString& text = wxEmptyString);
        static void OnProcessed(SpeechChannel channel, void *data);
        void OnIdle(wxIdleEvent& event);
#endif

    public:
        static bool Init(const wxString& lang = wxEmptyString,
                         bool enable = true,
                         wxInt32 volume = CSL_TTS_DEFAULT_VOLUME);
        static bool DeInit();
        static bool IsOk();

        static bool Set(bool enable, wxInt32 volume);
        static bool Enable(bool v = true);
        static bool Enabled();
        static int GetVolume();
        static int SetVolume(wxInt32 volume);

        static bool Say(const wxString& text);
        static bool Say(const wxString& text, wxInt32 volume);

    private:
        bool m_ok;
        bool m_enabled;
        wxInt32 m_volume;
        wxString m_lang;
};

#endif
