/***************************************************************************
 *   Copyright (C) 2007-2014 by Glen Masgai                                *
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

#ifndef CSLSTATUSBAR_H
#define CSLSTATUSBAR_H

enum { LIGHT_GREY,
       LIGHT_GREEN,
       LIGHT_YELLOW,
       LIGHT_RED };

class CslStatusBar : public wxStatusBar
{
    public:
        static CslStatusBar* Init(wxWindow *parent)
        {
            return (m_self = new CslStatusBar(parent));
        }

        static void Light(wxInt32 light)
        {
            m_self->SetLight(light);
        }

        static wxInt32 Light()
        {
            return m_self->GetLight();
        }

        static void SetText(wxUint32 id,const wxString& text)
        {
#ifdef __WXMSW__
            wxWindowUpdateLocker lock(m_self);
#endif
            m_self->SetStatusText(text, id);
        }

    private:
        CslStatusBar(wxWindow *parent);

        void OnSize(wxSizeEvent& event);

        DECLARE_EVENT_TABLE()

        static CslStatusBar* m_self;

        wxInt32 m_light;
        CslBufferedStaticBitmap *m_bmp;

        void SetLight(wxInt32 light);

        wxInt32 GetLight()
        {
            return m_light;
        }
};

#endif // CSLSTATUSBAR_H
