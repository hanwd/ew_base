#include "ewc_base/wnd/wnd_maker.h"
#include "ewc_base/evt/evt_manager.h"
#include "ewc_base/wnd/wnd_info.h"
#include "ewc_base/wnd/impl_wx/topwindow.h"
#include "ewc_base/wnd/impl_wx/iwnd_bookbase.h"
#include "ewc_base/wnd/impl_wx/iwnd_notebook.h"

#include "ewa_base/scripting/callable_memfunc.h"

#include "wx/notebook.h"


EW_ENTER


void WndProperty::reset()
{
	m_nId=-1;
	m_aExtra.clear();
	m_tFlags.clr(0);

	m_aSProperties.clear();
	m_aNProperties.clear();

	nvalue(-1);
	border(3);
	x(-1);
	y(-1);
	width(-1);
	height(-1);
	propotion(0);
	align(IDefs::ALIGN_LEFT);
	valign(IDefs::ALIGN_TOP);


}

void IValueOptionBook::add(wxWindow* p, int val, const String& t)
{
	OptionItem item;
	item.ivalue = val;
	item.page = p;
	item.value = t;

	aOption.push_back(item);
}

int IValueOptionBook::get_value(int v)
{
	if (size_t(v) <= aOption.size())
	{
		return aOption[v].ivalue;
	}
	return -1;
}

int IValueOptionBook::get_selection(int v)
{
	for (size_t i = 0; i < aOption.size(); i++)
	{
		if (aOption[i].ivalue == v) return i;
	}
	return -1;
}

void IValueOptionBook::set_selection(size_t i)
{
	if (i < aOption.size() && pbook)
	{
		pbook->SelPage(aOption[i].page);
	}
}


WndMaker::WndMaker(IWindowPtr t):pwin(NULL)
{
	icur.hwnd=t;
	pevtmgr.reset(&EvtManager::current());
}


WndMaker::WndMaker(WndModel* t):pwin(NULL)
{
	set_model(t);
}

void WndMaker::vd_set(Validator* vd)
{
	icur.p_vg=dynamic_cast<ValidatorGroup*>(vd);
	if(!vald)
	{
		if(icur.p_vg)
		{
			vald=icur.p_vg;
			icur.p_vg=NULL;
		}
		else
		{
			vald.reset(new ValidatorGroup);
			vald->append(vd);
		}
		return;
	}

	if(!vd) return;

	vald->append(vd);

	if (!icur.prop.book().empty())
	{
		auto& item(bookdata[icur.prop.book()]);
		if (!item) item.reset(new IValueOptionBook());
		vd->SetOptionData(item.get());
	}

	if(icur.p_vg)
	{

		aValdStack.push_back(vald);
		vald=icur.p_vg;
	}
}


wxSizerFlags MakeSizerFlag(const WndProperty& h)
{
	wxSizerFlags flag;
	flag.Proportion(h.propotion()).Border(wxALL,h.border());
	if(h.flags().get(WndMaker::IWND_EXPAND)) flag.Expand();
	return flag;
}

wxAuiPaneInfo MakePaneInfo(const WndProperty& h)
{
	wxAuiPaneInfo info;
	wxString pane=str2wx(h.pane());
	if(pane=="centerpane")
	{
		info.CenterPane();
		return info;
	}

	info.Name(pane).Caption(pane);
	if(h.width()>0)
	{
		info.BestSize(wxSize(h.width(),h.height()));
	}
	info.Left();

	return info;
}

static wxAuiManager* GetAuiManager(wxWindow* w)
{
	{
		IWnd_topwindow<wxFrame>* topw=dynamic_cast<IWnd_topwindow<wxFrame>*>(w);
		if(topw) return &topw->amgr;
	}
	{
		IWnd_topwindow<wxDialog>* topw=dynamic_cast<IWnd_topwindow<wxDialog>*>(w);
		if(topw) return &topw->amgr;
	}
	return NULL;
}

