#include "ewc_base/wnd/impl_wx/iwnd_webview.h"

#include <wx/webview.h>
#include <wx/uri.h>
#include <comutil.h> 
#include "wx/msw/webview_ie.h"

#pragma comment(lib,"comsuppw.lib")

#include <olectl.h>
#include <oleidl.h>
#include <exdispid.h>
#include <exdisp.h>
#include <mshtml.h>

EW_ENTER


DISPID IE_FindId(IDispatch *pObj, LPOLESTR pName)
{
	DISPID id = 0;
	if(FAILED(pObj->GetIDsOfNames(IID_NULL,&pName,1,LOCALE_SYSTEM_DEFAULT,&id))) id = -1;
	return id;
}

bool IE_InvokeMethod(IDispatch *pObj, LPOLESTR pName, VARIANT *pVarResult, VARIANT *p, int cArgs)
{
	DISPID dispid = IE_FindId(pObj, pName);
	if(dispid == -1) return false;

	DISPPARAMS ps;
	ps.cArgs = cArgs;
	ps.rgvarg = p;
	ps.cNamedArgs = 0;
	ps.rgdispidNamedArgs = NULL;

	HRESULT hr= pObj->Invoke(dispid, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_METHOD, &ps, pVarResult, NULL, NULL);
	return SUCCEEDED(hr);
}


HRESULT IE_GetProperty(IDispatch *pObj, LPOLESTR pName, VARIANT *pValue)
{
	DISPID dispid = IE_FindId(pObj, pName);
	if(dispid == -1) return E_FAIL;

	DISPPARAMS ps;
	ps.cArgs = 0;
	ps.rgvarg = NULL;
	ps.cNamedArgs = 0;
	ps.rgdispidNamedArgs = NULL;

	return pObj->Invoke(dispid, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_PROPERTYGET, &ps, pValue, NULL, NULL);
}

HRESULT IE_SetProperty(IDispatch *pObj, LPOLESTR pName, VARIANT *pValue)
{
	DISPID dispid = IE_FindId(pObj, pName);
	if(dispid == -1) return E_FAIL;

	DISPPARAMS ps;
	ps.cArgs = 1;
	ps.rgvarg = pValue;
	ps.cNamedArgs = 0;
	ps.rgdispidNamedArgs = NULL;

	return pObj->Invoke(dispid, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_PROPERTYPUT, &ps, NULL, NULL, NULL);
}


wxCOMPtr<IHTMLWindow2>  IE_GetWindow(wxWebView* bw)
{

	wxCOMPtr<IDispatch> dispatch;
	wxCOMPtr<IHTMLDocument2> document;
    wxCOMPtr<IHTMLWindow2> window;

	wxWebViewIE* ie=dynamic_cast<wxWebViewIE*>(bw);
	if(!ie) return window;

	IWebBrowser2* wb=(IWebBrowser2*)ie->GetNativeBackend();

	HRESULT result = wb->get_Document(&dispatch);

	if(!dispatch||!SUCCEEDED(result))
	{
		return window;
	}


	dispatch->QueryInterface(IID_IHTMLDocument2, (void**)&document);

	if(!document)
	{
		return window;
	}

    //wxString language = "javascript";
    document->get_parentWindow(&window);
	return window;

}

/*
enum VARENUM
    {
        VT_EMPTY	= 0,
        VT_NULL	= 1,
        VT_I2	= 2,
        VT_I4	= 3,
        VT_R4	= 4,
        VT_R8	= 5,
        VT_CY	= 6,
        VT_DATE	= 7,
        VT_BSTR	= 8,
        VT_DISPATCH	= 9,
        VT_ERROR	= 10,
        VT_BOOL	= 11,
        VT_VARIANT	= 12,
        VT_UNKNOWN	= 13,
        VT_DECIMAL	= 14,
        VT_I1	= 16,
        VT_UI1	= 17,
        VT_UI2	= 18,
        VT_UI4	= 19,
        VT_I8	= 20,
        VT_UI8	= 21,
        VT_INT	= 22,
        VT_UINT	= 23,
        VT_VOID	= 24,
        VT_HRESULT	= 25,
        VT_PTR	= 26,
        VT_SAFEARRAY	= 27,
        VT_CARRAY	= 28,
        VT_USERDEFINED	= 29,
        VT_LPSTR	= 30,
        VT_LPWSTR	= 31,
        VT_RECORD	= 36,
        VT_INT_PTR	= 37,
        VT_UINT_PTR	= 38,
        VT_FILETIME	= 64,
        VT_BLOB	= 65,
        VT_STREAM	= 66,
        VT_STORAGE	= 67,
        VT_STREAMED_OBJECT	= 68,
        VT_STORED_OBJECT	= 69,
        VT_BLOB_OBJECT	= 70,
        VT_CF	= 71,
        VT_CLSID	= 72,
        VT_VERSIONED_STREAM	= 73,
        VT_BSTR_BLOB	= 0xfff,
        VT_VECTOR	= 0x1000,
        VT_ARRAY	= 0x2000,
        VT_BYREF	= 0x4000,
        VT_RESERVED	= 0x8000,
        VT_ILLEGAL	= 0xffff,
        VT_ILLEGALMASKED	= 0xfff,
        VT_TYPEMASK	= 0xfff
    } ;
*/

