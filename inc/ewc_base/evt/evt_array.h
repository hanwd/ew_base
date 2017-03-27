#ifndef __H_EW_UI_VALUEPROXY_ARRAY__
#define __H_EW_UI_VALUEPROXY_ARRAY__

#include "ewc_base/evt/evt_element.h"

class wxDataViewColumn;

EW_ENTER


enum
{
	COLUMNTYPE_NUM,		// number of row, 1..n, *
	COLUMNTYPE_CHECKBOX,
	COLUMNTYPE_STRING,  // string
	COLUMNTYPE_INT,		// integer
	COLUMNTYPE_DOUBLE,	// double
	COLUMNTYPE_EXPR,
	COLUMNTYPE_ICON,
	COLUMNTYPE_ICONTEXT,
};

enum
{
	COLUMNFLAG_EDITABLE	=1<<0, // ediable
	COLUMNFLAG_CRITICAL	=1<<1, // can move to this column by enter key
	COLUMNFLAG_NEW_ITEM =1<<2, // new item if this column is modified
	COLUMNFLAG_SORTABLE =1<<3,
	COLUMNFLAG_SORTDESC =1<<4,
};

class DLLIMPEXP_EWC_BASE IValueColumn : public ObjectData
{
public:
	int type;
	int width;
	String name;
	BitFlags flags;
	int ndot;

	IValueColumn();
	bool IsReadonly() const;
	bool IsCritical() const;

	static wxDataViewColumn* CreateDataViewColumn(IValueColumn&,size_t n);


	virtual void var2val(wxVariant& variant,String& v) const;
	virtual void var2val(wxVariant& variant,float& v) const;
	virtual void var2val(wxVariant& variant,double& v) const;
	virtual void var2val(wxVariant& variant,int32_t& v) const;
	virtual void var2val(wxVariant& variant,int64_t& v) const;
	virtual void var2val(wxVariant& variant,bool& v) const;
	virtual void var2val(wxVariant& variant,Variant& v) const;

	virtual void val2var(wxVariant& variant,const String& v) const;
	virtual void val2var(wxVariant& variant,float v) const;
	virtual void val2var(wxVariant& variant,double v) const;
	virtual void val2var(wxVariant& variant,int32_t v) const;
	virtual void val2var(wxVariant& variant,int64_t v) const;
	virtual void val2var(wxVariant& variant,bool v) const;
	virtual void val2var(wxVariant& variant, Variant& v) const;

	bool val_cmp(double r1, double r2)
	{
		if (std::isnan(r1) && std::isnan(r2)) return false;
		if (std::isnan(r1)) return false;
		if (std::isnan(r2)) return true;
		return flags.get(COLUMNFLAG_SORTDESC) ? r2<r1 : r1<r2;
	}

	bool val_cmp(float r1, float r2)
	{
		if (std::isnan(r1) && std::isnan(r2)) return false;
		if (std::isnan(r1)) return false;
		if (std::isnan(r2)) return true;
		return flags.get(COLUMNFLAG_SORTDESC) ? r2<r1 : r1<r2;
	}

	template<typename T>
	bool val_cmp(const T& r1, const T& r2)
	{
		return flags.get(COLUMNFLAG_SORTDESC) ? r2<r1:r1<r2;
	}

};

class DLLIMPEXP_EWC_BASE ICellInfo
{
public:
	ICellInfo(wxVariant& v,size_t r,size_t c):variant(v),row(r),col(c){}

	 wxVariant &variant;
	 size_t row;
	 size_t col;
};

template<typename T>
class DLLIMPEXP_EWC_BASE IValueColumnT : public IValueColumn
{
public:
	typedef T itemtype;
	virtual bool SetValue(itemtype& val, const ICellInfo& cell)
	{
		EW_UNUSED(val);
		EW_UNUSED(cell);
		return false;
	}

	virtual void GetValue(const itemtype& val,ICellInfo& cell) const =0;

	virtual bool Compare(const itemtype&, const itemtype&){ return false; }


};


template<typename T,typename E>
class DLLIMPEXP_EWC_BASE IValueColumnExT : public IValueColumnT<T>
{
public:
	typedef T itemtype;


	off_t offset;

	IValueColumnExT(const String& s,int w,const E& o)
	{
		name=s;
		width=w;
		offset=(char*)&o-(char*)NULL;
		type=tl::is_same_type<E,bool>::value?COLUMNTYPE_CHECKBOX:COLUMNTYPE_STRING;
		flags.add(COLUMNFLAG_EDITABLE);
	}

