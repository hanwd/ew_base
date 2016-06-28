#include "ewa_base/testing/test.h"
#include "ewa_base/util/regex.h"
#include "ewa_base/util/strlib.h"
#include "../../ewa_base/util/regex_impl.h"

#include <regex>
#include <iostream>
#include <ctime>

using namespace ew;




class EW_Regex
{
public:
	static bool regex_match(const char* s,const char* p)
	{
		Regex re(p);
		return re.match(s);
	}
};

class StdRegex
{
public:
	static bool regex_match(const char* s,const char* p)
	{
		std::regex re(p);
		return std::regex_match(s,re);
	}

};

template<typename P>
void regex_test(const String& type)
{	
	TimePoint p1=Clock::now();

	//for(int i=0;i<500;i++)
	{
		TEST_ASSERT(P::regex_match("a","a"));
		TEST_ASSERT(P::regex_match("abc","abc"));

		TEST_ASSERT(P::regex_match("","a*"));
		TEST_ASSERT(P::regex_match("a","a*"));
		TEST_ASSERT(P::regex_match("aaa","a*"));
		TEST_ASSERT(!P::regex_match("aaab","a*"));

		TEST_ASSERT(!P::regex_match("","a+"));
		TEST_ASSERT(P::regex_match("a","a+"));
		TEST_ASSERT(P::regex_match("aaa","a+"));

		TEST_ASSERT(P::regex_match("bc","a*bc"));
		TEST_ASSERT(P::regex_match("abc","a*bc"));
		TEST_ASSERT(P::regex_match("aaaabc","a*bc"));

		TEST_ASSERT(P::regex_match("bc","a?bc"));
		TEST_ASSERT(P::regex_match("abc","a?bc"));
		TEST_ASSERT(!P::regex_match("aabc","a?bc"));

		TEST_ASSERT(!P::regex_match("a","a{2,3}"));
		TEST_ASSERT(P::regex_match("aa","a{2,3}"));
		TEST_ASSERT(P::regex_match("aaa","a{2,3}"));
		TEST_ASSERT(!P::regex_match("aaaa","a{2,3}"));

		TEST_ASSERT(!P::regex_match("a","[b-f]"));
		TEST_ASSERT(P::regex_match("b","[b-f]"));
		TEST_ASSERT(P::regex_match("f","[b-f]"));
		TEST_ASSERT(!P::regex_match("g","[b-f]"));

		TEST_ASSERT(P::regex_match("g","[^b-f]"));
		TEST_ASSERT(!P::regex_match("b","[^b-f]"));

		TEST_ASSERT(P::regex_match("a","a|b"));
		TEST_ASSERT(P::regex_match("b","a|b"));
		TEST_ASSERT(!P::regex_match("c","a|b"));

		TEST_ASSERT(P::regex_match("abaaacaabbbaababcababaaacabbb","((b|(a|b)*)*c*(a|b)*)*"));
	
		TEST_ASSERT(P::regex_match("dcbaadadced","(a*b*c*d*e*)*"));
		TEST_ASSERT(P::regex_match("dcbaadadced","(a*|b*|c*|d*|e*)*"));
		TEST_ASSERT(P::regex_match("adsdfskdf","\\w+"));
		TEST_ASSERT(P::regex_match("adsd fskdf","\\w+\\s+\\w+"));
	}

	TimePoint p2=Clock::now();
	this_logger().LogMessage("%s : %g ms",type,(p2-p1)/TimeSpan::MilliSeconds(1));

}


TEST_DEFINE(TEST_Regex)
{
	regex_test<EW_Regex>("ew::Regex");
	//regex_test<StdRegex>("std::regex");
	//system("pause");
}


