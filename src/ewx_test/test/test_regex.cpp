#include "ewa_base/testing/test.h"
#include "ewa_base/util/regex.h"
#include "ewa_base/util/strlib.h"
#include "../../ewa_base/util/regex_impl.h"

#include <regex>
#include <iostream>
#include <ctime>

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
		Regex re(p);
		return re.match(s);
	}
};


template<typename P>
void regex_test(const String& type)
{	
	TimePoint p1=Clock::now();

	for(int i=0;i<500;i++)
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


	}


	TimePoint p2=Clock::now();
	this_logger().LogMessage("%s : %g ms",type,(p2-p1)/TimeSpan::MilliSeconds(1));

}



void test_ews()
{

	bool flag;
	Match res;
	Regex re;

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
			Console::WriteLine(res[2][i]);
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
}

void test_std()
{
	::clock_t t1=::clock();
	bool flag=true;
	try
	{
		std::regex re("((\\w+)\\s*)+");
		flag=std::regex_match("The quick brown fox jumps over the lazy dog",re);
	}
	catch(std::exception& e)
	{
		::printf("%s\n",e.what());
	}
	::clock_t t2=::clock();
	::printf("%g s\n",0.001*double(t2-t1));
	system("pause");
}

TEST_DEFINE(TEST_Regex)
{
	//test_std();
	//test_ews();

	//regex_impl<regex_policy_char_recursive> impl;
	//RegexParser parser(0);

	//impl.item_map["id"]=parser.parse("a");
	//impl.item_map["equal"]=parser.parse("\\=");
	//impl.item_map["expr"]=parser.parse("`id``equal``id`");
	//impl.item_map["stmt"]=parser.parse("`id`*\\;");
	//impl.item_map["stmt_seq"]=parser.parse("`stmt`*");
	//impl.item_map["block"]=parser.parse("\\{`stmt_seq`\\}");

	//String sss=";;";
	//const char* p1=sss.c_str();
	//const char* p2=p1+sss.length();

	//Regex re;
	//re.assign("(a*\\;)*");
	//bool flag1=re.match(sss);

	//::printf("----------\n");

	//bool flag2=impl.match(static_cast<regex_item_root*>(impl.item_map["stmt_seq"]),p1,p2);



	regex_test<MyRegex>("ew::Regex");
	regex_test<StdRegex>("std::regex");
	system("pause");
}