void WndMaker::_check_hwnd()
{
	if(!icur.hwnd)
	{
		Exception::XError("no window is defined");
	}
}


WndMaker& WndMaker::szr(wxSizer* psz,const WndProperty& p)
{
	awin.push_back(icur);
	icur.p_sz=psz;
	icur.prop=p;
	icur.p_vg.reset(NULL);

	ld(0);
	return *this;
}


WndMaker& WndMaker::row(const WndProperty& p)
{
	_check_hwnd();

	if(p.label()=="")
	{
		return szr(new wxBoxSizer(wxVERTICAL),p);
	}
	else
	{
		wxStaticBox *box = new wxStaticBox(icur.hwnd, wxID_ANY, str2wx(p.label()));
		return szr(new wxStaticBoxSizer(box,wxVERTICAL),p);
	}
}

WndMaker& WndMaker::col(const WndProperty& p)
{
	_check_hwnd();

	if(p.label()=="")
	{
		return szr(new wxBoxSizer(wxHORIZONTAL),p);
	}
	else
	{
		wxStaticBox *box = new wxStaticBox(icur.hwnd, wxID_ANY, str2wx(p.label()));
		return szr(new wxStaticBoxSizer(box,wxHORIZONTAL),p);
	}
}

void WndMaker::set_evtmgr(EvtManager* p)
{
	if(p)
	{
		pevtmgr.reset(p);
	}
	else
	{
		pevtmgr.reset(&EvtManager::current());
	}
}


void WndMaker::set_model(WndModel* p)
{
	pmodel.reset(p);

	pevtmgr.reset(NULL);
	icur.hwnd=NULL;
	vald.reset(NULL);
	ld(0);

	if(pmodel)
	{
		pevtmgr.reset(&pmodel->local_evtmgr);
		icur.hwnd=pmodel->GetParent();
	}
	else
	{
		pevtmgr.reset(&EvtManager::current());
	}



}

WndMaker& WndMaker::end()
{
	if(awin.empty())
	{
		Exception::XError("tag not match!");
	}

	if(icur.p_vg)
	{
		vald=aValdStack.back();
		aValdStack.pop_back();
	}

	itmp=icur;
	icur=awin.back();
	awin.pop_back();

	if(itmp.p_sz) // sizer end
	{
		if(icur.p_sz)
		{
			wxSizerFlags flag=MakeSizerFlag(itmp.prop);
			icur.p_sz->Add(itmp.p_sz,flag);
		}
		else if(itmp.prop.width()==-1)
		{
			icur.hwnd->SetSizer(itmp.p_sz);
		}
		else
		{
			icur.hwnd->SetSizer(itmp.p_sz);
		}
	}
	else if(itmp.hwnd) // window end
	{

		if(icur.p_sz) // window in sizer
		{
			wxSizerFlags flag=MakeSizerFlag(itmp.prop);
			icur.p_sz->Add(itmp.hwnd,flag);
		}
		else if(!icur.hwnd) // top window
		{
			wxAuiManager *pmgr=GetAuiManager(itmp.hwnd);
			if(pmgr)
			{
				if(pmgr->GetManagedWindow()==NULL && itmp.prop.sprops("mgr")=="aui")
				{
					pmgr->SetManagedWindow(itmp.hwnd);
				}

				if(pmgr->GetManagedWindow()!=NULL)
				{
					pmgr->Update();
				}
			}

			pwin=itmp.hwnd;

			if(pmodel)
			{
				pmodel->DoUpdate(*this);
			}

			if(itmp.prop.width()<0)
			{
				itmp.hwnd->Fit();
				itmp.hwnd->Layout();
			}
			else
			{
				itmp.hwnd->Layout();
			}

			set_model(NULL);

		}
		else if(itmp.prop.page()!="") // window in notebook
		{
			if (IWnd_notebook* book = dynamic_cast<IWnd_notebook*>(icur.hwnd))
			{
				book->IAddPage(itmp.hwnd,itmp.prop);
			}
			else if (IWnd_bookbase* book = dynamic_cast<IWnd_bookbase*>(icur.hwnd))
			{
				book->IAddPage(itmp.hwnd, itmp.prop);
			}
			else
			{
				Exception::XError("no book defined");
			}

			if (icur.p_bk)
			{
				icur.p_bk->add(itmp.hwnd, itmp.prop.nvalue(), itmp.prop.page());
			}
		}
		else if(itmp.prop.pane()!="")
		{
			wxAuiManager *pmgr=GetAuiManager(icur.hwnd);

			if(!pmgr)
			{
				Exception::XError("no aui_manager");
			}

			if(pmgr->GetManagedWindow()==NULL)
			{
				pmgr->SetManagedWindow(icur.hwnd);
			}
		
			wxAuiPaneInfo info=MakePaneInfo(itmp.prop);
			pmgr->AddPane(itmp.hwnd,info);
	
		}
		else if(awin.empty())
		{
			pwin=itmp.hwnd;

			if(pmodel)
			{
				pmodel->DoUpdate(*this);
			}

			if(itmp.prop.width()<0)
			{
				itmp.hwnd->Fit();
				itmp.hwnd->Layout();
			}
			else
			{
				itmp.hwnd->Layout();
			}

			if (pmodel)
			{
				pmodel->m_pWindow=pwin;
			}
		}
		else
		{
			pwin=itmp.hwnd;
		}
	}
	else if(icur.p_sz) // space
	{
		if(itmp.prop.propotion()==0)
		{
			icur.p_sz->AddSpacer(itmp.prop.width());
		}
		else
		{
			icur.p_sz->AddStretchSpacer(itmp.prop.propotion());
		}
	}
	else
	{
		Exception::XError("no sizer is defined");
	}

	return *this;
}

