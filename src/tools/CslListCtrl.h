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

#ifndef CSLLISTCTRL_H
#define CSLLISTCTRL_H

#include <CslToolTip.h>

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EXPORTED_EVENT_TYPE(CSL_DLL_GUITOOLS, wxCSL_EVT_COMMAND_LIST_COLUMN_TOGGLED, wxID_ANY)
DECLARE_EXPORTED_EVENT_TYPE(CSL_DLL_GUITOOLS, wxCSL_EVT_COMMAND_LIST_ALL_ITEMS_SELECTED, wxID_ANY)
END_DECLARE_EVENT_TYPES()

#define CSL_EVT_LIST_COLUMN_TOGGLED(id, fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
                               wxCSL_EVT_COMMAND_LIST_COLUMN_TOGGLED, id, wxID_ANY, \
                               (wxObjectEventFunction)(wxEventFunction) \
                               wxStaticCastEvent(wxListEventFunction, &fn), \
                               (wxObject*)NULL \
                             ),

#define CSL_EVT_LIST_ALL_ITEMS_SELECTED(id, fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
                               wxCSL_EVT_COMMAND_LIST_ALL_ITEMS_SELECTED, id, wxID_ANY, \
                               (wxObjectEventFunction)(wxEventFunction) \
                               wxStaticCastEvent(wxListEventFunction, &fn), \
                               (wxObject*)NULL \
                             ),

enum
{
    CSL_LIST_IMG_GREEN = 0,
    CSL_LIST_IMG_YELLOW,
    CSL_LIST_IMG_RED,
    CSL_LIST_IMG_GREY,
    CSL_LIST_IMG_GREEN_EXT,
    CSL_LIST_IMG_YELLOW_EXT,
    CSL_LIST_IMG_RED_EXT,
    CSL_LIST_IMG_INFO,
    CSL_LIST_IMG_SORT_ASC,
    CSL_LIST_IMG_SORT_DSC,

    CSL_LIST_IMG_GAMES_START
};

class CSL_DLL_GUITOOLS CslListSort : public wxObject
{
    public:
        enum { SORT_ASC = 0, SORT_DSC };

        CslListSort(wxInt32 mode=SORT_ASC, wxInt32 column=0)
        {
            Init(mode, column);
        }

        void Init(wxInt32 mode, wxInt32 column)
        {
            Mode=mode;
            Column=column;
        }

        template<class T>
        wxInt32 Cmp(const T& first, const T& second, wxInt32 mode=-1) const
        {
            wxInt32 m=mode<0 ? Mode : mode;

            return first<second ? m==SORT_ASC ? -1 : 1 :
                   first==second ? 0 : m==SORT_ASC ? 1 : -1;
        }

        wxInt32 Mode;
        wxInt32 Column;

    private:
        DECLARE_DYNAMIC_CLASS(CslListSort)
};


class CSL_DLL_GUITOOLS CslListColumn : public wxObject
{
    public:
        CslListColumn(const wxString& name = wxEmptyString,
                      wxListColumnFormat format = wxLIST_FORMAT_LEFT,
                      float weight = 0.0f,
                      bool locked = false)
            { Create(name, format, weight, locked); }

        CslListColumn& Create(const wxString& name,
                              wxListColumnFormat format,
                              float weight, bool locked)
        {
            Name = name;
            Format = format;
            Width = 0;
            Weight = weight;
            Locked = locked;
            return *this;
        }

        wxString Name;
        wxListColumnFormat Format;
        wxInt32 Width;
        float Weight;
        bool Locked;

    private:
        DECLARE_DYNAMIC_CLASS(CslListColumn)
};

WX_DEFINE_USER_EXPORTED_ARRAY(CslListColumn*, CslArrayListColumn, class CSL_DLL_GUITOOLS);


class CSL_DLL_GUITOOLS CslListColumns : public wxObject
{
    friend class CslListCtrl;

    public:
        CslListColumns() :
            m_lockedCount(0), m_columnMask(0)
        {
            m_columns.Alloc(32);
        }

        ~CslListColumns()
        {
            Clear();
        }

    private:
        void AddColumn(const wxString& name, wxListColumnFormat format = wxLIST_FORMAT_LEFT,
                       float weight = 1.0f, bool enabled = true, bool locked = false);
        void RemoveColumn(wxInt32 id);
        void ToggleColumn(wxInt32 id);

        void Clear()
        {
            m_columnMask = m_lockedCount = 0;
            WX_CLEAR_ARRAY(m_columns);
        }

        CslListColumn* GetColumn(wxInt32 id)
        {
            wxASSERT_MSG(id>=0 && id<GetCount(), wxT("invalid column"));
            return (id<0 || id>=GetCount()) ? NULL : m_columns[id];
        }

        CslListColumn* GetColumn(const wxString& name)
        {
            loopv(m_columns) if (m_columns[i]->Name==name)
                return m_columns[i];

            return NULL;
        }

        bool LockColumn(wxInt32 id, bool lock);

        CslArrayListColumn& GetColumns() { return m_columns; }

    public:
        wxInt32 GetColumnId(wxInt32 id, bool absolute = true) const;
        wxUint32 GetColumnMask() const { return m_columnMask; }

