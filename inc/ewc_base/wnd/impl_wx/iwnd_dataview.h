#ifndef __UI_IWND_DATAVIEW__
#define __UI_IWND_DATAVIEW__

#include "ewc_base/wnd/impl_wx/window.h"
#include "ewc_base/evt/evt_array.h"

#include "wx/dataview.h"
#include "wx/renderer.h"

EW_ENTER


class IValueDataModel :public wxDataViewVirtualListModel
{
public:

	IValueDataModel(){}

	virtual bool IsContainer( const wxDataViewItem & ) const
	{
		return false;
	}

	virtual bool GetAttrByRow( unsigned int , unsigned int , wxDataViewItemAttr & )
	{
		return true;
	}

	int GetColNewItem()
	{
		for(int i=0;i<(int)m_pData->aColumnInfo.size();i++)
		{
			if(m_pData->aColumnInfo[i]->flags.get(COLUMNFLAG_NEW_ITEM))
			{
				return i;
			}
		}
		for(int i=0;i<(int)m_pData->aColumnInfo.size();i++)
		{
			if(m_pData->aColumnInfo[i]->flags.get(COLUMNFLAG_CRITICAL))
			{
				return i;
			}
		}
		return -1;
	}

	bool NextItem(int& row,int& col)
	{
		if(!m_pData) return false;

		for(size_t i=0;i<m_pData->aColumnInfo.size();i++)
		{
			size_t n=(col+1+i)%m_pData->aColumnInfo.size();
			if(m_pData->aColumnInfo[n]->flags.get(COLUMNFLAG_CRITICAL))
			{
				col=n;
				if(col+1+i>=m_pData->aColumnInfo.size()) row+=1;
				return true;
			}
		}

		return false;

	}

	bool TestEdit(size_t row,size_t col)
	{
		if(!m_pData) return false;
		return m_pData->TestEdit(row,col);
	}
	
	void DeleteItem( const wxDataViewItem &item )
	{
		if(!m_pData) return;

		unsigned int row = GetRow( item );
		wxArrayInt rows;
		rows.Add(row);
		m_pData->Delete(&rows[0],1);
		RowsDeleted( row );
		Reset(m_pData->GetRowCount());
	}

	bool MoveItem(size_t row1,size_t row2)
	{
		if(!m_pData) return false;
		if(m_pData->MoveItem(row1,row2))
		{
			size_t r1=std::min(row1,row2);
			size_t r2=std::max(row1,row2);
			for(size_t i=r1;i<=r2;i++)
			{
				RowChanged(i);
			}
		}
		return true;
	}

	void DeleteItems( const wxDataViewItemArray &m_aItems )
	{
		if(!m_pData) return;
		if(m_aItems.empty()) return;

		wxArrayInt rows;
		for (size_t i = 0; i < m_aItems.GetCount(); i++)
		{
			unsigned int row = GetRow( m_aItems[i] );
			rows.Add(row);
		}
		m_pData->Delete(&rows[0],m_aItems.size());
		RowsDeleted(rows);
		Reset(m_pData->GetRowCount());
	}

	wxDataViewColumn* CreateColumn(size_t n);

	virtual unsigned int GetColumnCount() const
	{
		if(!m_pData) return 0;
		return m_pData->aColumnInfo.size();
	}

    virtual wxString GetColumnType( unsigned int col ) const
    {
		if(!m_pData||col>=m_pData->aColumnInfo.size()) return "";

		switch(m_pData->aColumnInfo[col]->type)
		{
		case COLUMNTYPE_ICON:
			return "wxBitmap";
		case COLUMNTYPE_ICONTEXT:
			return "wxDataViewIconText";
		default:
			return "string";
		}
    }

	virtual unsigned int GetColumnWidth(unsigned int n) const
	{
		if(!m_pData) return 80;
		return m_pData->aColumnInfo[n]->width;
	}

	void Resize(unsigned int size)
	{
		if(!m_pData) return;
		return m_pData->Resize(size);
	}

	virtual unsigned int GetRowCount()
	{
		if(!m_pData) return 0;
		return m_pData->GetRowCount();	
	}


	virtual void GetValueByRow( wxVariant &variant,
		unsigned int row, unsigned int col ) const
	{
		if(!m_pData) return;

		ICellInfo cell(variant,row,col);
		m_pData->GetValueByRow(cell);
	}

	virtual bool SetValueByRow( const wxVariant &variant,unsigned int row, unsigned int col )
	{
		if(!m_pData) return false;

		const ICellInfo cell((wxVariant &)variant,row,col);
		bool f= m_pData->SetValueByRow(cell);
		Reset(m_pData->GetRowCount());
		return f;
	}
	ValidatorHolderT<EvtProxyArray> m_pData;
};


class DLLIMPEXP_EWC_BASE IWnd_dataview : public wxDataViewCtrl 
{

public:

	enum
	{
		ID_MU_BEGIN = wxID_HIGHEST+1,
		ID_MU_DELETE,
		ID_MU_NEW_ITEM,
		ID_MU_INSERT_UP,
		ID_MU_INSERT_DOWN,
		ID_MU_MOVE_UP,
		ID_MU_MOVE_DOWN,
		ID_MU_MOVE_TOP,
		ID_MU_MOVE_BOTTOM,
		ID_MU_END
	};

	IWnd_dataview(wxWindow* p,const WndPropertyEx& h);

	void ReCreateColumns();

	~IWnd_dataview();

	void OnSize(wxSizeEvent &evt);

	void OnCommandEvents(wxCommandEvent& evt);
	void OnContextMenu( wxDataViewEvent &evt );
	void OnActivated( wxDataViewEvent &evt );

	void OnColumnHeaderClick(wxDataViewEvent &evt);
	void OnItemEditingStarting(wxDataViewEvent &evt);
	void OnItemEditingStarted(wxDataViewEvent &evt);
	void OnItemEditingDone(wxDataViewEvent &evt);
	void OnItemValueChanged(wxDataViewEvent &evt);

	void OnSelectionChanged(wxDataViewEvent &evt);

	virtual bool ValueChanged(int row,int col);

	static bool FinishByReturnKey;

	IValueDataModel* GetModel(){return model;}

	wxDataViewItemArray m_aLastSelection;

	void EnsureRowVisible(int row)
	{
		wxDataViewItem item=GetItemByRow(row);
		if(item.IsOk())
		{
			EnsureVisible(item);
		}
	}

	DataPtrT<Validator> m_pVald;
protected:
	IValueDataModel* model;
	DECLARE_EVENT_TABLE();
};

EW_LEAVE

#endif