EvtBase* WndMaker::find(const String& s)
{
	if(s=="") return NULL;
	return pevtmgr->chained_get(s);
}


WndMaker& WndMaker::add(const String& t,const WndProperty& p)
{
	return win(t,p).end();
}

WndMaker& WndMaker::load_script(const String& fp)
{
	Executor ewsl;
	ewsl.push(new CallableMaker(*this));
	bool flag = ewsl.execute_file(fp, 1);
	if (!flag)
	{
		System::LogError("%s execute failed", fp);
	}
	return *this;
}

WndMaker& WndMaker::win(const String& t,const WndProperty& p)
{

	if(!icur.hwnd)
	{
		if(t!="frame"&&t!="dialog")
		{
			Exception::XError("window_parent is NULL");
		}
	}

	if(t=="row")
	{
		return row(p);
	}
	if(t=="col")
	{
		return col(p);
	}

	if(t!="space")
	{
		EvtBase* vp=find(p.name());
		if (vp)
		{
			p.id(vp->m_nId);
		}

		if(t=="frame"||t=="dialog")
		{
			if(!pmodel)
			{
				set_model(new WndModel);
				pmodel->flags.add(EvtBase::FLAG_CHECKED);
			}
			if(p.flags().get(IDefs::IWND_NO_CLOSE))
			{
				pmodel->flags.add(WndModel::FLAG_NO_CLOSE);
			}
			if(p.flags().get(IDefs::IWND_AUTO_FIT))
			{
				pmodel->flags.add(WndModel::FLAG_AUTO_FIT);
			}
		}

		wxWindow *w=WndInfoManger::current().Create(t,icur.hwnd,p);
		if(w) return win(w,p);

		System::LogTrace("unknown wnd type %s",t);
	}

	awin.push_back(icur);
	icur=WndProxyState(p);

	ld(0);
	return *this;
}


WndMaker& WndMaker::add(IWindowPtr t,const WndProperty& p)
{
	return win(t,p).end();
}

