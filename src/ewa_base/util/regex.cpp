#include "ewa_base/util/regex.h"
#include "regex_parser.h"
#include "regex_impl.h"

EW_ENTER

Regex::Regex()
{

}

Regex::Regex(const Regex& r):pimpl(r.pimpl)
{

}

Regex::Regex(const String& s,int f)
{
	assign(s,f);
}

Regex& Regex::operator=(const Regex& o)
{
	pimpl=o.pimpl;
	return *this;
}

bool Regex::assign(const String& s,int f)
{
	RegexParser parser(f);
	regex_item_seq* q=parser.parse(s);
	pimpl.reset(q);
	return pimpl!=NULL;
}

bool Regex::match(const String& s)
{
	regex_impl<const char*> impl;

	const char* q1=s.c_str();
	const char* q2=q1+s.length();

	if(!pimpl) return false;

	return impl.match(static_cast<regex_item_seq*>(pimpl.get()),q1,q2);
}

regex_item_seq* do_get_regex_item_seq(Regex& re)
{
	ObjectData* pitem=*(ObjectData**)&re;
	if(!pitem) return NULL;
	return static_cast<regex_item_seq*>(pitem);
}

bool regex_execute(const String& s,Match& res,Regex& re,bool fg)
{
	regex_impl<const char*> impl;

	res.matchs.clear();
	res.orig_str=s;
	res.orig_reg=re;

	const char* q1=res.orig_str.c_str();
	const char* q2=q1+res.orig_str.length();

	regex_item_seq* seq=do_get_regex_item_seq(re);

	bool flag=fg?impl.match(seq,q1,q2):impl.search(seq,q1,q2);

	if(!flag) return false;

	impl.update_match_results(res);

	return true;
}



bool Regex::match(const String& s,Match& m)
{
	return regex_execute(s,m,*this,true);
}

bool Regex::search(const String& s,Match& m)
{
	return regex_execute(s,m,*this,false);
}



bool regex_match(const String& s,Regex& re)
{
	return re.match(s);
}

bool regex_match(const String& s,Match& res,Regex& re)
{
	return regex_execute(s,res,re,true);
}

bool regex_search(const String& s,Match& res,Regex& re)
{
	return regex_execute(s,res,re,false);
}

String regex_replace(const String& s,Regex& re,const String& p)
{
	Match res;
	re.match(s,res);
	return res.replace(p);
}



bool Match::search_next()
{
	if(matchs.empty())
	{
		return false;
	}

	const char* q1=orig_str.c_str();
	const char* q2=q1+orig_str.length();

	const char* p1=matchs[0][0].it_end;

	matchs.clear();

	if(p1<q1||p1>=q2)
	{
		return false;
	}

	regex_impl<const char*> impl;
	regex_item_seq* seq=do_get_regex_item_seq(orig_reg);

	bool flag=impl.search(seq,p1,q2);

	if(!flag) return false;

	impl.update_match_results(*this);

	return true;

}

String Match::replace(const String& dst) const
{
	StringBuffer<char> sb;
	const char* p1=dst.c_str();
	while(*p1)
	{
		if(*p1!='$' )
		{
			sb.push_back(*p1++);
			continue;
		}
		if(p1[1]>='0'&&p1[1]<='9')
		{
			int n=p1[1]-'0';
			if(n<(int)matchs.size())
			{
				sb<<matchs[n];
			}
			p1+=2;
		}
		else
		{
			sb.push_back(p1[1]);	
			p1+=2;
		}
	}
	return sb;
}

Match::item_array::operator String() const 
{
	if(empty()) return "";
	const_iterator it=cbegin();
	String result(*it++);
	while(it!=cend()) result+="\t"+*it++;
	return result;
}

EW_LEAVE


