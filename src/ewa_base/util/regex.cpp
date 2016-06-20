#include "ewa_base/util/regex.h"
#include "regex_parser.h"
#include "regex_impl.h"

EW_ENTER

regex::regex()
{

}

regex::regex(const regex& r):pimpl(r.pimpl)
{

}

regex::regex(const String& s)
{
	assign(s);
}



regex& regex::operator=(const regex& o)
{
	pimpl=o.pimpl;
	return *this;
}

bool regex::assign(const String& s)
{
	RegexParser parser;
	regex_item_seq* q=parser.parse(s);
	pimpl.reset(q);
	return pimpl!=NULL;
}

bool regex_match(const String& s,regex& re)
{
	regex_base<const char*> impl;

	const char* q1=s.c_str();
	const char* q2=q1+s.length();

	ObjectData* pr=*(ObjectData**)&re;
	if(!pr) return false;
	return impl.match(static_cast<regex_item_seq*>(pr),q1,q2);
}

String cmatch::operator[](size_t i) const
{
	if(i>=matchs.size()) Exception::XInvalidIndex();
	if(matchs[i].size()==1)
	{
		return String(matchs[i][0].first,matchs[i][0].second);
	}
	else
	{
		return String(matchs[i][0].first,matchs[i][0].second);	
	}
}

bool regex_match(const String& s,cmatch& res,regex& re)
{
	regex_base<const char*> impl;

	res.orig_str=s;

	const char* q1=res.orig_str.c_str();
	const char* q2=q1+res.orig_str.length();

	ObjectData* pr=*(ObjectData**)&re;
	if(!pr) return false;

	bool flag=impl.match(static_cast<regex_item_seq*>(pr),q1,q2);

	if(!flag) return false;

	res.matchs.clear();

	class item
	{
	public:

		item(){}
		item(const char* p,int n):str(p,p),num(n){}

		std::pair<const char*,const char*> str;
		int num;
	};

	 arr_1t<item> q;

	for(size_t i=0;i<impl.stkSeqpos.size();i++)
	{
		int n=impl.stkSeqpos[i].num;
		if(n>=0)
		{
			q.push_back(item(impl.stkSeqpos[i].pos,impl.stkSeqpos[i].num));
		}
		else if(n<0)
		{
			q.back().str.second=impl.stkSeqpos[i].pos;
			if(q.back().num>=(int)res.matchs.size())
			{
				res.matchs.resize(q.back().num+1);
			}
			res.matchs[q.back().num].push_back(q.back().str);
			q.pop_back();
		}
	}

	return true;
}


EW_LEAVE