WndMaker& WndMaker::win(IWindowPtr t,const WndProperty& p)
{

	wxWindow* w=icur.hwnd;

	awin.push_back(icur);
	icur=WndProxyState(p,t);

	if (icur.prop.book().empty())
	{
		//icur.p_bk = awin.back().p_bk;
	}
	else
	{
		auto& item(bookdata[icur.prop.book()]);
		if (!item) item.reset(new IValueOptionBook());

		if (IWnd_bookbase* p = dynamic_cast<IWnd_bookbase*>(t))
		{
			item->pbook = p;
			icur.p_bk = item;
		}

	}

	if(!t) return *this;

	EvtBase* pv=find(p.name());
	WndInfo* wi=WndInfoManger::current().GetWndInfo(icur.hwnd);

	if(pv) aEvtArray.push_back(pv);

	if(wi)
	{
		wi->Update(*this,icur.hwnd,pv);
	}
	else
	{
		vd_set(NULL);
	}
	

	if(w && t->GetParent()!=w)
	{
		t->Reparent(w);
	}

	ld(0);
	return *this;
}

class CallableFunctionMaker : public CallableFunction
{
public:

	typedef WndMaker& return_type;
	typedef WndMaker target_type;

	CallableFunctionMaker(const String& n):CallableFunction(n){}
	target_type& get_target(Executor& ewsl,CallableData* pd);
	int set_result(Executor& ewsl,return_type);
};


class CallableFunctionMakerNewModel : public CallableFunctionMaker
{
public:
	CallableFunctionMakerNewModel(const String& n):CallableFunctionMaker(n){}

	int __fun_call(Executor& ewsl,int pm)
	{
		WndMaker& km(get_target(ewsl,ewsl.ci1.nbp[StackState1::SBASE_THIS].kptr()));

		if(pm<1) ewsl.kerror("invalid param");		
		String w=variant_cast<String>(ewsl.ci0.nbx[1]);
		km.set_model(new WndModel(w));
		ewsl.ci0.nbx[1]=ewsl.ci1.nbp[StackState1::SBASE_THIS];
		return 1;
	}
};

class CallableFunctionMakerWin : public CallableFunctionMaker
{
public:
	CallableFunctionMakerWin(const String& n):CallableFunctionMaker(n){}

	int __fun_call(Executor& ewsl,int pm)
	{
		WndMaker& km(get_target(ewsl,ewsl.ci1.nbp[StackState1::SBASE_THIS].kptr()));

		if(pm<1) ewsl.kerror("invalid param");		
		String w=variant_cast<String>(ewsl.ci0.nbx[1]);

		if(pm<2)
		{
			km.win(w);
		}
		else
		{
			km.win(w,get_target(ewsl,ewsl.ci0.nbx[2].kptr()));
		}

		ewsl.ci0.nbx[1]=ewsl.ci1.nbp[StackState1::SBASE_THIS];
		return 1;
	}
};

class CallableFunctionMakerAdd : public CallableFunctionMaker
{
public:
	CallableFunctionMakerAdd(const String& n):CallableFunctionMaker(n){}

	int __fun_call(Executor& ewsl,int pm)
	{
		WndMaker& km(get_target(ewsl,ewsl.ci1.nbp[StackState1::SBASE_THIS].kptr()));

		if(pm<1) ewsl.kerror("invalid param");		
		String w=variant_cast<String>(ewsl.ci0.nbx[1]);

		if(pm<2)
		{
			km.add(w);
		}
		else
		{
			km.add(w,get_target(ewsl,ewsl.ci0.nbx[2].kptr()));
		}


		ewsl.ci0.nbx[1]=ewsl.ci1.nbp[StackState1::SBASE_THIS];
		return 1;
	}
};

class CallableFunctionMakerRow : public CallableFunctionMaker
{
public:
	CallableFunctionMakerRow(const String& n):CallableFunctionMaker(n){}

	int __fun_call(Executor& ewsl,int pm)
	{
		WndMaker& km(get_target(ewsl,ewsl.ci1.nbp[StackState1::SBASE_THIS].kptr()));

		if(pm<1)
		{
			km.row();
		}
		else
		{
			km.row(get_target(ewsl,ewsl.ci0.nbx[1].kptr()));
		}

		ewsl.ci0.nbx[1]=ewsl.ci1.nbp[StackState1::SBASE_THIS];
		return 1;
	}
};



