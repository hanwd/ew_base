#include "ewc_base/evt/evt_option.h"
#include "ewc_base/wnd/wnd_model.h"
#include "ewc_base/wnd/wnd_maker.h"
#include "ewc_base/wnd/impl_wx/iwnd_combo.h"
#include "ewc_base/wnd/impl_wx/iwnd_bookbase.h"
#include "ewc_base/wnd/impl_wx/iwnd_treectrl.h"


EW_ENTER

IPageItem::IPageItem(EvtDynamicPage* p) :dpage(p)
{
	binit = false;
	if (dpage)
	{
		sname = dpage->m_sName;
	}
}

EvtDynamicPage::EvtDynamicPage(const String& s,const String& n)
	:basetype(s)
	,wm(WndManager::current())
	,m_sName(n.empty()?s:n)
{
	
}


wxWindow* EvtDynamicPage::CreatePage(wxWindow* w,EvtManager* p)
{
	WndMaker km(w);


	km.border(2).sv(0);
	km.width(120).flags(km.IWND_WITHCOLON).align(km.ALIGN_RIGHT).sv(1);
	km.propotion(1).sv(2);
	km.flags(km.IWND_EXPAND).sv(3);

	km.set_evtmgr(p);

	DoCreatePage(km);

	wxWindow* pw=km.get();
	if (!pw)
	{
		pw = new wxWindow(w, -1);
	}

	m_pVald=km.vald;

	if(m_pValdGroup)
	{
		m_pValdGroup->append(m_pVald.get());
	}
	return pw;
}

void  EvtDynamicPage::DoCreatePage(WndMaker&)
{

}

EvtDynamicPageScript::EvtDynamicPageScript(const String& s, const String& f,const String& n) :basetype(s,n), m_sScriptFile(f)
{

}

void EvtDynamicPageScript::DoCreatePage(WndMaker& km)
{
	Executor ewsl;
	ewsl.push(new CallableMaker(km));
	if (!ewsl.execute_file(m_sScriptFile,1))
	{
		return;
	}
}



class IWnd_combo_selection : public IWnd_combo
{
public:

	IPageSelector& Target;

	IWnd_combo_selection(wxWindow* p, const WndPropertyEx& h, IPageSelector& t) :IWnd_combo(p, h), Target(t)
	{
		this->Connect(wxEVT_COMBOBOX, wxCommandEventHandler(IWnd_combo_selection::OnCommandIntChanged));
	}

	~IWnd_combo_selection()
	{

	}

	void LoadPages(const String& s)
	{
		EvtGroup* gp = WndManager::current().evtmgr.get_group(s);
		if (!gp) return;
		LoadChildren(gp);
	}

	void LoadChildren(EvtGroup* gp)
	{

		wxArrayString aOptions;

		gp->PrepareItems();
		for (size_t i = 0; i<gp->size(); i++)
		{
			if (EvtDynamicPage* pg = dynamic_cast<EvtDynamicPage*>((*gp)[i].get()))
			{
				Target.items.push_back(pg);
				aOptions.push_back(str2wx(pg->m_sId));
			}
		}

		Append(aOptions);
	}

	void OnCommandIntChanged(wxCommandEvent& evt)
	{
		int id = evt.GetInt();
		m_pVald->DoSetValue(id);
	}


};


class ValidatorSelector : public Validator
{
public:
	IPageSelector& Target;
	ValidatorSelector(IPageSelector& t) :Target(t)
	{
		ival = 0;
		pwin = NULL;
	}

	~ValidatorSelector()
	{

	}

	String sval;
	int32_t ival;

	IWnd_combo_selection* pwin;

	virtual bool DoWndExecute(IWndParam& cmd)
	{
		if (cmd.action == IDefs::ACTION_TRANSFER2WINDOW)
		{
			DoSetValue(ival);
		}
		return Target.p_sel && Target.p_sel->WndExecute(cmd);
	}

	virtual bool DoSetValue(const String& val)
	{
		sval = val;
		for (size_t i = 0; i < Target.items.size(); i++)
		{
			if (Target.items[i].sname == val)
			{
				return DoSetValue(i);
			}
		}

		pwin->SetValue(str2wx(val));
		ival = -1;
		Target.pbook->SelPage(NULL);
		Target.p_sel.reset(NULL);

		return false;
	}

	virtual bool DoGetValue(String& val)
	{
		val = sval;
		return true;
	}

