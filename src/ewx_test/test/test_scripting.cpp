
#include "ewa_base.h"

using namespace ew;

TEST_DEFINE(TEST_Scripting_variant)
{

	Variant a,b,c,d,e;
	a.reset(10.0);
	double v;
	TEST_ASSERT(a.get<double>(v) && v==10.0);
	TEST_ASSERT(a.set<double>(20.0));
	TEST_ASSERT(a.get<double>(v) && v==20.0);

	double* p=a.ptr<double>();
	TEST_ASSERT(p!=NULL && *p==20.0);
	TEST_ASSERT(a.ref<double>()==20.0);

	b.reset(20.0);
	c.reset(11);

	TEST_ASSERT(a==b);
	TEST_ASSERT(a!=c);

	c=a;
	TEST_ASSERT(a==c);

	d.reset("10");
	e.reset(15);


	TEST_ASSERT(variant_cast<int>(a)==20);
	TEST_ASSERT(variant_cast<int>(d)==10);	// cast string "10" to int
	TEST_ASSERT(variant_cast<String>(e)==String("15"));	// cast int64_t 15 to string "15"


}


TEST_DEFINE(TEST_Scripting_Executor1)
{

	Executor ewsl;

	ewsl.push(1);
	ewsl.push(2);
	ewsl.call2<pl_add>();
	TEST_ASSERT(variant_cast<int>(ewsl.top())==3);
	ewsl.popq();

	ewsl.push(1.23);
	ewsl.get_system("math");
	ewsl.get_index("sin");
	ewsl.callx(1);			// call math.sin(1.23)
	ewsl.set_global("ans");	// save result to table["ans"]

	TEST_ASSERT(variant_cast<double>(ewsl.tb1["ans"])==::sin(1.23));

// string cat operator..
	ewsl.execute("local a='hello';local b='world';global c=a..b;print(ans);");
	TEST_ASSERT(variant_cast<String>(ewsl.tb1["c"])==String("helloworld"));


#define EVAL_ASSERT(T,X) TEST_ASSERT(ewsl.eval<T>(#X)==(X));

// add sub mul div mod
	EVAL_ASSERT(int,2+3);
	EVAL_ASSERT(int,2-3);
	EVAL_ASSERT(int,2*3);
	EVAL_ASSERT(int,4/2);
	EVAL_ASSERT(int,6%4);

// bitwise
	EVAL_ASSERT(int,6|2);
	EVAL_ASSERT(int,6&2);
	EVAL_ASSERT(int,~6);

// relation
	EVAL_ASSERT(bool,6!=2);
	EVAL_ASSERT(bool,6==2);
	EVAL_ASSERT(bool,6>2);
	EVAL_ASSERT(bool,6>=2);
	EVAL_ASSERT(bool,6<2);
	EVAL_ASSERT(bool,6<=2);

// logical
	EVAL_ASSERT(bool,true&&true);
	EVAL_ASSERT(bool,true&&false);
	EVAL_ASSERT(bool,false&&true);
	EVAL_ASSERT(bool,false&&false);

	EVAL_ASSERT(bool,true||true);
	EVAL_ASSERT(bool,true||false);
	EVAL_ASSERT(bool,false||true);
	EVAL_ASSERT(bool,false||false);


	EVAL_ASSERT(bool,!false);
	EVAL_ASSERT(bool,!true);

	TEST_ASSERT(ewsl.eval<double>("math.sqrt(2.3)")==::sqrt(2.3));
	TEST_ASSERT(ewsl.eval<double>("math.log(2.3)")==::log(2.3));
	TEST_ASSERT(ewsl.eval<double>("math.log10(2.3)")==::log10(2.3));
	TEST_ASSERT(ewsl.eval<double>("math.abs(-2.3)")==::fabs(-2.3));

// complex
	TEST_ASSERT(ewsl.eval<std::complex<double> >("complex(1.2,2.3)")==std::complex<double>(1.2,2.3));
	TEST_ASSERT(ewsl.eval<std::complex<double> >("1.2+2.3i")==std::complex<double>(1.2,2.3));

	TEST_ASSERT(ewsl.eval<double>("math.real(complex(1.2,2.3))")==1.2);
	TEST_ASSERT(ewsl.eval<double>("math.imag(complex(1.2,2.3))")==2.3);
	TEST_ASSERT(ewsl.eval<std::complex<double> >("math.conj(complex(1.2,2.3))")==std::complex<double>(1.2,-2.3));

	TEST_ASSERT(ewsl.eval<double>("math.abs(complex(3,4))")==std::abs(std::complex<double>(3,4)));
	TEST_ASSERT(ewsl.eval<std::complex<double> >("math.exp(complex(3,4))")==std::exp(std::complex<double>(3,4)));


	TEST_ASSERT(ewsl.eval<std::complex<double> >("math.sqrt(complex(3,4))")==std::sqrt(std::complex<double>(3,4)));
	TEST_ASSERT(ewsl.eval<std::complex<double> >("math.pow(complex(3,4),2)")==std::pow(std::complex<double>(3,4),2));
	TEST_ASSERT(ewsl.eval<std::complex<double> >("math.sin(complex(3,4))")==std::sin(std::complex<double>(3,4)));
	TEST_ASSERT(ewsl.eval<std::complex<double> >("math.cos(complex(3,4))")==std::cos(std::complex<double>(3,4)));
	TEST_ASSERT(ewsl.eval<std::complex<double> >("math.tan(complex(3,4))")==std::tan(std::complex<double>(3,4)));

// casting

	TEST_ASSERT(ewsl.eval<String>("123")==String("123"));
	TEST_ASSERT(ewsl.eval<String>("1.25")==String("1.25"));
	TEST_ASSERT(ewsl.eval<double>("'1.25'")==1.25);
	TEST_ASSERT(ewsl.eval<int64_t>("'123'")==123);
}