class CallableFunctionMakerCol : public CallableFunctionMaker
{
public:
	CallableFunctionMakerCol(const String& n):CallableFunctionMaker(n){}

	int __fun_call(Executor& ewsl,int pm)
	{
		WndMaker& km(get_target(ewsl,ewsl.ci1.nbp[StackState1::SBASE_THIS].kptr()));
		if(pm<1)
		{
			km.col();
		}
		else
		{
			km.col(get_target(ewsl,ewsl.ci0.nbx[1].kptr()));
		}

		ewsl.ci0.nbx[1]=ewsl.ci1.nbp[StackState1::SBASE_THIS];
		return 1;
	}
};

class CallableFunctionMakerEnd : public CallableFunctionMaker
{
public:
	CallableFunctionMakerEnd(const String& n):CallableFunctionMaker(n){}

	int __fun_call(Executor& ewsl,int pm)
	{
		WndMaker& km(get_target(ewsl,ewsl.ci1.nbp[StackState1::SBASE_THIS].kptr()));
		km.end();
		ewsl.ci0.nbx[1]=ewsl.ci1.nbp[StackState1::SBASE_THIS];
		return 1;
	}
};


VariantTable& get_callable_maker_table()
{
	static VariantTable value;
	if(value.empty())
	{
		value["new"].kptr(new CallableFunctionMakerNewModel("km.new"));
		value["win"].kptr(new CallableFunctionMakerWin("km.win"));
		value["add"].kptr(new CallableFunctionMakerAdd("km.add"));
		value["row"].kptr(new CallableFunctionMakerRow("km.row"));
		value["col"].kptr(new CallableFunctionMakerCol("km.col"));
		value["end"].kptr(new CallableFunctionMakerEnd("km.end"));

		typedef CallableFunctionTargetFunc1<CallableFunctionMaker,int> func1_type_int;
		typedef CallableFunctionTargetFunc1<CallableFunctionMaker,String> func1_type_str;
		typedef CallableFunctionTargetFunc2<CallableFunctionMaker,int,int> func2_type_int;
		typedef CallableFunctionTargetFunc2<CallableFunctionMaker,String,String> func2_type_str;

		#define WNDMAKER_FUNCTION_I1(X) value[#X].kptr(new func1_type_int("km."#X,&WndMaker::X));
		#define WNDMAKER_FUNCTION_S1(X) value[#X].kptr(new func1_type_str("km."#X,&WndMaker::X));
		#define WNDMAKER_FUNCTION_I2(X) value[#X].kptr(new func2_type_int("km."#X,&WndMaker::X));
		#define WNDMAKER_FUNCTION_S2(X) value[#X].kptr(new func2_type_str("km."#X,&WndMaker::X));


		WNDMAKER_FUNCTION_I1(ld);
		WNDMAKER_FUNCTION_I1(sv);

		WNDMAKER_FUNCTION_I1(propotion);
		WNDMAKER_FUNCTION_I1(border);
		WNDMAKER_FUNCTION_I1(align);
		WNDMAKER_FUNCTION_I1(valign);
		WNDMAKER_FUNCTION_I1(width);
		WNDMAKER_FUNCTION_I1(height);
		WNDMAKER_FUNCTION_I1(flags);
		WNDMAKER_FUNCTION_I1(nvalue);
		WNDMAKER_FUNCTION_S1(icon);
		WNDMAKER_FUNCTION_S1(name);
		WNDMAKER_FUNCTION_S1(label);
		WNDMAKER_FUNCTION_S1(value);
		WNDMAKER_FUNCTION_S1(page);
		WNDMAKER_FUNCTION_S1(pane);
		WNDMAKER_FUNCTION_S1(desc);
		WNDMAKER_FUNCTION_S1(hint);
		WNDMAKER_FUNCTION_S1(help);
		WNDMAKER_FUNCTION_S1(tooltip);
		WNDMAKER_FUNCTION_S1(add_extra);
		WNDMAKER_FUNCTION_S1(book);
		WNDMAKER_FUNCTION_S1(load_script);


		WNDMAKER_FUNCTION_I2(position);
		WNDMAKER_FUNCTION_I2(size);
		WNDMAKER_FUNCTION_S2(sprops);

		#define WNDMAKER_DEFINE_C(X) value[#X].reset((int)IDefs::X);

		WNDMAKER_DEFINE_C(IWND_EXPAND);
		WNDMAKER_DEFINE_C(IWND_READONLY);
		WNDMAKER_DEFINE_C(IWND_DISABLED);
		WNDMAKER_DEFINE_C(IWND_MULTILINE);
		WNDMAKER_DEFINE_C(IWND_WITHCOLON);
		WNDMAKER_DEFINE_C(IWND_PAGEONLY);
		WNDMAKER_DEFINE_C(IWND_HORIZONTAL);
		WNDMAKER_DEFINE_C(IWND_VERTICAL);
		WNDMAKER_DEFINE_C(IWND_NOHEADER);

		WNDMAKER_DEFINE_C(ALIGN_LEFT);
		WNDMAKER_DEFINE_C(ALIGN_CENTER);
		WNDMAKER_DEFINE_C(ALIGN_RIGHT);
		WNDMAKER_DEFINE_C(ALIGN_TOP);
		WNDMAKER_DEFINE_C(ALIGN_MIDDLE);
		WNDMAKER_DEFINE_C(ALIGN_BOTTOM);

		WNDMAKER_DEFINE_C(IWND_PROCESS_TAB);
		WNDMAKER_DEFINE_C(IWND_TE_READONLY);
		WNDMAKER_DEFINE_C(IWND_NO_RESIZABLE);
		WNDMAKER_DEFINE_C(IWND_NO_CAPTION);
		WNDMAKER_DEFINE_C(IWND_NO_SYS_MENU);
		WNDMAKER_DEFINE_C(IWND_NO_CLOSE_BOX);

		WNDMAKER_DEFINE_C(IWND_NO_CLOSE);
		WNDMAKER_DEFINE_C(IWND_AUTO_FIT);

	}
	return value;
}