        wxInt32 GetCount(bool enabled = false) const
        {
            return enabled ? BitCount32(m_columnMask) : m_columns.size();
        }

        bool IsEnabled(wxInt32 id) const
        {
            return CSL_FLAG_CHECK(m_columnMask, 1<<id);
        }

        bool IsLocked(wxInt32 id) const
        {
            const CslListColumn *c = ((CslListColumns*)(this))->GetColumn(id);
            return c && c->Locked;
        }

        wxInt32 GetLockedCount() const { return m_lockedCount; }

    private:
        wxInt32 m_lockedCount;
        wxUint32 m_columnMask;
        CslArrayListColumn m_columns;

        DECLARE_DYNAMIC_CLASS(CslListColumns)
};


class CSL_DLL_GUITOOLS CslListCtrl : public wxListCtrl
{
    public:
        CslListCtrl(wxWindow* parent = NULL,
                    wxWindowID id = wxID_ANY,
                    const wxPoint& pos = wxDefaultPosition,
                    const wxSize& size = wxDefaultSize,
                    long style = wxLC_REPORT,
                    const wxValidator& validator = wxDefaultValidator,
                    const wxString& name = wxListCtrlNameStr);

        virtual ~CslListCtrl();

#ifdef __WXMSW__
        // functions from generic implementation
        bool InReportView() const
            { return CSL_FLAG_CHECK(GetWindowStyle(), wxLC_REPORT); }
        bool HasHeader() const
            { return InReportView() && !CSL_FLAG_CHECK(GetWindowStyle(), wxLC_NO_HEADER); }
#endif
        wxInt32 GetHeaderHeight()
        {
#ifdef __WXMSW__
            return 0;
#else
            return HasHeader() ? ((wxWindow*)m_headerWin)->GetSize().GetHeight() : 0;
#endif
        }

        void CreateScreenShot();

        static wxInt32 GetGameImage(CslEngine* engine, wxUint32 fourcc, wxInt32 offset=0);
        static wxInt32 GetCountryFlag(CslEngine* engine, wxUint32 ip);

        void ListAddColumn(const wxString& name,
                           wxListColumnFormat format = wxLIST_FORMAT_LEFT,
                           float weight = 1.0f,
                           bool enabled = true, bool locked = false);
        void ListDeleteColumn(wxInt32 column);
        wxUint32 ListToggleColumn(wxInt32 column);

        bool ListSetColumn(wxInt32 column, const wxString& name = wxEmptyString);
        bool ListSetItem(wxInt32 row, wxInt32 column,
                         const wxString& text = wxEmptyString,
                         wxInt32 image = -1);

        void ListLockColumn(wxInt32 column, bool lock = true);
        bool ListColumnIsLocked(wxInt32 column) const
            { return m_columns.IsLocked(column); }

        wxInt32 GetColumnId(wxInt32 id, bool absolute = true) const
            { return m_columns.GetColumnId(id, absolute); }
        wxUint32 ListGetColumnMask() const
            { return m_columns.GetColumnMask(); }
        bool ListColumnIsEnabled(wxInt32 id) const
            { return m_columns.IsEnabled(id); }

        void SetSortCallback(wxListCtrlCompare fn, wxInt32 mode, wxInt32 column);
        void ListSort(wxInt32 column = -1);

        virtual void ListAdjustSize(const wxSize& size = wxDefaultSize);

        static void CreateImageList(CslEngine* engine);

    protected:
        wxInt32 ListFindItem(void *data);

        // virtual
        virtual bool ListFindItemCompare(void *data1, void *data2) { return data1==data2; }
        virtual wxWindow* GetScreenShotWindow() { return this; }
        virtual wxString GetScreenShotFileName();
        virtual void GetToolTipText(wxInt32 row, CslToolTipEvent& WXUNUSED(event)) { }

        virtual wxSize GetImageListSize()
        {
            wxInt32 x, y;
            if (ListImageList.GetSize(0, x, y))
                return wxSize(x, y);
            return wxDefaultSize;
        }

        wxUint32 m_mouseLastMove;

        CslListSort m_sortHelper;

        bool m_dontAdjustSize;
        bool m_userColumnSize;
        CslListColumns m_columns;

        wxInt32 m_processSelectEvent;
        wxArrayPtrVoid m_selected;

        static wxImageList ListImageList;
    private:
#ifdef __WXMSW__
        void OnEraseBackground(wxEraseEvent& event);
#endif
        void OnColumnDragStart(wxListEvent& event);
        void OnColumnDragEnd(wxListEvent& event);
        void OnSize(wxSizeEvent& event);
        void OnMouseMove(wxMouseEvent& event);
        void OnMenu(wxCommandEvent& event);
        void OnContextMenu(wxContextMenuEvent& event);
        void OnColumnLeftClick(wxListEvent& event);
        void OnColumnRightClick(wxListEvent& event);
        void OnKeyDown(wxKeyEvent &event);
        void OnItem(wxListEvent& event);
        void OnToolTip(CslToolTipEvent& event);

        wxListCtrlCompare m_sortCallback;

        DECLARE_EVENT_TABLE()
        DECLARE_DYNAMIC_CLASS_NO_COPY(CslListCtrl)
};

#endif //CSLLISTCTRL_H
