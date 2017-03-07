
#include "ewc_base/app/res_bitmap.h"
#include "ewc_base/app/res_manager.h"
#include "ewc_base/wnd/impl_wx/impl_wx.h"
#include <wx/artprov.h>

EW_ENTER

template<typename A>
class serial_helper_test<A,wxBitmap>
{
public:
	typedef wxBitmap type;

	static void g(SerializerReader& ar,type& val)
	{
		int32_t nx=-1,ny=-1,na=0;
		ar & nx & ny;
		if(nx<0||ny<0)
		{
			val=wxNullBitmap;
			return;
		}

		ar & na;
		wxImage img(nx,ny);

		ar.recv((char*)img.GetData(),nx*ny*3);
		if(na)
		{
			img.InitAlpha();
			ar.recv((char*)img.GetAlpha(),nx*ny);
		}	

		val=wxBitmap(img);
	}

	static void g(SerializerWriter& ar,type& val)
	{
		int32_t nx=-1,ny=-1,na=0;
		if(!val.IsOk())
		{
			ar & nx & ny;
			return;
		}
		wxImage img(val.ConvertToImage());
		nx=img.GetWidth();
		ny=img.GetHeight();
		na=img.HasAlpha();

		ar & nx & ny & na;
		ar.send((const char*)img.GetData(),nx*ny*3);
		if(na)
		{
			ar.send((const char*)img.GetAlpha(),nx*ny);
		}

	}
};

class BitmapBundle
{
public:
	enum
	{
		FLAG_SCALED	=1<<0,
	};

	void set(const wxBitmap& bmp,int w=-1)
	{
		if(!bmp.IsOk()) return;
		if(w<0||bmp.GetHeight()==w)
		{
			bmp_normal=bmp;
			flags.del(FLAG_SCALED);
		}
		else
		{
			bmp_normal=wxBitmap(bmp.ConvertToImage().Scale(w,w));
			flags.add(FLAG_SCALED);
		}
	}

	bool update(const wxBitmap& bmp,int w)
	{
		if(bmp_normal.IsOk()) return true;
		if(!bmp.IsOk()) return false;
		set(bmp,w);
		return true;
	}

	bool IsOk()
	{
		return bmp_normal.IsOk();
	}

	wxBitmap bmp_normal;
	wxBitmap bmp_disabled;
	BitFlags flags;

	void Serialize(Serializer& ar)
	{
		ar & flags;

		if(ar.is_reader() && flags.get(FLAG_SCALED))
		{
			bmp_normal=wxNullBitmap;
			bmp_disabled=wxNullBitmap;
			return;
		}
		
		if(ar.is_writer() && flags.get(FLAG_SCALED))
		{
			return;
		}

		ar & bmp_normal;
		ar & bmp_disabled;

	}
};


class CallableBitmap : public CallableData
{
public:

	wxBitmap sample;
	bst_map<int,BitmapBundle> bmp_scaled;

	virtual CallableData* DoClone(ObjectCloneState&){return this;}

	bool LoadFile(const String& s)
	{
		arr_1t<String> a;a.push_back(s);
		return LoadFile(a);
	}

	bool LoadFile(const arr_1t<String>& s);

	void AddBitmap(const wxBitmap& bmp);

	BitmapBundle& GetBundle(int w);

	wxBitmap& GetBitmap(int w=-1);

	bool update(IToolItemPtr item)
	{
		wxToolBarBase* tb=item->GetToolBar();
		wxSize sz=tb->GetToolBitmapSize();

		BitmapBundle& bundle(GetBundle(sz.y));
		if(!bundle.IsOk())
		{
			return false;
		}

		item->SetNormalBitmap(bundle.bmp_normal);
		item->SetDisabledBitmap(bundle.bmp_disabled);
		return true;
	
	}

	bool update(IMenuItemPtr item)
	{
		if(item->IsCheckable()) return true;

		BitmapBundle& bundle(GetBundle(16));
		if(!bundle.IsOk())
		{
			return true;
		}

		item->SetBitmap(bundle.bmp_normal);
		item->SetDisabledBitmap(bundle.bmp_disabled);
		return true;
	}


	static const wxBitmap& null_bitmap()
	{
		static wxBitmap bmp;
		return bmp;
	}

	void Serialize(Serializer& ar)
	{
		basetype::Serialize(ar);

		ar & sample;
		ar & bmp_scaled;

	}


	DECLARE_OBJECT_INFO(CallableBitmap,ObjectInfo)

};

