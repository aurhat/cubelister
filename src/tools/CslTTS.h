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

#define CSL_TTS_DEFAULT_VOLUME  50

class CSL_DLL_GUITOOLS CslTTSSettings : public wxObject
{
    public:
        CslTTSSettings(bool enabled = true,
                       wxInt32 volume = CSL_TTS_DEFAULT_VOLUME)
            { Create(enabled, volume); }

        bool operator==(const CslTTSSettings& settings) const
        {
            return Enabled == settings.Enabled &&
                   Volume  == settings.Volume;
        }
        bool operator!=(const CslTTSSettings& settings) const
            { return !(*this == settings); }

        bool IsEmpty() const { return !Enabled || Volume<0; }

        void Create(bool enabled = true,
                    wxInt32 volume = CSL_TTS_DEFAULT_VOLUME)
        {
            Enabled = enabled;
            Volume = volume;
        }

        bool Enabled;
        wxInt32 Volume;

     private:
         DECLARE_DYNAMIC_CLASS(CslTTSMessage)
};

CSL_DLL_GUITOOLS extern const CslTTSSettings CslTTSEmptySettings;


class CSL_DLL_GUITOOLS CslTTSMessage : public wxObject
{
    public:
        CslTTSMessage(const wxString& text = wxEmptyString,
                      const CslTTSSettings& settings = CslTTSEmptySettings)
             { Create(text, settings); }

        void Create(const wxString& text = wxEmptyString,
                    const CslTTSSettings& settings = CslTTSEmptySettings)
        {
            Text = text;
            Settings = settings;
        }

        wxString Text;
        CslTTSSettings Settings;

     private:
         DECLARE_DYNAMIC_CLASS(CslTTSMessage)
};

WX_DEFINE_USER_EXPORTED_ARRAY(CslTTSMessage*, CslArrayCslTTSMessage, class CSL_DLL_GUITOOLS);


#if defined(__WXMAC__) || defined(__WXMSW__)
class CSL_DLL_GUITOOLS CslTTS : public wxEvtHandler
#else
class CslTTS
#endif
{
    public:
        static bool Init(const CslTTSSettings& settings);
        static bool DeInit();
        static bool IsOk() { return GetInstance().m_ok; }

        static bool IsEnabled() { return GetInstance().m_settings.Enabled; }
        static wxInt32 GetVolume() { return GetInstance().m_settings.Volume; }
        static CslTTSSettings GetSettings() { return GetInstance().m_settings; }

        static void SetSettings(const CslTTSSettings& settings)
            { GetInstance().m_settings = settings; }

        static bool Say(const wxString& text,
                        const CslTTSSettings& settings = CslTTSEmptySettings);

    private:
        CslTTS();
        CslTTS(const CslTTS& tts) { }

        static CslTTS& GetInstance();

        static bool SetVolume(wxInt32 volume);
        bool ApplySettings(const CslTTSSettings& settings);

#if defined(__WXMSW__) || defined(__WXMAC__)
    #ifdef __WXMSW__
        static void wxCALLBACK OnProcessed(WPARAM wParam, LPARAM lParam);
    #endif
    #ifdef __WXMAC__
        static void OnProcessed(SpeechChannel channel, void *data);
    #endif
        bool Process(CslTTSMessage *message = NULL);

        void OnIdle(wxIdleEvent& event);

        CslArrayCslTTSMessage m_messages;
        wxCriticalSection m_messagesCritSection;
#endif
        bool m_ok;
        CslTTSSettings m_settings;
};

#endif
