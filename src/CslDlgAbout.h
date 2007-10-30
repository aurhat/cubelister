
#ifndef CSLDLGABOUT_H
#define CSLDLGABOUT_H

#include <wx/wx.h>
#include <wx/image.h>

// begin wxGlade: ::dependencies
#include <wx/notebook.h>
// end wxGlade

// begin wxGlade: ::extracode

// end wxGlade

class CslPanelAboutImage : public wxPanel
{
    public:
        CslPanelAboutImage(wxWindow *parent,wxInt32 id);

    private:
        void OnPaint(wxPaintEvent& event);
        DECLARE_EVENT_TABLE();

    protected:
        wxBitmap m_bitmap;
};


class CslDlgAbout: public wxDialog
{
    public:
        // begin wxGlade: CslDlgAbout::ids
        // end wxGlade
        CslDlgAbout(wxWindow* parent,int id=wxID_ANY,const wxString& title=wxEmptyString,
                    const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize,
                    long style=wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER);

    private:
        // begin wxGlade: CslDlgAbout::methods
        void set_properties();
        void do_layout();
        // end wxGlade

        void OnCommandEvent(wxCommandEvent& event);

        DECLARE_EVENT_TABLE();

    protected:
        // begin wxGlade: CslDlgAbout::attributes
        CslPanelAboutImage* panel_bitmap;
        wxStaticText* label_name;
        wxStaticText* label_version;
        wxStaticText* label_desc;
        wxStaticText* label_copyright;
        wxTextCtrl* text_ctrl_credits;
        wxPanel* notebook_pane_credits;
        wxTextCtrl* text_ctrl_license;
        wxPanel* notebook_pane_license;
        wxNotebook* notebook;
        wxButton* button_close;
        // end wxGlade
}; // wxGlade: end class


#endif // CSLDLGABOUT_H