CallableMaker::CallableMaker(WndModel* p):CallableTableProxy(get_callable_maker_table()),_km(p),km(_km)
{
	flags.clr(FLAG_READONLY|FLAG_SET_THIS);
}

CallableMaker::CallableMaker(WndMaker& k):CallableTableProxy(get_callable_maker_table()),km(k)
{
	flags.clr(FLAG_READONLY|FLAG_SET_THIS);
}


int CallableMaker::__fun_call(Executor& ewsl,int pm)
{
	WndModel* pmodel=NULL;
	if(pm==1)
	{
		CallableWndModelPtr* p_callable_modelptr=dynamic_cast<CallableWndModelPtr*>(ewsl.ci0.nbx[1].kptr());
		if(p_callable_modelptr)
		{
			pmodel=p_callable_modelptr->pmodel.get();
		}
		else if (dynamic_cast<CallableMaker*>(ewsl.ci0.nbx[1].kptr()))
		{
			return 1;
		}

		if(!pmodel)
		{
			ewsl.kerror("invalid param");
		}
	}

	ewsl.ci0.nbx[1].kptr(new CallableMaker(pmodel));
	return 1;
}

WndMaker& CallableFunctionMaker::get_target(Executor& ewsl,CallableData* pd)
{
	CallableMaker* pm=dynamic_cast<CallableMaker*>(pd);
	if(!pm) ewsl.kerror("invalid call");
	return pm->km;
}

int CallableFunctionMaker::set_result(Executor& ewsl,return_type)
{
	ewsl.ci0.nbx[1]=ewsl.ci1.nbp[StackState1::SBASE_THIS];
	return 1;
}

EW_LEAVE
