#include "ewc_base/evt/evt_array.h"
#include "ewc_base/evt/validator.h"
#include "ewc_base/wnd/impl_wx/window.h"

EW_ENTER


IValueColumn::IValueColumn()
{
	name="No.";
	type=COLUMNTYPE_NUM;
	width=40;
	ndot = -1;
}

bool IValueColumn::IsReadonly() const
{
	return type==COLUMNTYPE_NUM||!flags.get(COLUMNFLAG_EDITABLE);
}

bool IValueColumn::IsCritical() const
{
	return flags.get(COLUMNFLAG_CRITICAL);
}


Validator* EvtProxyArray::CreateValidator(wxWindow* w)
{
	WndInfo* wi=WndInfoManger::current().GetWndInfo(w);
	if(!wi) return NULL;
	return wi->CreateValidator(w,this);
}

bool EvtProxyArray::TestEdit(size_t row,size_t col)
{
	size_t n=GetRowCount();

	if(row+1<n)
	{
		if(col<aColumnInfo.size())
		{
			return !aColumnInfo[col]->IsReadonly();
		}		
		return false;
	}

	if(row+1==n)
	{
		if(col<aColumnInfo.size())
		{
			return aColumnInfo[col]->flags.get(COLUMNFLAG_NEW_ITEM);
		}		
		return false;
	}
	return false;
}

void EvtProxyArray::Resize(size_t size)
{
	unsigned int ncol = aColumnInfo.size();
	int w0 = 40;
	aColumnInfo[0]->width=w0;
	unsigned i=1;
	while(i<ncol&&ncol>1)
	{
		int wi = (size - w0)/(ncol-1);
		aColumnInfo[i]->width=wi;
		i++;
	}
}



EvtProxyArraySimpleT<String>::EvtProxyArraySimpleT(arr_1t<String>& v)
	:EvtProxyArrayRefT<String>(v)
{
	class IValueColumnSimple : public IValueColumnT<String>
	{
	public:
		IValueColumnSimple()
		{
			name="Value";
			width=240;
			type=COLUMNTYPE_STRING;
			flags.clr(COLUMNFLAG_EDITABLE|COLUMNFLAG_CRITICAL|COLUMNFLAG_NEW_ITEM);
		}

		virtual bool SetValue(itemtype& val,const ICellInfo& cell)
		{
			val=wx2str(cell.variant.GetString());return true;
		}

		virtual void GetValue(const itemtype& val,ICellInfo& cell) const
		{
			cell.variant=str2wx(val);
		}

	};

	AddColumnNum();
	aColumnInfo.push_back(new IValueColumnSimple);
}


EvtProxyArraySimpleT<int>::EvtProxyArraySimpleT(arr_1t<int>& v):EvtProxyArrayRefT<int>(v)
{
	class IValueColumnSimple : public IValueColumnT<int>
	{
	public:
		IValueColumnSimple()
		{
			name="Value";
			width=120;
			type=1;
			flags.clr(COLUMNFLAG_EDITABLE|COLUMNFLAG_CRITICAL|COLUMNFLAG_NEW_ITEM);
		}

		virtual bool SetValue(itemtype& val,const ICellInfo& cell)
		{
			val=cell.variant.GetLong();return true;
		}

		virtual void GetValue(const itemtype& val,ICellInfo& cell) const
		{
			cell.variant=(long)val;
		}
	};

	AddColumnNum();
	aColumnInfo.push_back(new IValueColumnSimple);
}


EvtProxyArraySimpleT<double>::EvtProxyArraySimpleT(arr_1t<double>& v):EvtProxyArrayRefT<double>(v)
{
	class IValueColumnSimple : public IValueColumnT<int>
	{
	public:
		IValueColumnSimple()
		{
			name="Value";
			width=120;
			type=1;
			flags.clr(COLUMNFLAG_EDITABLE|COLUMNFLAG_CRITICAL|COLUMNFLAG_NEW_ITEM);
		}
		virtual bool SetValue(itemtype& val,const ICellInfo& cell)
		{
			val=cell.variant.GetDouble();return true;
		}

		virtual void GetValue(const itemtype& val,ICellInfo& cell) const
		{
			cell.variant=wxVariant(val);
		}
	};

	AddColumnNum();
	aColumnInfo.push_back(new IValueColumnSimple);
}

EW_LEAVE