TEST_DEFINE(TEST_Scripting_Executor2)
{

	Executor ewsl;


	ewsl.execute("try{assert(false,1,2,3);}catch(...){println('catch:',...);};");
	
	ewsl.execute("println('time:',os.time());");

	ewsl.execute("local z1=array_double.ones(2,2);local z2=z1;printex(z1);printex(z2);");

	ewsl.execute(
"println("
"	typeid(nil),"
"	typeid(true),"
"	typeid(1),"
"	typeid(1.0),"
"	typeid(1i),"
"	typeid('string'),"
"	typeid(table()),"
"	typeid(array_integer.ones(1)),"
"	typeid(array_double.ones(1)),"
"	typeid(array_complex.ones(1)),"
"	typeid([1,2,3]),"
"	typeid(function(){}));"
	);

	ewsl.execute(
"local s='Hello';"
"println('string:',s);"
"println('lower:',s.lower());"
"println('upper:',s.upper());"
"println('reverse:',s.reverse());"
"println('substr(2,-1):',s.substr(2,-1));"
"println(string.format('{1},{2}','hello','world'));"
);


	ewsl.execute(
"logger.log_message('{1} from script!','logger.log_message');"
"logger.log_warning('{1} from script!','logger.log_warning');"
"logger.log_error('{1} from script!','logger.log_error');"
	);

	ewsl.execute("local a=[1,2,3,4];printex(a);");

	ewsl.execute("local a=table();local b=clone(a);println(a===b,a==b);");

	ewsl.execute("local a,b=1,2;global c=a+b;");
	TEST_ASSERT(variant_cast<int>(ewsl.tb1["c"])==3);


	ewsl.execute("global x=1;global y=2;x,y=y,x;");
	TEST_ASSERT(variant_cast<int>(ewsl.tb1["x"])==2);
	TEST_ASSERT(variant_cast<int>(ewsl.tb1["y"])==1);

	ewsl.execute("global s=0;if(1+2==3) s=1;else s=-1;");
	TEST_ASSERT(variant_cast<int>(ewsl.tb1["s"])==1);

	ewsl.execute("global s=0;for(local i=0;i<10;i++) {if(i==2) continue; if(i==5) break; s+=i;};");
	TEST_ASSERT(variant_cast<int>(ewsl.tb1["s"])==1+3+4);

	ewsl.execute("global s=0;while(s%2==0&&s<100) {s=s+2;}");
	TEST_ASSERT(variant_cast<int>(ewsl.tb1["s"])==100);

	ewsl.execute("global s=0;do {s=s+2;} while(false);");
	TEST_ASSERT(variant_cast<int>(ewsl.tb1["s"])==2);

	ewsl.execute("global s=0;for(local i=0;i<100;i++) for(local j=0;j<100;j++) {s=s+i+j;if(j==2) break2;}");
	TEST_ASSERT(variant_cast<int>(ewsl.tb1["s"])==1+2);

// array
	ewsl.execute("global s=linspace(0,10,11)*2;");
	arr_xt<double>& s(ewsl.tb1["s"].ref<arr_xt<double> >());
	TEST_ASSERT(s.size()==11);
	for(size_t i=0;i<s.size();i++)
	{
		TEST_ASSERT(s(i)==(double)i*2);
	}

// function
	ewsl.execute("function fn(x,y){return x+y,x-y;}; global a,b=fn(1,2);");
	TEST_ASSERT(variant_cast<int>(ewsl.tb1["a"])==1+2);
	TEST_ASSERT(variant_cast<int>(ewsl.tb1["b"])==1-2);

	ewsl.execute("function fn(x,y){return x+y,x-y;}; global a,b=fn(1,2),8;");
	TEST_ASSERT(variant_cast<int>(ewsl.tb1["a"])==1+2);
	TEST_ASSERT(variant_cast<int>(ewsl.tb1["b"])==8);

	ewsl.execute("local z=0;function fn(){z=z+1;return z;}; global a=fn();global b=fn();");
	TEST_ASSERT(variant_cast<int>(ewsl.tb1["a"])==1);
	TEST_ASSERT(variant_cast<int>(ewsl.tb1["b"])==2);


	ewsl.execute("function f(n){if(n<2) return 1;return n*self(n-1);};global c=f(1);global d=f(4);");
	TEST_ASSERT(variant_cast<int>(ewsl.tb1["c"])==1);
	TEST_ASSERT(variant_cast<int>(ewsl.tb1["d"])==24);

// printing from 0 to 16
	ewsl.execute("function fn(i,j){if(i==j) return j;else return i,self(i+1,j);};println('print 0~10:',fn(0,10))");


// class
	ewsl.execute(
"class Rect"
"{"
"	self.x=0;"
"	self.y=0;"
"	function meta.area()"
"	{"
"		return this.x*this.y;"
"	};"
"};"
"global a=Rect();a.x=10;a.y=20;"
"global s=a.area();"
);
	TEST_ASSERT(variant_cast<int>(ewsl.tb1["s"])==10*20);

// for_each
	ewsl.execute("local s=linspace(0,10,11);for_each(v in s) println(v);");
	ewsl.execute("local s=table();s.a=1;s.b=2;for_each(k,v in s) {println(k,'=',v);}");
	ewsl.execute("local s=[0,'hello',3.5];for_each(v in s) println(v);");

// switch
	ewsl.execute(
"for_each(s in [0,2,4,6,17,'abc']){print(s,'in '); "
"switch(s){"
"	case 0,1,2: println('0,1,2');"
"	case 3,6,9: println('3,6,9');"
"	case 4,'abc': println('4,abc');"
"	default: println('other');"
"}}");

//coroutine
	ewsl.execute(
"#import coroutine;\n"
"function f(...){println('co',...);println('co',coroutine.yield(4,5,6));return 10;};"
"global co=coroutine.create(f);"
"println('stat',coroutine.status(co));"
"println('main',coroutine.resume(co,1,2,3));"
"println('stat',coroutine.status(co));"
"println('main',coroutine.resume(co,7,8,9));"
"println('stat',coroutine.status(co));"
"println('main',coroutine.resume(co,11));"
		);


// exception
	ewsl.execute(
"function fn_throw(){...=1,2,3;println('throw',...);throw ...;};"
"try"
"{"
"	println('before throw');"
"	fn_throw();"
"	println('after throw(this should not be print)');"
"}"
"catch(...)"
"{"
"	println('catch',...);"
"}"
"finally"
"{"
"	println('in finally');"
"}"
);

// simulate Lua pcall like protected call
	ewsl.execute(
"function pcall(fn,...){"
"	try{"
"		return true,fn(...);"
"	}"
"	catch(...){"
"		return false,...;"
"	}"
"};"
"println('pcall(math.add,1,2))',pcall(math.add,1,2));"
"println('pcall(math.add,1,nil))',pcall(math.add,1,nil));"
);

}