	virtual bool DoSetValue(int32_t val)
	{
		if (size_t(val) >= Target.items.size())
		{
			ival = val;
			sval.Printf("unknown(%d)", ival);
			pwin->ChangeValue(str2wx(sval));
			Target.pbook->SelPage(NULL);
			Target.p_sel.reset(NULL);
			return false;
		}
		ival = val;
		sval = Target.items[val].sname;

		pwin->SetSelection(val);
		//pwin->SetValue(str2wx(sval));

		IPageItem& item(Target.items[ival]);


		if (!item.dpage)
		{
			Target.p_sel = item.pvald;
			Target.pbook->SelPage(NULL);
			return true;
		}

		if (!item.binit)
		{
			item.binit = true;
			item.ppage = item.dpage->CreatePage(Target.pbook, Target.pevtmgr);
			item.pvald = item.dpage->m_pVald;
			if (item.pvald)
			{
				item.pvald->WndExecuteEx(IDefs::ACTION_TRANSFER2WINDOW);
				item.pvald->WndExecuteEx(IDefs::ACTION_UPDATECTRL);
			}
		}

		Target.p_sel = item.pvald;
		Target.pbook->SelPage(item.ppage);

		return true;

	}

	virtual bool DoGetValue(int32_t& val)
	{
		val = ival;
		return true;
	}

};
void IPageSelector::init_combo(wxWindow* p, const String& s)
{

	EW_ASSERT(!pvald);
	pvald.reset(new ValidatorSelector(*this));

	IWnd_combo_selection* pwin_type = new IWnd_combo_selection(p, WndProperty().flags(IDefs::IWND_READONLY), *this);
	pwin_type->m_pVald.reset(pvald.get());

	static_cast<ValidatorSelector&>(*pvald).pwin = pwin_type;

	pwin_type->LoadPages(s);

	pbook = new IWnd_bookbase(p, WndProperty());
	ptype = pwin_type;



}



class WndOptionPage : public ObjectNode
{
public:

	IPageSelector& Target;
	int index;

	WndOptionPage(IPageSelector& t,int i)
		:ObjectNode(t.items[i].dpage->MakeLabel())
		, Target(t)
		, index(i)
	{

	}

	typedef ObjectNode basetype;

	virtual bool OnTreeEvent(enumTreeEventType t, ITreeEventData& d)
	{
		IPageItem& item(Target.items[index]);

		if (!basetype::OnTreeEvent(t, d)) return false;
		if (t == TREE_SELECTED)
		{
			if (!item.ppage)
			{
				item.dpage->m_pValdGroup.cast_and_set(Target.pvald.get());
				item.ppage = item.dpage->CreatePage(Target.pbook,Target.pevtmgr);
				if (item.dpage->m_pVald)
				{
					item.dpage->m_pVald->WndExecuteEx(IDefs::ACTION_TRANSFER2WINDOW);
					item.dpage->m_pVald->WndExecuteEx(IDefs::ACTION_UPDATECTRL);
				}
			}

			Target.p_sel = item.pvald;
			Target.pbook->SelPage(item.ppage);

			return true;
		}
		return true;
	}

};


class WndOptionFolder : public ObjectNodeGroup
{
public:

	WndOptionFolder(const String& s) :ObjectNodeGroup(s){}

	typedef ObjectNodeGroup basetype;
	virtual bool OnTreeEvent(enumTreeEventType t, ITreeEventData& d)
	{
		if (!basetype::OnTreeEvent(t, d)) return false;
		return true;
	}

};


class IWnd_treectrl_selection : public IWnd_treectrl
{
public:

	IPageSelector& Target;

	IWnd_treectrl_selection(wxWindow* p, const WndPropertyEx& h, IPageSelector& t) :IWnd_treectrl(p, h), Target(t)
	{
		
	}



	void LoadPages(const String& s)
	{
		EvtGroup* gp = WndManager::current().evtmgr.get_group(s);
		if (!gp) return;
		LoadChildren(gp,NULL);
	}

	void LoadChildren(EvtGroup* gp, ObjectNodeGroup* pd)
	{
		gp->PrepareItems();
		ObjectNodeGroup* nd = new WndOptionFolder(gp->MakeLabel());
		if (pd)
		{
			ExAppendNode(nd, pd);
		}
		else
		{
			nd->flags.add(ObjectNode::FLAG_EXPANDED);
			ExSetRoot(nd);
		}

		IPageItem item;

		for (size_t i = 0; i < gp->size(); i++)
		{
			EvtBase* pe = (*gp)[i].get();
			EvtGroup* sg = dynamic_cast<EvtGroup*>(pe);
			if (sg)
			{
				LoadChildren(sg, nd);
			}
			else if(item.dpage.cast_and_set(pe))
			{
				Target.items.push_back(item);
				ExAppendNode(new WndOptionPage(Target,Target.items.size()-1), nd);
			}
		}
	}
};

void IPageSelector::init_tree(wxWindow* p, const String& s)
{

	pvald.reset(new ValidatorGroup);
	pbook = new IWnd_bookbase(p, WndProperty());

	IWnd_treectrl_selection* pwin_type = new IWnd_treectrl_selection(p, WndProperty(),*this);
	pwin_type->LoadPages(s);
	ptype = pwin_type;



}


EW_LEAVE
