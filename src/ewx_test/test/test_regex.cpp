#include "ewa_base/testing/test.h"
#include "ewa_base/util/regex.h"
#include <regex>
#include <iostream>

using namespace ew;


class StdRegex
{
public:
	static bool regex_match(const char* s,const char* p)
	{
		std::regex re(p);
		return std::regex_match(s,re);
	}

};

class MyRegex
{
public:
	static bool regex_match(const char* s,const char* p)
	{
		regex re(p);
		return ew::regex_match(s,re);
	}
};


template<typename P>
void regex_test(const String& type)
{	
	TimePoint p1=Clock::now();

	for(int i=0;i<300;i++)
	{
		TEST_ASSERT(P::regex_match("a","a"));
		TEST_ASSERT(P::regex_match("abc","abc"));

		TEST_ASSERT(P::regex_match("","a*"));
		TEST_ASSERT(P::regex_match("a","a*"));
		TEST_ASSERT(P::regex_match("aaa","a*"));

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
		TEST_ASSERT(P::regex_match("adsdfskdf","\\w+"));
		TEST_ASSERT(P::regex_match("adsd fskdf","\\w+\\s+\\w+"));

		TEST_ASSERT(P::regex_match("adsd fskdf sdfsdkjdsfk sdfskfs df sdfsjdf sfdjsdf sjfsfj sf sdjf sdfj sdf s","(\\w+\\s*)+"));

	}


	TimePoint p2=Clock::now();
	this_logger().LogMessage("%s : %g ms",type,(p2-p1)/TimeSpan::MilliSeconds(1));

}


TEST_DEFINE(TEST_Regex)
{
	ew::regex re;
	ew::cmatch res;

	//re.assign("\\w+?$");
	//ew::regex_match("abcd",res,re);
	//ew::regex_match("ab,cd",res,re);
	//re.assign("(\\w+\\,|$)+");
	//ew::regex_match("ab,cd",res,re);
	//re.assign("[d-f]+");
	//ew::regex_search("abc def ggg dfe",res,re);

	re.assign("(\\w+\\s*)+");

	bool flag=ew::regex_match("adsd fskdf sdfsdkjdsfk sdfskfs df sdfsjdf sfdjsdf sjfsfj sf sdjf sdfj sdf s",res,re);


	regex_test<StdRegex>("std_regex");
	regex_test<MyRegex>("my_regex");

}


