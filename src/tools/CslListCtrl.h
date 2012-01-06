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

#ifndef CSLLISTCTRL_H
#define CSLLISTCTRL_H

/**
    @author Glen Masgai <mimosius@users.sourceforge.net>
*/

#include <CslToolTip.h>

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE(wxCSL_EVT_COMMAND_LIST_ALL_ITEMS_SELECTED, wxID_ANY)
END_DECLARE_EVENT_TYPES()

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
    CSL_LIST_IMG_SORT_ASC_LIGHT,
    CSL_LIST_IMG_SORT_DSC_LIGHT,

    CSL_LIST_IMG_GAMES_START
};

class CslListSort
{
    public:
        enum { SORT_ASC = 0, SORT_DSC };

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
};

class CslListColumns
{
    private:
        friend class CslListCtrl;

        struct Column
        {
            wxString Name;
            wxListColumnFormat Format;
            int Width;
            float Weight;
            bool Locked;
        };

        CslListColumns() : m_lockedCount(0), m_columnMask(0) { }

        void AddColumn(const wxString& name, wxListColumnFormat format=wxLIST_FORMAT_LEFT,
                       float weight=1.0f, bool enabled=true, bool locked=false)
        {
            CSL_FLAG_SET(m_columnMask, enabled ? 1<<GetCount() : 0);
            Column& c=m_columns.add();
            c.Name=name;
            c.Format=format;
            c.Weight=weight;
            c.Locked=locked;

            if (locked)
                m_lockedCount++;
        }

        void RemoveColumn(wxInt32 id)
        {
            wxASSERT_MSG(id>=0 && id<m_columns.length(), wxT("Invalid column id."));
            const Column& c = m_columns[id];
            if (c.Locked) m_lockedCount--;
            CSL_FLAG_UNSET(m_columnMask, 1<<id);
            m_columns.remove(id);
        }

        void ToggleColumn(wxInt32 id)
        {
            wxASSERT_MSG(id>=0 && id<GetCount(), wxT("Invalid column id."));
            if (id<0 || id>=GetCount())
                return;

            if (CSL_FLAG_CHECK(m_columnMask, 1<<id))
                CSL_FLAG_UNSET(m_columnMask, 1<<id);
            else
                CSL_FLAG_SET(m_columnMask, 1<<id);
        }

        bool IsEnabled(wxInt32 id) const { return CSL_FLAG_CHECK(m_columnMask, 1<<id); }

        void Clear()
        {
            m_columnMask=0;
            m_lockedCount=0;
            loopvrev(m_columns) m_columns.remove(i);
        }

        Column* GetColumn(wxInt32 id)
        {
            wxASSERT_MSG(id>=0 && id<GetCount(), wxT("invalid column"));
            if (id<0 || id>=GetCount())
                return NULL;
            return &m_columns[id];
        }

        Column* GetColumn(const wxString& name)
        {
            loopv(m_columns) if (m_columns[i].Name==name)
                return &m_columns[i];
            return NULL;
        }

        wxInt32 GetColumnId(wxInt32 id, bool absolute=true) const
        {
            wxASSERT_MSG(id>=0 && id<GetCount(), wxT("invalid column"));

            if (absolute)
            {
                for (wxInt32 i=0; i<=id && i<GetCount(); i++)
                {
                    if (!IsEnabled(i))
                        id++;
                }
                id=min(id, GetCount()-1);
            }
            else
            {
                for (wxInt32 i=0, c=id; i<c; i++)
                {
                    if (!IsEnabled(i))
                        id--;
                }
                id=max(0, id);
            }

            return id;
        }

        wxUint32 GetEnabledColumns() const
        {
            return m_columnMask;
        }

        wxInt32 GetCount(bool enabled=false) const
        {
            if (enabled)
                return BitCount32(m_columnMask);
            else
                return m_columns.length();
        }

        wxInt32 GetLockedCount() const { return m_lockedCount; }

        vector<Column>& GetColumns() { return m_columns; }

    private:
        wxInt32 m_lockedCount;
        wxUint32 m_columnMask;
        vector<Column> m_columns;
};

class CslListCtrl : public wxListCtrl
{
    DECLARE_DYNAMIC_CLASS(CslListCtrl)

    private:
        CslListCtrl() { };

    public:
        CslListCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos=wxDefaultPosition,
                    const wxSize& size=wxDefaultSize, long style=wxLC_ICON,
                    const wxValidator& validator=wxDefaultValidator,
                    const wxString& name=wxListCtrlNameStr);
        ~CslListCtrl();

        void CreateScreenShot();
        static wxInt32 GetGameImage(wxUint32 fourcc, wxInt32 offset=0);
        static wxInt32 GetCountryFlag(wxUint32 ip);

        void ListAddColumn(const wxString& name, wxListColumnFormat format=wxLIST_FORMAT_LEFT,
                              float weight=1.0f, bool enabled=true, bool locked=false);
        void ListDeleteColumn(wxInt32 column);
        wxUint32 ListToggleColumn(wxInt32 column);
        bool ListSetItem(wxInt32 row, wxInt32 column, const wxString& text=wxEmptyString, wxInt32 image=-1);
        bool ListSetColumn(wxInt32 column, const wxString& text=wxEmptyString);

        wxUint32 ListGetColumnMask() { return m_columns.GetEnabledColumns(); }
        bool ListIsColumnEnabled(wxInt32 id) { return m_columns.IsEnabled(id); }

        virtual void ListAdjustSize(const wxSize& size=wxDefaultSize);

    private:
        wxInt32 m_processSelectEvent;
        wxUint32 m_mouseLastMove;

        static void CreateImageList();

#ifdef __WXMSW__
        void OnEraseBackground(wxEraseEvent& event);
#endif
        void OnMouseMove(wxMouseEvent& event);
        void OnMenu(wxCommandEvent& event);
        void OnContextMenu(wxContextMenuEvent& event);
        void OnColumnLeftClick(wxListEvent& event);
        void OnColumnRightClick(wxListEvent& event);
        void OnKeyDown(wxKeyEvent &event);
        void OnItem(wxListEvent& event);
        void OnToolTip(CslToolTipEvent& event);

        DECLARE_EVENT_TABLE()

    protected:
        static wxImageList ListImageList;

        VoidPointerArray m_selected;

        CslListSort m_sortHelper;
        CslListColumns m_columns;

        wxInt32 ListFindItem(void *data);
        virtual bool ListFindItemCompare(void *data1, void *data2) { return data1==data2; }

        virtual void OnListUpdate() {};

        void ListSort(wxInt32 column=-1);
        virtual void OnListSort() {};

        virtual wxWindow* GetScreenShotWindow() { return this; }
        virtual wxString GetScreenShotFileName();

        virtual void GetToolTipText(wxInt32 row, CslToolTipEvent& event) { }

        virtual wxSize GetImageListSize()
        {
            wxInt32 x, y;
            if (ListImageList.GetSize(0, x, y))
                return wxSize(x, y);
            return wxDefaultSize;
        }
};

#endif //CSLLISTCTRL_H