	virtual bool SetValue(itemtype& val,const ICellInfo& cell)
	{
		if(!flags.get(COLUMNFLAG_EDITABLE))
		{
			return false;
		}
		const itemtype& item(val);
		E* p=(E*)(((char*)&item)+offset);
		var2val(cell.variant,*p);
		return true;
	}

	virtual void GetValue(const itemtype& val,ICellInfo& cell) const
	{
		const itemtype& item(val);
		E* p=(E*)(((char*)&item)+offset);
		val2var(cell.variant,*p);
	}

	virtual bool Compare(const itemtype& lhs, const itemtype& rhs)
	{
		E* p1=(E*)(((char*)&lhs)+offset);
		E* p2=(E*)(((char*)&rhs)+offset);
		
		return val_cmp(*p1,*p2);

	}

};

class DLLIMPEXP_EWC_BASE EvtProxyArray : public EvtProxyBase
{
public:


	virtual Validator* CreateValidator(wxWindow*);
	
	virtual void GetValueByRow(ICellInfo& cell) const=0;
	virtual bool SetValueByRow(const ICellInfo& cell)=0;

	virtual size_t GetRowCount() const =0;
	virtual bool MoveItem(size_t row1,size_t row2)=0;
	virtual void Delete(const int*,size_t)=0;

	virtual bool TestEdit(size_t row,size_t col);
	virtual void Resize(size_t size);

	virtual bool OnColumnHeaderClick(int){ return false; }

	void AddColumnNum(){aColumnInfo.push_back(new IValueColumn());}
	
	arr_1t<DataPtrT<IValueColumn> > aColumnInfo;

};

template<typename T,typename V=arr_1t<T> >
class DLLIMPEXP_EWC_BASE EvtProxyArrayT : public EvtProxyArray
{
public:

	typedef EvtProxyArray basetype;
	typedef T itemtype;

	EvtProxyArrayT()
	{

	}
	
	virtual bool TestNewRow()
	{
		if(flags.get(FLAG_READONLY)) return false;

		if(def_data==tmp_data)
		{
			return false;
		}
		tmp_value.push_back(tmp_data);
		tmp_data=def_data;
		return true;
	}


	void Delete(const int* p,size_t n)
	{
		arr_1t<bool> _vecbool;
		_vecbool.resize(tmp_value.size(),false);
		for(size_t i=0;i<n;i++)
		{
			size_t n=p[i];
			if(n<_vecbool.size())
			{
				_vecbool[n]=true;
			}
		}
		size_t i=0;
		size_t j=0;
		for(i=0;i<_vecbool.size();i++)
		{
			if(_vecbool[i])
			{
				continue;
			}
			if(i!=j)
			{
				tmp_value[j]=tmp_value[i];
			}
			j++;
		}
		tmp_value.resize(j);
	}

	size_t GetRowCount() const
	{
		bool E=!flags.get(FLAG_READONLY);
		size_t sz=tmp_value.size()+(E?1:0);
		return sz;
	}

	virtual void GetValueByRow(ICellInfo& cell) const
	{
		if( aColumnInfo[cell.col]->type==COLUMNTYPE_NUM)
		{
			if (cell.row == tmp_value.size())
			{
				WxImpl<String>::set(cell.variant,"*");
			}
			else
			{
				WxImpl<long>::set(cell.variant,(long)cell.row+1);
			}	
			return;
		}

		const IValueColumnT<itemtype>* mvccol=static_cast<const IValueColumnT<itemtype>*>(aColumnInfo[cell.col].get());
		if(cell.row< tmp_value.size())
		{
			mvccol->GetValue(tmp_value[cell.row],cell);
		}
		else if(cell.row==tmp_value.size())
		{
			mvccol->GetValue(tmp_data,cell);
		}
		
	}

	virtual bool SetValueByRow(const ICellInfo& cell)
	{
		if( aColumnInfo[cell.col]->type==0)
		{
			return false;
		}

		IValueColumnT<itemtype>* mvccol=static_cast<IValueColumnT<itemtype>*>(aColumnInfo[cell.col].get());
		if(cell.row< tmp_value.size())
		{
			mvccol->SetValue(tmp_value[cell.row],cell);
		}
		else if(cell.row==tmp_value.size())
		{
			mvccol->SetValue(tmp_data,cell);
			TestNewRow();
		}
		else
		{
			return false;
		}
		return true;
	}