IMPLEMENT_OBJECT_INFO(CallableBitmap,ObjectInfo)


bool CallableBitmap::LoadFile(const arr_1t<String>& s)
{
	wxBitmap bmp;
	for(size_t i=0;i<s.size();i++)
	{
		if(s[i].find('.')<0)
		{
			AddBitmap(wxArtProvider::GetIcon(str2wx(s[i]),wxART_MENU));
			AddBitmap(wxArtProvider::GetIcon(str2wx(s[i]),wxART_TOOLBAR));
		}
		else
		{
			if(!bmp.LoadFile(str2wx(s[i]),wxBITMAP_TYPE_PNG)) return false;
			AddBitmap(bmp);
		}
	}
	return true;
}

void CallableBitmap::AddBitmap(const wxBitmap& bmp)
{
	if(!bmp.IsOk()) return;

	int sz=bmp.GetWidth();
	bmp_scaled[sz].set(bmp);

	if(!sample.IsOk()||sz>sample.GetWidth())
	{
		sample=bmp;
	}
}


BitmapBundle& CallableBitmap::GetBundle(int w)
{
	BitmapBundle& bundle(bmp_scaled[w]);
	bundle.update(sample,w);
	return bundle;
}

wxBitmap& CallableBitmap::GetBitmap(int w)
{
	if(w<=0) return sample;

	BitmapBundle &bmp(bmp_scaled[w]);
	if(!bmp.bmp_normal.IsOk())
	{
		bmp.update(sample,w);
	}

	return bmp.bmp_normal;
}


BitmapHolder::BitmapHolder()
{

}

void BitmapHolder::Serialize(Serializer& ar)
{
	ar & m_refData;
}

BitmapHolder::operator bool()
{
	return m_refData;
}

void BitmapHolder::SetData(CallableData* d)
{
	CallableBitmap* p=dynamic_cast<CallableBitmap*>(d);
	if(!p) return;

	m_refData.reset(p);
}


class CallableBitmapGroup : public CallableTableEx
{
public:

	DECLARE_OBJECT_INFO(CallableBitmapGroup,ObjectInfo)

	CallableBitmapGroup();

	void set(const String& id,const arr_1t<String>& bmps);
	void beg(const String& id){tmp_bmps.clear();tmp_id=id;}
	void add(const String& bmp){tmp_bmps.append(bmp);}
	void end(){set(tmp_id,tmp_bmps);}

	void link(const String& id,const String& ref)
	{
		BitmapHolder& bmp(bitmaps[id]);
		bmp=bitmaps[ref];
	}

	BitmapHolder get(const String& id)
	{
		BitmapHolder& bmp(bitmaps[id]);
		if(!bmp) bmp.AddBitmap(wxNullBitmap);
		return bmp;
	}

	BitmapHolder get(int id)
	{
		return bitmaps.get(id).second;
	}

	int find(const String& id)
	{
		return bitmaps.find2(id);
	}

	wxImageList imglists[3];

	size_t size(){return bitmaps.size();}

	wxImageList* get_imglist(int w)
	{
		EW_ASSERT(w==16||w==24||w==32);

		wxImageList& imglist(imglists[(w/8)%3]);

		if(imglist.GetHIMAGELIST()==NULL)
		{
			imglist.Create(w,w,true,0);

			for(size_t i=0;i<size();i++)
			{
				BitmapHolder p=bitmaps.get(i).second;
				imglist.Add(p.GetBitmap(w));		
			}
		}

		return &imglist;
	}

	void Serialize(Serializer& ar)
	{
		basetype::Serialize(ar);
		ar & bitmaps;
	}

protected:

	indexer_map<String,BitmapHolder> bitmaps;
	arr_1t<String> tmp_bmps;
	String tmp_id;
};

IMPLEMENT_OBJECT_INFO(CallableBitmapGroup,ObjectInfo)


wxImageList* BitmapGroupHolder::get_imglist(int w)
{
	CallableBitmapGroup& icons(*(CallableBitmapGroup*)m_refData.get());
	return icons.get_imglist(w);
}

void BitmapGroupHolder::set(const String& id,const arr_1t<String>& bmps)
{
	CallableBitmapGroup& icons(*(CallableBitmapGroup*)m_refData.get());
	icons.set(id,bmps);
}

void BitmapGroupHolder::beg(const String& id)
{
	CallableBitmapGroup& icons(*(CallableBitmapGroup*)m_refData.get());
	icons.beg(id);
}

