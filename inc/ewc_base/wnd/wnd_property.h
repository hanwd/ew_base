#ifndef __H_EW_UI_WINPROPERTY__
#define __H_EW_UI_WINPROPERTY__

#include "ewc_base/config.h"

EW_ENTER

class DLLIMPEXP_EWC_BASE WndProperty
{
public:

	WndProperty(){reset();}


	WndProperty& flags(int f)
	{
		m_tFlags.clr(f);
		return *this;
	}

	const BitFlags& flags() const
	{
		return m_tFlags;
	}

	WndProperty& id(int32_t v)
	{
		m_nId=v;
		return *this;
	}

	const WndProperty& id(int32_t v) const
	{
		m_nId=v;
		return *this;
	}

	int32_t id() const
	{
		return m_nId;
	}


	WndProperty& sprops(const String& n,const String& v)
	{
		m_aSProperties[n]=v;
		return *this;
	}

	const String& sprops(const String& v) const
	{
		return m_aSProperties[v];
	}

#define IWINPROPERTY_SPROPERTY(X)\
	WndProperty& X(const String& v){return sprops(#X,v);}\
	const String& X() const{return sprops(#X);}

	IWINPROPERTY_SPROPERTY(icon);
	IWINPROPERTY_SPROPERTY(name);
	IWINPROPERTY_SPROPERTY(label);
	IWINPROPERTY_SPROPERTY(value);
	IWINPROPERTY_SPROPERTY(page);
	IWINPROPERTY_SPROPERTY(pane);
	IWINPROPERTY_SPROPERTY(desc);
	IWINPROPERTY_SPROPERTY(hint);
	IWINPROPERTY_SPROPERTY(help);
	IWINPROPERTY_SPROPERTY(book);
	IWINPROPERTY_SPROPERTY(tooltip);

#undef IWINPROPERTY_SPROPERTY


	WndProperty& nprops(const String& n,int v)
	{
		m_aNProperties[n]=v;
		return *this;
	}

	int nprops(const String& v) const
	{
		return m_aNProperties[v];
	}


#define IWINPROPERTY_NPROPERTY(X)\
	WndProperty& X(int v){return nprops(#X,v);}\
	int X() const{return nprops(#X);}

	IWINPROPERTY_NPROPERTY(nvalue);
	IWINPROPERTY_NPROPERTY(align);
	IWINPROPERTY_NPROPERTY(valign);
	IWINPROPERTY_NPROPERTY(width);
	IWINPROPERTY_NPROPERTY(height);
	IWINPROPERTY_NPROPERTY(x);
	IWINPROPERTY_NPROPERTY(y);
	IWINPROPERTY_NPROPERTY(nb_dir);
	IWINPROPERTY_NPROPERTY(propotion);
	IWINPROPERTY_NPROPERTY(border);


#undef IWINPROPERTY_NPROPERTY

	WndProperty& add_extra(const String& s)
	{
		m_aExtra.push_back(s);
		return *this;
	}

	const arr_1t<String>& extra() const
	{
		return m_aExtra;
	}


	WndProperty& size(int32_t w,int32_t h)
	{
		width(w);
		height(h);
		return *this;
	}

	WndProperty& position(int32_t x_,int32_t y_)
	{
		x(x_);
		y(y_);
		return *this;
	}



	void reset();


	BitFlags m_tFlags;	
	mutable int32_t m_nId;
	arr_1t<String> m_aExtra;
	mutable bst_map<String,String> m_aSProperties;
	mutable bst_map<String,int> m_aNProperties;
};

EW_LEAVE
#endif
