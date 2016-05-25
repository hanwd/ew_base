
#include "ewa_base/testing/test.h"
#include "ewa_base/threading.h"
#include "ewa_base/collection/arr_xt.h"
#include "ewa_base/serialization.h"
#include "ewa_base/scripting.h"


using namespace ew;

template<typename T1,typename T2>
class serializable_data : public ObjectData
{
public:

	T1 val;
	T2 tmp;

	void Serialize(Serializer& ar)
	{
		ar & val & tmp;
	}

	DECLARE_OBJECT_INFO(serializable_data,ObjectInfo)
};

IMPLEMENT_OBJECT_INFO_T2(serializable_data,ObjectInfo)

class sample_data
{
public:
	bst_map<String,float64_t> aMaps;

	arr_1t<int32_t> aInts;
	String s1;
	DataPtrT<serializable_data<float,String> > dptr1;
	DataPtrT<ObjectData> dptr2;

	int32_t ivals[10];
	arr_xt<float64_t> arr;

	void Serialize(Serializer& ar)
	{
		ar & aInts;
		ar & aMaps;
		ar & s1 & ivals & arr;
		ar & dptr1 & dptr2;
	}

};


TEST_DEFINE(TEST_Serializer)
{

	StreamSerializer<SerializerReader> reader;
	StreamSerializer<SerializerWriter> writer;

	sample_data dat[2];

	dat[0].aInts.push_back(1);
	dat[0].aInts.push_back(5);
	dat[0].aMaps["hello"]=1.033;
	dat[0].aMaps["gg"]=1.034;
	dat[0].s1="sample_string";

	serializable_data<float,String>* p1=new serializable_data<float,String>;
	p1->val=1.324;
	p1->tmp="hello";

	dat[0].dptr1.reset(p1);
	dat[0].dptr2.reset(p1);

	dat[0].arr.resize(6,3,2);
	dat[0].arr(1,2,1)=3.2;
	dat[0].arr(0)=1.234;

	dat[0].ivals[3]=32;

	writer.open("test_serializer.dat");
	try
	{
		writer & Serializer::head & dat[0] & Serializer::tail;
	}
	catch(...)
	{
		TEST_ASSERT_MSG(false,"serializer write file failed!");
	}
	writer.close();

	reader.open("test_serializer.dat");
	try
	{
		reader & Serializer::head & dat[1] & Serializer::tail;
	}
	catch(...)
	{
		TEST_ASSERT_MSG(false,"serializer read file failed!");
	}
	reader.close();

	TEST_ASSERT(dat[1].aInts==dat[0].aInts);
	TEST_ASSERT(dat[1].aMaps==dat[0].aMaps);
	TEST_ASSERT(dat[1].s1==dat[0].s1);

	TEST_ASSERT(dat[1].dptr1==dat[1].dptr2);
	TEST_ASSERT(dat[1].dptr1!=NULL);
	TEST_ASSERT(dat[1].dptr1->val==dat[0].dptr1->val);

	TEST_ASSERT(dat[1].arr.size(0)==dat[0].arr.size(0));
	TEST_ASSERT(dat[1].arr.size(1)==dat[0].arr.size(1));
	TEST_ASSERT(dat[1].arr.size(2)==dat[0].arr.size(2));

	if(dat[1].arr.size(0)==dat[0].arr.size(0))
	{
		for(size_t i=0; i<dat[1].arr.size(0); i++)
		{
			TEST_ASSERT(dat[1].arr(i)==dat[0].arr(i));
		}
	}

	for(int i=0; i<10; i++)
	{
		TEST_ASSERT(dat[1].ivals[i]==dat[0].ivals[i]);
	}

	SerializerBuffer sbuf;

	double v1[4]= {1.234,234.0,323,432};
	double v2[4];

	try
	{
		sbuf.writer() & Serializer::head & v1 &Serializer::tail; // write data to buffer
		sbuf.reader() & Serializer::head & v2 &Serializer::tail; // read data from buffer
	}
	catch(...)
	{
		TEST_ASSERT_MSG(false,"SerializerBuffer failed!");
	}

	TEST_ASSERT(memcmp(v1,v2,sizeof(double)*4)==0);
	TEST_ASSERT(sbuf.skip()); // reader reach the end of buffer
}


TEST_DEFINE(TEST_SerializerSeek)
{
	typedef serializable_data<float,String> float_string;

	SerializerFile sf;

	if(sf.file.Open("seekable.dat",FileAccess::FLAG_WC))
	{

		SerializerWriter &writer(sf.writer());

		DataPtrT<float_string> p1(new float_string);
		DataPtrT<float_string> p2(new float_string);
		DataPtrT<CallableTableEx> p3(new CallableTableEx);

		p3->value["abc"].reset("hello");
		p3->value["cde"].reset("world");

		p1->val=1.0;
		p2->val=2.0;

		writer.flags.add(Serializer::FLAG_OFFSET_TABLE);
		writer & Serializer::head & p1 & p2 & p3 & Serializer::tail;

	}
	sf.file.Close();

	if(sf.file.Open("seekable.dat",FileAccess::FLAG_RD))
	{
		DataPtrT<float_string> p1;
		DataPtrT<float_string> p2;
		DataPtrT<CallableTableEx> p3;

		SerializerReader &reader(sf.reader());

		reader & Serializer::head;

		p3.reset(dynamic_cast<CallableTableEx*>(reader.read_object(3)));
		p1.reset(dynamic_cast<float_string*>(reader.read_object(1)));
		p2.reset(dynamic_cast<float_string*>(reader.read_object(2)));

		TEST_ASSERT(p3);

		if(p3)
		{
			String& s1=p3->value["abc"].ref<String>();
			String& s2=p3->value["cde"].ref<String>();
			TEST_ASSERT(s1=="hello");
			TEST_ASSERT(s2=="world");		
		}
		
		TEST_ASSERT(p1 && p1->val==1.0);
		TEST_ASSERT(p2 && p2->val==2.0);
	}

}