void BitmapGroupHolder::add(const String& bmp)
{
	CallableBitmapGroup& icons(*(CallableBitmapGroup*)m_refData.get());
	icons.add(bmp);
}

void BitmapGroupHolder::end()
{
	CallableBitmapGroup& icons(*(CallableBitmapGroup*)m_refData.get());
	icons.end();
}

BitmapHolder BitmapGroupHolder::get(const String& id)
{
	if(id=="Find")
	{
		AtomicSpin::noop();
	}
	CallableBitmapGroup& icons(*(CallableBitmapGroup*)m_refData.get());
	return icons.get(id);
}

BitmapHolder BitmapGroupHolder::get(int id)
{
	CallableBitmapGroup& icons(*(CallableBitmapGroup*)m_refData.get());
	return icons.get(id);
}

int BitmapGroupHolder::find(const String& id)
{
	CallableBitmapGroup& icons(*(CallableBitmapGroup*)m_refData.get());
	return icons.find(id);
}

const wxBitmap& BitmapGroupHolder::get_bitmap(const String& id,int w)
{
	BitmapHolder bmp=GetData()->get(id);
	return bmp.GetBitmap(w);
}

CallableBitmapGroup* BitmapGroupHolder::GetData()
{
	return (CallableBitmapGroup*)m_refData.get();
}

bool BitmapHolder::update(wxIconBundle& iconbundle) const
{
	int wd[]={16,24,32,48,64};

	for(int i=0;i<sizeof(wd)/sizeof(wd[0]);i++)
	{
		const wxBitmap& bmp(GetBitmap(wd[i]));
		if(!bmp.IsOk())
		{
			return false;
		}

		wxIcon icon;
		icon.CopyFromBitmap(bmp);
		iconbundle.AddIcon(icon);
	}
	return true;
}

const wxBitmap& BitmapHolder::GetBitmap(int w) const
{
	CallableBitmap* p=(CallableBitmap*)m_refData.get();
	if(!p) return CallableBitmap::null_bitmap();

	return p->GetBitmap(w);
}


bool BitmapHolder::LoadFile(const String& s)
{
	arr_1t<String> a;a.push_back(s);
	return LoadFile(a);
}

bool BitmapHolder::LoadFile(const arr_1t<String>& s)
{
	if(!m_refData) m_refData.reset(new CallableBitmap);
	return ((CallableBitmap*)m_refData.get())->LoadFile(s);
}

void BitmapHolder::AddBitmap(const wxBitmap& bmp)
{
	if(!m_refData) m_refData.reset(new CallableBitmap);
	CallableBitmap* p=(CallableBitmap*)m_refData.get();
	return p->AddBitmap(bmp);
}

bool BitmapHolder::update(IToolItemPtr toolitem) const
{
	CallableBitmap* hcb = (CallableBitmap*)m_refData.get();
	if (hcb && hcb->update(toolitem)) return true;

	hcb=(CallableBitmap*)ResManager::current().bmp_missing.GetData();
	if (hcb && hcb->update(toolitem)) return true;

	return false;
}

bool BitmapHolder::update(IMenuItemPtr menuitem) const
{
	CallableBitmap* p=(CallableBitmap*)m_refData.get();
	if(!p) return false;
	return p->update(menuitem);
}



class IResBitmapGroupBase : public CallableFunction
{
public:
	IResBitmapGroupBase(const String& s):CallableFunction(s){}

	CallableBitmapGroup* get_bitmap_group(Executor& ewsl)
	{
		CallableBitmapGroup* p=ResManager::current().icons.GetData();
		if(!p) ewsl.kerror("invalid group");
		return p;
	}
};

class IResBitmapGroupGet : public IResBitmapGroupBase
{
public:
	IResBitmapGroupGet():IResBitmapGroupBase("ui.res.get_item"){}

	virtual int __fun_call(Executor& ewsl,int pm)
	{
		ewsl.check_pmc(this,pm,1);

		CallableBitmapGroup* g=get_bitmap_group(ewsl);
		if(!g) 
		{
			ewsl.ci0.nbx[1].clear();
			return 1;
		}

		String val;
		if(ewsl.ci0.nbx[1].get(val))
		{
			ewsl.ci0.nbx[1].clear();
			return 1;
		}

		ewsl.ci0.nbx[1].kptr(g->get(val).GetData());
	
		return 1;
	};
};

class IResBitmapGroupBeg : public IResBitmapGroupBase
{
public:
	IResBitmapGroupBeg():IResBitmapGroupBase("ui.res.gp_beg"){}