	virtual bool MoveItem(size_t row1,size_t row2)
	{
		if(flags.get(FLAG_READONLY)) return false;

		size_t rows=tmp_value.size();
		if(row1>=rows||row2>=rows)
		{
			return false;
		}

		T tmp(tmp_value[row1]);
		int d=row2>row1?1:-1;
		for(size_t i=row1;i!=row2;i+=d)
		{
			tmp_value[i]=tmp_value[i+d];
		}
		tmp_value[row2]=tmp;
		return true;
	}

	String _m_sSortName;

	virtual bool OnColumnHeaderClick(int n)
	{

		if (n >= (int)aColumnInfo.size())
		{
			return false;
		}

		IValueColumnT<T>* pk = dynamic_cast<IValueColumnT<T>*>(aColumnInfo[n].get());
		if (!pk) return false;

		if (!pk->flags.get(COLUMNFLAG_SORTABLE))
		{
			return false;
		}

		const String& name(pk->name);
		if (name != _m_sSortName)
		{
			_m_sSortName = name;
		}
		else
		{
			pk->flags.inv(COLUMNFLAG_SORTDESC);
		}

		std::stable_sort(tmp_value.begin(), tmp_value.end(), [pk](const T& lhs, const T& rhs)
		{
			return pk->Compare(lhs, rhs);
		});

		return true;
	}

	V tmp_value;
	itemtype def_data;	// default data
	itemtype tmp_data;
};

template<typename T,typename V=arr_1t<T> >
class DLLIMPEXP_EWC_BASE EvtProxyArrayExT : public EvtProxyArrayT<T,V>
{
public:
	typedef EvtProxyArrayT<T,V> basetype;

	static const T& ze()
	{
		return *(T*)NULL;
	}

	void AddColumn(IValueColumn* col)
	{
		aColumnInfo.push_back(col);
		OnColumnAdded(*aColumnInfo.back());
	}

	void AddColumn(const String& n,int w,const String& v)
	{
		AddColumn(new IValueColumnExT<T, String>(n, w, v));
	}

	void AddColumn(const String& n,int w,const float& v)
	{
		AddColumn(new IValueColumnExT<T, float>(n, w, v));
	}

	void AddColumn(const String& n,int w,const double& v)
	{
		AddColumn(new IValueColumnExT<T, double>(n, w, v));
	}

	void AddColumn(const String& n,int w,const int& v)
	{
		AddColumn(new IValueColumnExT<T, int>(n, w, v));
	}

	void AddColumn(const String& n,int w,const bool& v)
	{
		AddColumn(new IValueColumnExT<T, bool>(n, w, v));
	}

	virtual void OnColumnAdded(IValueColumn&){}


};

template<typename T,typename V=arr_1t<T> >
class DLLIMPEXP_EWC_BASE EvtProxyArrayRefT : public EvtProxyArrayT<T,V>
{
public:
	typedef EvtProxyArrayT<T,V> basetype;

	V& value;
	EvtProxyArrayRefT(V& v):value(v){}
	virtual bool InternalTransfer2Model(){value=basetype::tmp_value;return true;}
	virtual bool InternalTransfer2Window(){basetype::tmp_value=value;return true;}

};


template<typename T,typename V=arr_1t<T> >
class DLLIMPEXP_EWC_BASE EvtProxyArrayExRefT : public EvtProxyArrayExT<T,V>
{
public:
	typedef EvtProxyArrayExT<T,V> basetype;

	V& value;
	EvtProxyArrayExRefT(V& v):value(v){}
	virtual bool InternalTransfer2Model(){value=basetype::tmp_value;return true;}
	virtual bool InternalTransfer2Window(){basetype::tmp_value=value;return true;}

};

template<typename T>
class EvtProxyArraySimpleT;

template<>
class DLLIMPEXP_EWC_BASE EvtProxyArraySimpleT<String>: public EvtProxyArrayRefT<String>
{
public:
	EvtProxyArraySimpleT(arr_1t<String>& v);
};

template<>
class DLLIMPEXP_EWC_BASE EvtProxyArraySimpleT<int>: public EvtProxyArrayRefT<int>
{
public:
	EvtProxyArraySimpleT(arr_1t<int>& v);
};

template<>
class DLLIMPEXP_EWC_BASE EvtProxyArraySimpleT<double>: public EvtProxyArrayRefT<double>
{
public:
	EvtProxyArraySimpleT(arr_1t<double>& v);

};

EW_LEAVE
#endif