TEST_DEFINE(TEST_Scripting_Executor3)
{
	Executor ewsl;

	ewsl.execute("local a,b,...=1,2,3,4;println(...);");

	ewsl.execute("local a=table();local b=a;local c=clone(a);println(a,b,c);");

	ewsl.execute(
"println(boolean.TRUE,boolean.FALSE);"
"println(integer.MIN_VALUE,integer.MAX_VALUE);"
"println(double.MIN_VALUE,double.MAX_VALUE);"
"println(complex.i_unit);"
"println(integer('123.45'));"
"println(1+2i);"
		);

	ewsl.execute(
"global a,b,c,z,n1,n2;"
"z='hello';"
"a=z[:-1:];"	// reverse string
"b=z[:end-1];"	// remove last character
"c=z[[1,3,0]];"
"n1=z.length();"
"z[1:2]='ab';"
		);

	TEST_ASSERT(variant_cast<String>(ewsl.tb1["a"])=="olleh");
	TEST_ASSERT(variant_cast<String>(ewsl.tb1["b"])=="hell");
	TEST_ASSERT(variant_cast<String>(ewsl.tb1["c"])=="elh");
	TEST_ASSERT(variant_cast<String>(ewsl.tb1["z"])=="hablo");
	TEST_ASSERT(variant_cast<int>(ewsl.tb1["n1"])==5);


	ewsl.execute(
"printex((0:10).trans());"						// 0 to 10
"printex((0:10)[end:begin].trans());"			// 10 to 0
"printex((0:10)[end-1:-1:begin+1].trans());"	// 9  to 1
"printex(diag([1,2,3,4]));"						// matrix 4x4
"printex(diag([1,2,3,4])[:,:end-1]);"			// sub matrix
"printex(array_double.ones(4,4)*1.5);"
"printex(array_integer.zeros(4,4)+1);"
"printex([1,'hello',3.5,complex(1,2)].trans());"
"printex(function(){local a=table();a.x=1;a.y=2;return a;}());"
		);

	ewsl.execute(
"function reverse_println(...)"
"{"
"	println(unpack([...][:-1:]));"
"};"
"reverse_println(1,2,3);" // 3 2 1
"...=1,2,3,4;"
"reverse_println(...);"	// 4 3 2 1
"local a,b,...=1,2,3,4;"
"reverse_println(...);" // 4 3
		);


}

