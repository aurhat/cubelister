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

#ifndef CSLDLGCONNECTPASS_H
#define CSLDLGCONNECTPASS_H

class CslConnectPassInfo
{
    public:
        CslConnectPassInfo(const wxString& password=wxEmptyString,
                           const wxString& adminpassword=wxEmptyString,
                           const bool admin=false) :
                Password(password),AdminPassword(adminpassword),Admin(admin) {}

        wxString Password,AdminPassword;
        bool Admin;
};


class CslDlgConnectPass: public wxDialog
{
    public:
    // begin wxGlade: CslDlgConnectPass::ids
    // end wxGlade
        CslDlgConnectPass(wxWindow* parent, CslConnectPassInfo *info);

    private:
    // begin wxGlade: CslDlgConnectPass::methods
    void set_properties();
    void do_layout();
    // end wxGlade

        void OnCommandEvent(wxCommandEvent& event);

        DECLARE_EVENT_TABLE()

    protected:
    // begin wxGlade: CslDlgConnectPass::attributes
    wxStaticBox* sizer_ctrl_staticbox;
    wxTextCtrl* m_tcPassword;
    wxCheckBox* m_cbAdmin;
    wxSizer* m_bsDlg;
    // end wxGlade

        CslConnectPassInfo *m_info;
}; // wxGlade: end class


#endif // CSLDLGCONNECTPASS_H