bool vv_cast(Variant& v1,const VARIANT& v2)
{
	switch(v2.vt)
	{
	case VT_BSTR:
		{
			StringBuffer<char> sb;
			IConv::unicode_to_ansi(sb, v2.bstrVal, SysStringLen(v2.bstrVal));
			v1.ref<String>()=sb;
		}
		return true;
	case VT_BOOL:
		v1.reset(v2.boolVal!=0);
		return true;
	case VT_I4:
	case VT_I8:
		v1.reset((int64_t)v2.llVal);
		return true;
	case VT_R8:
		v1.reset((double)v2.dblVal);
		return true;
	default:
		return false;
	};

	return true;
}


bool vv_cast(VARIANT& v1,const Variant& v2_)
{
	VariantClear(&v1);

	Variant& v2(const_cast<Variant&>(v2_));
	switch(v2.type())
	{
	case TL_VARIANT_TYPES::id<String>::value :
		{
			String& str(v2.get<String>());			
			v1.vt=VT_BSTR;
			v1.bstrVal=_com_util::ConvertStringToBSTR(str.c_str());
		}
		return true;
	case TL_VARIANT_TYPES::id<bool>::value :
		{
			v1.vt=VT_BOOL;
			v1.intVal=v2.get<bool>();
		}
		return true;
	case TL_VARIANT_TYPES::id<int64_t>::value :
		{
			v1.vt=VT_I4;
			v1.intVal=v2.get<int64_t>();
		}
		return true;
	case TL_VARIANT_TYPES::id<double>::value :
		{
			v1.vt=VT_R8;
			v1.dblVal=v2.get<double>();
		}
		return true;
	default:
		return false;
	}
	return true;
}


class MsVariant : public VARIANT
{
public:
	MsVariant(){VariantInit(this);}
};


bool IWnd_webview::FindObj(const String& fn)
{
	StringBuffer<wchar_t> sb;
	IConv::ansi_to_unicode(sb, fn.c_str(), fn.length());

	wxCOMPtr<IHTMLWindow2> ie(IE_GetWindow(m_pImpl));
	if (!ie) return false;

	MsVariant wnd;

	if (IE_GetProperty(ie, L"window", &wnd) == E_FAIL)
	{
		return false;
	}

	DISPID dispid = IE_FindId(wnd.pdispVal, sb.c_str());
	if (dispid == -1) return false;

	return true;
}

bool IWnd_webview::Invoke(Variant& rt,const String& fn,const arr_1t<Variant>& pm)
{
	wxCOMPtr<IHTMLWindow2> ie(IE_GetWindow(m_pImpl));
	if(!ie) return false;

	MsVariant ret, wnd, ifn;

	StringBuffer<wchar_t> sb;
	IConv::ansi_to_unicode(sb,fn.c_str(),fn.length());

	if(IE_GetProperty(ie, L"window", &wnd)==E_FAIL)
	{
		return false;
	}

	size_t n = pm.size();
	arr_1t<MsVariant> params;
	params.resize(n);

	for(size_t i=0;i<n;i++)
	{
		vv_cast(params[i],pm[n-i-1]);
	}

	bool flag=IE_InvokeMethod(wnd.pdispVal, sb.c_str(), &ret, params.data(),params.size());

	if(flag)
	{
		vv_cast(rt,ret);
	}
	else
	{
		flag = flag;
	}

	return flag;	
}


EW_LEAVE
