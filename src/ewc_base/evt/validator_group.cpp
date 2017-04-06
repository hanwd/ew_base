#include "ewc_base/evt/validator_group.h"
#include "ewc_base/app/data_defs.h"

EW_ENTER


ValidatorGroup::ValidatorGroup()
{

}

ValidatorGroup::~ValidatorGroup()
{

}

bool ValidatorGroup::OnWndEvent(IWndParam& cmd,int phase)
{
	for(size_t i=0;i<size();i++)
	{
		(*this)[i]->OnWndEvent(cmd,phase);
	}
	return true;
}

bool ValidatorGroup::WndExecute(IWndParam& cmd)
{
	bool flag=true;
	for(size_t i=0;i<size();i++)
	{
		Validator* v=(*this)[i].get();
		if(!v->WndExecute(cmd))
		{
			return false;
		}
	}
	return true;
}



ValidatorGroupEx::ValidatorGroupEx()
{
	pageid=-1;
}

bool ValidatorGroupEx::WndExecute(IWndParam& cmd)
{
	if(pageid<0||cmd.action==IDefs::ACTION_WINDOW_INIT)
	{
		return basetype::WndExecute(cmd);
	}
		
	return (*this)[pageid]->WndExecute(cmd);

}


ValidatorGroupBook::ValidatorGroupBook()
{


}

void ValidatorGroupBook::IAddPage(wxWindow *, const WndProperty&)
{


}

void ValidatorGroupBook::ISelPage(wxWindow* w)
{
	if (!w)
	{
		m_pSelectedPage = NULL;
		return;
	}

	if (w == m_pSelectedPage)
	{
		return;
	}

	m_pSelectedPage = w;

	wxWindow* b = w->GetParent();
	wxWindow* p = b->GetParent();

	if (flags.get(FLAG_DYNAMIC_SIZE_X | FLAG_DYNAMIC_SIZE_Y))
	{
		b->Fit();
		p->Fit();
		p->Layout();
		p->Refresh();
	}
	else if (flags.get(FLAG_FITINSIDE))
	{
		b->FitInside();
	}
	else if (flags.get(FLAG_FITLAYOUT))
	{
		b->Fit();
		p->Layout();
		p->Refresh();
	}
}

bool ValidatorGroupBook::IGetBestSize(int& x, int& y)
{
	const wxWindow* w = GetWindow();

	const wxWindowList &list(w->GetChildren());
	//wxSize size1(basetype::DoGetBestSize());

	wxSize size1(x, y);
	if (m_pSelectedPage) size1 = m_pSelectedPage->GetBestSize();

	wxSize size2(x, y);
	for (wxWindowList::const_iterator it = list.begin(); it != list.end(); ++it)
	{
		wxSize _wsz = (*it)->GetBestSize();
		if (_wsz.x>size2.x) size2.x = _wsz.x;
		if (_wsz.y>size2.y) size2.y = _wsz.y;
	}

	if (!flags.get(FLAG_DYNAMIC_SIZE_X))
	{
		size1.x = std::max(size1.x, size2.x);
	}

	if (!flags.get(FLAG_DYNAMIC_SIZE_Y))
	{
		size1.y = std::max(size1.y, size2.y);
	}

	if (size1.x<m_v2MinSize[0]) size1.x = m_v2MinSize[0];
	if (size1.y<m_v2MinSize[1]) size1.y = m_v2MinSize[1];

	x = size1.x;
	y = size1.y;

	return true;

}

void ValidatorGroupBook::ISetMinSize(int x, int y)
{
	m_v2MinSize.set2(x, y);
}

void ValidatorGroupBook::IEnableDynamicSize(bool bflag)
{
	flags.set(FLAG_DYNAMIC_SIZE_X | FLAG_DYNAMIC_SIZE_Y, bflag);
}



EW_LEAVE
