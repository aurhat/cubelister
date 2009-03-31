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

#ifndef CSLSTATUSBAR_H
#define CSLSTATUSBAR_H

/**
 @author Glen Masgai <mimosius@users.sourceforge.net>
*/


enum { LIGHT_GREEN = 0, LIGHT_GREY, LIGHT_RED, LIGHT_YELLOW };

class CslStatusBar : public wxStatusBar
{
    public:
        CslStatusBar(wxWindow *parent);

        static void InitBar(CslStatusBar *bar) { m_self=bar; }
        static void Light(wxInt32 light) { m_self->SetLight(light); };
        static wxInt32 Light() { return m_self->GetLight(); };
        static void SetText(wxUint32 id,const wxString& text)
        {
            m_self->SetStatusText(text,id);
        };

    private:
        static CslStatusBar* m_self;

        void OnSize(wxSizeEvent& event);
        DECLARE_EVENT_TABLE()

    protected:
        wxStaticBitmap *m_bmp;
        wxInt32 m_light;

        void SetLight(wxInt32 light);
        wxInt32 GetLight() { return m_light; };
};

#endif // CSLSTATUSBAR_H
