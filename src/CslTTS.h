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

#ifndef CSLTTS_H
#define CSLTTS_H

/**
    @author Glen Masgai <mimosius@gmx.de>
*/


#ifdef __WXMAC__
class CslTTS : public wxEvtHandler
#else
class CslTTS
#endif
{
    private:
        CslTTS();
        CslTTS(const CslTTS& tts) {}

        static CslTTS& GetInstance();

#if defined (__WXMAC__)
        void Process(const wxString& text=wxEmptyString);
        static void OnProcessed(SpeechChannel channel,void *data);

        void OnIdle(wxIdleEvent &event);
#endif

    public:
        static bool Init(const wxString& lang=wxEmptyString);
        static bool DeInit();
        static bool IsOk();

        static int GetVolume(wxInt32 volume);
        static int SetVolume(wxInt32 volume);
        static void Say(const wxString& text);

    private:
        bool m_ok;
        wxInt32 m_volume;
        wxString m_lang;
};

#endif