TEST_DEFINE(TEST_Regex2)
{

	Match res;
	Regex re;

	re.assign("a|(ab)");
	TEST_ASSERT(re.match("ab"));

	re.assign("((.*)$)*",Regex::FLAG_RE_MULTILINE);
	TEST_ASSERT(re.match("abc\r\ncde",res));
	TEST_ASSERT(re.match("abc sdfsdf\ncde",res));
	TEST_ASSERT(re.match("abc sdfsdf\ncde sdfsf\n",res));

	re.assign("abc",Regex::FLAG_RE_IGNORECASE);
	TEST_ASSERT(re.match("AbC"));

	re.assign("abc");
	TEST_ASSERT(!re.match("AbC"));
	
	re.assign("((\\w+)\\s*)+");

	String aaa="The quick brown fox jumps over the lazy dog";
	arr_1t<String> words=string_split(aaa," ");

	TEST_ASSERT(re.match(aaa,res));

	if(res.size()==3 && res[2].size()==9)
	{
		for(size_t i=0;i<9;i++)
		{
			TEST_ASSERT(res[2][i]==words[i]);
		}
	}

	re.assign("\\w+");
	if(re.search(aaa,res))
	{
		TEST_ASSERT(res[0]=="The");
		int nd=1;
		while(res.search_next())
		{
			TEST_ASSERT(res[0]==words[nd++]);
		}
	}
	re.assign("(\\w+) (\\w+)");	
	TEST_ASSERT(ew::regex_replace("hello world",re,"$2 $1")=="world hello");

	RegexEx re2;
	re2.prepare("id","[a-zA-z][a-zA-z0-9]*");
	re2.prepare("value","`double`|`int`");
	re2.prepare("sign","[\\+\\-]");
	re2.prepare("int","`sign`?0|([1-9][0-9]*)");
	re2.prepare("double","`int`?\\.[0-9]*([eEdD]`int`)?");
	re2.prepare("expr","`id`\\=`value`");

	re2.assign("`id`");
	TEST_ASSERT(re2.match("a"));
	TEST_ASSERT(re2.match("abc9e"));

	re2.assign("`sign`");
	TEST_ASSERT(re2.match("+"));
	TEST_ASSERT(re2.match("-"));

	re2.assign("`int`");
	TEST_ASSERT(re2.match("0"));
	TEST_ASSERT(re2.match("12"));
	TEST_ASSERT(re2.match("+134"));
	TEST_ASSERT(re2.match("-134"));

	re2.assign("`double`");
	TEST_ASSERT(re2.match(".123"));
	TEST_ASSERT(re2.match("0.3"));
	TEST_ASSERT(re2.match("1.3e12"));
	TEST_ASSERT(re2.match("-1.34e-3"));

	re2.assign("`value`");
	TEST_ASSERT(re2.match(".123"));
	TEST_ASSERT(re2.match("0.3"));
	TEST_ASSERT(re2.match("1.3e12"));
	TEST_ASSERT(re2.match("-1.34e-3"));
	TEST_ASSERT(re2.match("0"));
	TEST_ASSERT(re2.match("12"));
	TEST_ASSERT(re2.match("+134"));
	TEST_ASSERT(re2.match("-134"));

	re2.assign("`expr`");
	TEST_ASSERT(re2.match("a=0"));
	TEST_ASSERT(re2.match("a=11"));
	TEST_ASSERT(re2.match("a=+11"));
	TEST_ASSERT(re2.match("a=-11"));
	TEST_ASSERT(!re2.match("a=01"));

	TEST_ASSERT(re2.match("abc=-1.32e-3"));
	TEST_ASSERT(re2.match("A=.32E-3"));
	TEST_ASSERT(!re2.match("9a=-1.32e-3"));	
}


TEST_DEFINE(TEST_Regex3)
{

	RegexEx re2;

	re2.prepare("id","[a-zA-z][a-zA-z0-9]*");
	re2.prepare("value","`double`|`int`");
	re2.prepare("sign","[\\+\\-]");
	re2.prepare("int","`sign`?0|([1-9][0-9]*)");
	re2.prepare("double","`int`?\\.[0-9]*([eEdD]`int`)?");
	re2.prepare("explist","`expr`(\\,`expr`)*");
	re2.prepare("item","`value`|`id`|(\\(`expr`\\))");
	re2.prepare("item0","`item`(`dot`|`fun`)*");
	re2.prepare("dot","\\.`id`");
	re2.prepare("fun","\\(`explist`??\\)");

	re2.prepare("op2","[\\+\\-]");
	re2.prepare("op1","[\\*\\/]");
	re2.prepare("item1","`item0`(`op1``item0`)*");
	re2.prepare("item2","`item1`(`op2``item1`)*");
	re2.prepare("expr","`item2`");

	re2.assign("`expr`");

	TEST_ASSERT(re2.match("a"));
	TEST_ASSERT(re2.match("-1.3e-9"));
	TEST_ASSERT(re2.match("-1.3e-9*3+9"));
	TEST_ASSERT(re2.match("sin(pi*0.3)"));
	TEST_ASSERT(re2.match("a.x"));
	TEST_ASSERT(re2.match("b(3,4)"));
	TEST_ASSERT(re2.match("b()"));

	TEST_ASSERT(re2.match("sqrt(a.x*a.x,a.y*a.y)"));
	TEST_ASSERT(re2.match("sqrt(a.x*a.x,a.y*a.y)+3*4+5*6/32"));
	

}

