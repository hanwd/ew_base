#include "ewa_base/basic/formater.h"
#include "ewa_base/basic/string.h"
EW_ENTER


template<typename S>
void FormatStateT<S>::str_append(const String& s)
{
	str_append(s.c_str());
}

template<typename S>
void FormatStateT<S>::str_append_s(const String& x)
{
	str_append(x.c_str());
}

template<typename S>
void FormatStateT<S>::str_append_s(const wchar_t* x)
{
	str_append(String(x));
}

template class FormatStateT<StringBuffer<char> >;
template class FormatStateT<FormatBuffer >;


//template bool StringFormater::handle(FormatStateSb& st);
//template bool StringFormater::handle(FormatStateFb& st);

void FormatState0::init(const wchar_t* p)
{
	phold.reset(new StringBuffer<char>);
	IConv::unicode_to_utf8(*phold,p,std::char_traits<wchar_t>::length(p));
	init(phold->data());
}


EW_LEAVE