	virtual int __fun_call(Executor& ewsl,int pm)
	{

		ewsl.check_pmc(this,pm,1);
		CallableBitmapGroup* g=get_bitmap_group(ewsl);
		String val;
		if(!ewsl.ci0.nbx[1].get(val))
		{
			ewsl.kerror("invalid param");
		}
		g->beg(val);
		return 0;
	};
};

class IResBitmapGroupEnd : public IResBitmapGroupBase
{
public:
	IResBitmapGroupEnd():IResBitmapGroupBase("ui.res.gp_end"){}
	virtual int __fun_call(Executor& ewsl,int pm)
	{
		ewsl.check_pmc(this,pm,0);
		CallableBitmapGroup* g=get_bitmap_group(ewsl);
		g->end();

		return 0;
	};
};


class IResBitmapGroupAdd : public IResBitmapGroupBase
{
public:
	IResBitmapGroupAdd():IResBitmapGroupBase("ui.res.gp_add"){}
	virtual int __fun_call(Executor& ewsl,int pm)
	{
		CallableBitmapGroup* g=get_bitmap_group(ewsl);
		String val;
		for(int i=1;i<=pm;++i)
		{
			if(!ewsl.ci0.nbx[i].get(val))
			{
				ewsl.kerror("invalid param");
			}
			g->add(val);
		}

		return 0;
	};
};


class IResBitmapGroupSet : public IResBitmapGroupBase
{
public:
	IResBitmapGroupSet():IResBitmapGroupBase("ui.res.gp_set"){}
	virtual int __fun_call(Executor& ewsl,int pm)
	{
		if(pm<1)
		{
			ewsl.kerror("invalid param count");
		}

		CallableBitmapGroup* g=get_bitmap_group(ewsl);
		arr_1t<String> val;
		val.resize(pm);
		for(int i=1;i<=pm;++i)
		{
			if(!ewsl.ci0.nbx[i].get(val[pm-i]))
			{
				ewsl.kerror("invalid param");
			}
		}

		String id=val.back();val.pop_back();

		if(val.size()==1 && val[0].c_str()[0]=='@')
		{
			g->link(id,val[0].c_str()+1);
		}
		else
		{
			g->set(id,val);
		}

		return 0;
	};
};



class IResBitmapGroupSave : public IResBitmapGroupBase
{
public:
	IResBitmapGroupSave():IResBitmapGroupBase("ui.res.save"){}

	virtual int __fun_call(Executor& ewsl,int pm)
	{
		if(pm!=1)
		{
			ewsl.kerror("invalid param count");
		}

		String fn;
		if(!ewsl.ci0.nbx[1].get(fn))
		{
			ewsl.kerror("invalid file");
		}

		SerializerStream ar;

		if(!ar.openfile(fn,FLAG_FILE_WR|FLAG_FILE_CR))
		{

		}

		ar.writer() & ResManager::current().icons;

		return 0;
	};
};

class IResBitmapGroupLoad : public IResBitmapGroupBase
{
public:
	IResBitmapGroupLoad():IResBitmapGroupBase("ui.res.load"){}

	virtual int __fun_call(Executor& ewsl,int pm)
	{
		if(pm!=1)
		{
			ewsl.kerror("invalid param count");
		}

		String fn;
		if(!ewsl.ci0.nbx[1].get(fn))
		{
			ewsl.kerror("invalid file");
		}

		SerializerStream ar;

		if(!ar.openfile(fn,FLAG_FILE_RD))
		{

		}

		ar.reader() & ResManager::current().icons;

		return 0;
	};
};

CallableBitmapGroup::CallableBitmapGroup()
{

}

void CallableBitmapGroup::set(const String& id,const arr_1t<String>& bmps)
{
	BitmapHolder& bmp(bitmaps[id]);
	bmp.LoadFile(bmps);
}



BitmapGroupHolder::BitmapGroupHolder()
{
	CG_GGVar &gi(CG_GGVar::current());

	gi.add(new IResBitmapGroupSet);
	gi.add(new IResBitmapGroupGet);
	gi.add(new IResBitmapGroupAdd);
	gi.add(new IResBitmapGroupBeg);
	gi.add(new IResBitmapGroupEnd);

	gi.add(new IResBitmapGroupLoad);
	gi.add(new IResBitmapGroupSave);

	m_refData.reset(new CallableBitmapGroup);
}


void BitmapGroupHolder::Serialize(Serializer& ar)
{
	ar & m_refData;
}

EW_LEAVE
