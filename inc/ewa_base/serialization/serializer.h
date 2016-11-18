
#ifndef __H_EW_SERIALIZER__
#define __H_EW_SERIALIZER__


#include "ewa_base/basic/object_ex.h"
#include "ewa_base/basic/bitflags.h"
#include "ewa_base/basic/exception.h"
#include "ewa_base/collection/indexer_set.h"
#include "ewa_base/collection/indexer_map.h"
#include "ewa_base/collection/arr_1t.h"
#include "ewa_base/collection/arr_xt.h"
#include "ewa_base/collection/bst_set.h"
#include "ewa_base/collection/bst_map.h"

#pragma push_macro("new")
#undef new
#include <complex>
#include <vector>
#include <map>
#include <set>
#pragma pop_macro("new")


namespace tl
{

	template<typename T>
	struct is_pod<std::complex<T> > : public is_pod<T> {};

	template<typename X,typename Y>
	struct is_pod<std::pair<X,Y> > : public value_type<is_pod<X>::value&&is_pod<Y>::value> {};

};

EW_ENTER

DEFINE_OBJECT_NAME(std::complex<float32_t>,"c32");
DEFINE_OBJECT_NAME(std::complex<float64_t>,"c64");

typedef uint32_t intarr_t;




template<typename T>
class serial_type
{
public:

	static const int value=tl::is_convertible<T,ObjectData>::value?Serializer::PTRTAG_CACHED:
		tl::is_convertible<T,Object>::value?Serializer::PTRTAG_OBJ:
		tl::is_pod<T>::value?Serializer::PTRTAG_POD:Serializer::PTRTAG_CLS;
};

template<typename T> Serializer& operator &(Serializer& ar,T& val);
template<typename T> SerializerReader& operator &(SerializerReader& ar,T& val);
template<typename T> SerializerWriter& operator &(SerializerWriter& ar,T& val);

template<typename T> SerializerReader& operator >>(Serializer& ar,T& val);
template<typename T> SerializerWriter& operator <<(Serializer& ar,T& val);
template<typename T> SerializerReader& operator >>(SerializerReader& ar,T& val);
template<typename T> SerializerWriter& operator <<(SerializerWriter& ar,T& val);


template<typename A,typename T>
class serial_helper_test;

template<typename A,typename T>
class serial_helper_func;

template<typename A,typename T,int N>
class serial_switch;

template<typename A,typename T>
class serial_pod;


template<typename A,typename T>
class serial_pod
{
public:
	static const int value=tl::is_pod<T>::value;
	static void g(SerializerReader& ar,T& val)
	{
		ar.recv_checked((char*)&val,sizeof(T));
	}
	static void g(SerializerWriter& ar,T& val)
	{
		ar.send_checked((char*)&val,sizeof(T));
	}
};


template<typename A,typename I,typename T>
class serial_pod_cast : public serial_pod<A,T>
{
public:
	static const int value=0;
	typedef I type;
	static void g(SerializerReader& ar,type& val_)
	{
		T val;
		serial_pod<A,T>::g(ar,val);
		val_=val;
	}
	static void g(SerializerWriter& ar,type& val_)
	{
		T val(val_);
		serial_pod<A,T>::g(ar,val);
	}
};

template<typename A,typename T>
class serial_pod_cast<A,bool,T> : public serial_pod<A,T>
{
public:
	static const int value=0;
	typedef bool type;
	static void g(SerializerReader& ar,type& val_)
	{
		T val;
		serial_pod<A,T>::g(ar,val);
		val_=val!=0;
	}
	static void g(SerializerWriter& ar,type& val_)
	{
		T val(val_?1:0);
		serial_pod<A,T>::g(ar,val);
	}
};

template<typename A> class serial_pod<A,bool> : public serial_pod_cast<A,bool,char> {};
//template<typename A> class serial_pod<A,long> : public serial_pod_cast<A,long,int64_t> {};
//template<typename A> class serial_pod<A,unsigned long> : public serial_pod_cast<A,unsigned long,int64_t> {};
template<typename A> class serial_pod<A,wchar_t> : public serial_pod_cast<A,wchar_t,int32_t> {};



template<typename A,typename T,bool>
class serial_arr_handler_impl
{
public:
	static void g(A& ar,T* ptr,intarr_t s)
	{
		for(intarr_t i=0; i<s; i++)
		{
			serial_helper_test<A,T>::g(ar,ptr[i]);
		}
	}
};


template<typename A,typename T>
class serial_arr_handler_impl<A,T,true>
{
public:
	static void g(SerializerReader& ar,T* ptr,intarr_t s)
	{
		ar.recv_checked((char*)ptr,sizeof(T)*s);
	}

	static void g(SerializerWriter& ar,T* ptr,intarr_t s)
	{
		ar.send_checked((char*)ptr,sizeof(T)*s);
	}
};

template<typename A,typename T>
class serial_arr_handler : public serial_arr_handler_impl<A,T,serial_pod<A,T>::value> {};


template<typename A,typename T,typename V>
class serial_arr
{
public:
	typedef V type;

	static void g(SerializerReader& ar,type& val)
	{
		int n=ar.size_count(val.size());

		val.resize(n);
		if(n==0) return;
		serial_arr_handler<A,T>::g(ar,&val[0],n);
	}

	static void g(SerializerWriter& ar,type& val)
	{
		int n=ar.size_count(val.size());

		if(n==0) return;
		serial_arr_handler<A,T>::g(ar,&val[0],n);
	}
};


template<typename A,typename V>
class serial_arr<A,bool,V>
{
public:
	typedef bool T;
	typedef V type;

	static void g(SerializerReader& ar,type& val)
	{
		int n=ar.size_count(val.size());
		val.resize(n);
		if(n==0) return;
		for(intarr_t i=0; i<n; i++)
		{
			bool c1;
			serial_pod<A,bool>::g(ar,c1);
			val[i]=c1;
		}
	}
	static void g(SerializerWriter& ar,type& val)
	{
		int n=ar.size_count(val.size());
		if (n == 0) return;
		for (int i = 0; i < n; i++)
		{
			bool c1(val[i]);
			serial_pod<A, bool>::g(ar, c1);
		}
	}

};

template<typename A, typename T, typename V>
class serial_set
{
public:
	typedef V type;
	typedef typename type::iterator iterator;
	static void g(SerializerReader& ar, type& val)
	{
		int n=ar.size_count(val.size());

		T v;
		for (int i = 0; i < n; i++)
		{
			ar & v;
			val.insert(v);
		}
	}

	static void g(SerializerWriter& ar, type& val)
	{
		//int n=ar.size_count(val.size());
		for (iterator it = val.begin(); it != val.end(); it++)
		{
			ar << (*it);
		}
	}

};


template<typename A, typename T, int N>
class serial_ptr
{
public:
	typedef serial_switch<A, T, N> basetype;
	typedef T* pointer;

	static void g(SerializerReader& ar, pointer& val)
	{
		int32_t sval(0);
		serial_pod<A, int32_t>::g(ar, sval);
		if (sval == A::PTRTAG_NIL)
		{
			delete val;
			val = NULL;
			return;
		}

		if (sval != N)
		{
			ar.errstr("INVALID_POINTER_TYPE");
			return;
		}

		pointer tmp = new T();
		try
		{
			serial_helper_test<A, T>::g(ar, *tmp);
			delete val;
			val = tmp;
		}
		catch (...)
		{
			delete tmp;
			throw;
		}
	}

	static void g(SerializerWriter& ar, pointer& val)
	{
		int32_t sval = val == NULL ? A::PTRTAG_NIL : N;
		serial_pod<A, int32_t>::g(ar, sval);
		if (sval == A::PTRTAG_NIL) return;
		serial_helper_test<A, T>::g(ar, *val);
	}
};


template<typename A, typename T>
class serial_ptr<A, T, Serializer::PTRTAG_OBJ> : public serial_switch<A, T, Serializer::PTRTAG_OBJ>
{
public:
	typedef serial_switch<A, T, Serializer::PTRTAG_OBJ> basetype;
	typedef T* pointer;

	static void d(Object* p)
	{
		if (p) return;
		if (p != p->GetObjectInfo().GetCachedInstance())
		{
			delete p;
		}
	}

	static void g(SerializerReader& ar,pointer& val)
	{
		int32_t sval=A::PTRTAG_NIL;
		serial_pod<A,int32_t>::g(ar,sval);

		if(sval==A::PTRTAG_NIL)
		{
			d(val);
			val=NULL;
			return;
		}

		if(sval>=A::PTRTAG_CACHED)
		{
			ObjectData* dptr=ar.cached_objects().load_ptr(ar,sval);
			val=dynamic_cast<pointer>((Object*)dptr);
			if(!val)
			{
				ar.errstr("INVALID_CACHED_INDEX");
			}
			return;
		}
		if(sval!=A::PTRTAG_OBJ)
		{
			ar.errstr("INVALID_POINTER_TYPE");
			return;
		}

		Object* obj=ar.cached_objects().create(ar.object_type(""));
		if(!obj)
		{
			ar.errstr("CANNOT_CREATE_OBJECT");
			return;
		}

		pointer tmp=dynamic_cast<pointer>(obj);
		if(!tmp)
		{
			d(obj);
			ar.errstr("INCOMPATIBLE_OBJECT_TYPE");
			return;
		}

		try
		{
			basetype::g(ar,*tmp);
			delete val;
			val=tmp;
		}
		catch(...)
		{
			d(tmp);
			throw;
		}

	}

	static void g(SerializerWriter& ar,pointer& val)
	{
		int32_t sval=val==NULL? Serializer::PTRTAG_NIL : Serializer::PTRTAG_OBJ;

		if(sval==Serializer::PTRTAG_NIL)
		{
			serial_pod<A,int32_t>::g(ar,sval);
			return;
		}

		ObjectData* dptr(dynamic_cast<ObjectData*>(val));
		if(dptr)
		{
			ar.cached_objects().save_ptr(ar,dptr,true);
		}
		else
		{
			serial_pod<A,int32_t>::g(ar,sval);
			ar.object_type(val->GetObjectName());
			basetype::g(ar,*val);
		}
	}
};


template<typename A, typename T>
class serial_ptr<A, T, Serializer::PTRTAG_CACHED> : public serial_ptr<A, T, Serializer::PTRTAG_OBJ>
{
public:

};



template<typename A,typename T,typename V>
class serial_ptr_holder : public serial_ptr<A,T,serial_type<T>::value>
{
public:
	typedef V type;
	typedef serial_ptr<A,T,serial_type<T>::value> basetype;

	static void g(SerializerReader& ar,type& val)
	{
		T* p(NULL);
		try
		{
			basetype::g(ar,p);
		}
		catch(...)
		{
			delete p;
			throw;
		}
		val.reset(p);
	}
	static void g(SerializerWriter& ar,type& val)
	{
		T* p(val.get());
		basetype::g(ar,p);
	}
};


template<typename A,typename T>
class serial_switch<A,T,Serializer::PTRTAG_CLS>
{
public:
	static void g(A& ar,T& val)
	{
		val.Serialize(ar);
	}
};

template<typename A,typename T>
class serial_switch<A,T,Serializer::PTRTAG_POD> : public serial_pod<A,T> {};

template<typename A,typename T>
class serial_switch<A,T,Serializer::PTRTAG_OBJ>
{
public:
	static void g(A& ar,Object& val)
	{
		val.Serialize(ar);
	}
};

template<typename A,typename T>
class serial_switch<A,T,Serializer::PTRTAG_CACHED>
{
public:
	static void g(A& ar,Object& val)
	{
		val.Serialize(ar);
	}
};


template<typename A,typename T>
class serial_helper_func : public serial_switch<A,T,serial_type<T>::value>
{
public:

};

template<typename A,typename T>
class serial_helper_func<A,T*> : public serial_ptr<A,T,serial_type<T>::value>
{
public:

};


template<typename A,typename X,typename Y>
class serial_helper_func<A,std::pair<X,Y> >
{
public:
	typedef std::pair<X,Y> type;
	static void g(A& ar,type& val)
	{
		if(serial_pod<A,X>::value && serial_pod<A,Y>::value)
		{
			serial_pod<A,type>::g(ar,val);
		}
		else
		{
			ar & val.first & val.second;
		}
	}
};


template<typename A,typename T,typename X>
class serial_helper_func<A,std::vector<T,X> > : public serial_arr<A,T,std::vector<T,X> >
{
public:

};

template<typename A,typename T,typename X>
class serial_helper_func<A,arr_1t<T,X> > : public serial_arr<A,T,arr_1t<T,X> >
{
public:

};

template<typename A,typename T>
class serial_helper_func<A,StringBuffer<T> > : public serial_arr<A,T,StringBuffer<T> >
{
public:

};


template<typename A,typename K,typename T,typename X,typename Y>
class serial_helper_func<A,std::map<K,T,X,Y> > : public serial_set<A,std::pair<K,T>,std::map<K,T,X,Y> >
{
public:

};

template<typename A,typename K,typename T,typename X,typename Y>
class serial_helper_func<A,bst_map<K,T,X,Y> > : public serial_set<A,std::pair<K,T>,bst_map<K,T,X,Y> >
{
public:

};

template<typename A,typename K,typename T,typename X,typename Y>
class serial_helper_func<A,indexer_map<K,T,X,Y> > : public serial_set<A,std::pair<K,T>,indexer_map<K,T,X,Y> >
{
public:

};

template<typename A,typename T,typename X,typename Y>
class serial_helper_func<A,std::set<T,X,Y> > : public serial_set<A,T,std::set<T,X,Y> >
{
public:

};

template<typename A,typename T,typename X,typename Y>
class serial_helper_func<A,bst_set<T,X,Y> > : public serial_set<A,T,bst_set<T,X,Y> >
{
public:

};

template<typename A,typename T,typename X,typename Y>
class serial_helper_func<A,indexer_set<T,X,Y> > : public serial_set<A,T,indexer_set<T,X,Y> >
{
public:

};

template<typename A,typename T,size_t N>
class serial_helper_func<A,T[N] >
{
public:
	typedef T type[N];

	static void g(SerializerReader& ar,type& val)
	{
		int n=ar.size_count(N);
		if(n!=(intarr_t)N) ar.errstr("invalid_array_size");
		serial_arr_handler<A,T>::g(ar,&val[0],n);
	}

	static void g(SerializerWriter& ar,type& val)
	{
		int n=ar.size_count(N);
		serial_arr_handler<A,T>::g(ar,&val[0],n);
	}

};

template<typename A,typename T>
class serial_helper_func<A,LitePtrT<T> > : public serial_ptr_holder<A,T,LitePtrT<T> >
{
public:

};

template<typename A,typename T>
class serial_helper_func<A,DataPtrT<T> > : public serial_ptr_holder<A,T,DataPtrT<T> >
{
public:

};

template<typename A,typename T,typename X,typename Y>
class serial_helper_func<A,std::basic_string<T,X,Y> > : public serial_arr<A,T,std::basic_string<T,X,Y> >
{
public:

};


template<typename A>
class DLLIMPEXP_EWA_BASE serial_helper_func<A,String>
{
public:
	typedef String type;

	static void g(SerializerReader& ar,type& val);
	static void g(SerializerWriter& ar,type& val);
};



template<typename A,typename T,typename X>
class serial_helper_func<A,arr_xt<T,X> >
{
public:
	typedef arr_xt<T,X> type;

	static void g(SerializerReader& ar,type& val)
	{
		uint32_t d[8];
		arr_xt_dims x;

		ar.recv_checked((char*)d,sizeof(d));
		for(int i=0;i<6;i++)
		{
			d[7]=d[7]^d[i];
			x[i]=d[i];
		}

		d[7]=d[7]^d[6];
		if(d[7]!=0)
		{
			ar.errstr("invalid arr_xt dimension");
		}

		val.resize(x,d[6]);
		serial_arr_handler<A,T>::g(ar,val.data(),d[6]);

	}

	static void g(SerializerWriter& ar,type& val)
	{

		const arr_xt_dims &x(val.size_ptr());

		uint32_t d[8];
		d[7]=0;
		for(int i=0;i<6;i++)
		{
			d[i]=x[i];
			d[7]=d[7]^x[i];
		}
		d[6]=val.size();
		d[7]=d[7]^d[6];

		ar.send_checked((char*)d,sizeof(d));
		serial_arr_handler<A,T>::g(ar,val.data(),d[6]);
	}
};


template<typename A,typename T>
class serial_helper_test : public serial_helper_func<A,T>
{
public:

};

template<typename A,typename T>
class serial_helper_test<A,const T>
{
public:
	static void g(SerializerWriter& ar,const T& val)
	{
		serial_helper_func<A,T>::g(ar,const_cast<T&>(val));
	}

	static void g(SerializerReader& ar,const T&)
	{
		ar.errstr("attempt_to_serialize_const");
	}
};


template<typename T>
SerializerReader& operator >>(SerializerReader& ar,T& val)
{
	serial_helper_test<SerializerReader,T>::g(ar,val);
	return ar;
}

template<typename T>
SerializerWriter& operator <<(SerializerWriter& ar,T& val)
{
	serial_helper_test<SerializerWriter,T>::g(ar,val);
	return ar;
}

template<typename T>
SerializerReader& operator &(SerializerReader& ar,T& val)
{
	serial_helper_test<SerializerReader,T>::g(ar,val);
	return ar;
}

template<typename T>
SerializerWriter& operator &(SerializerWriter& ar,T& val)
{
	serial_helper_test<SerializerWriter,T>::g(ar,val);
	return ar;
}

template<typename T>
Serializer& operator &(Serializer& ar,T& val)
{
	if(ar.is_reader())
	{
		serial_helper_test<SerializerReader,T>::g(static_cast<SerializerReader&>(ar),val);
		return ar;
	}

	if(ar.is_writer())
	{
		serial_helper_test<SerializerWriter,T>::g(static_cast<SerializerWriter&>(ar),val);
		return ar;
	}

	ar.errstr("INVALID_SERIALIZER");
	return ar;
}

template<typename T>
SerializerReader& operator >>(Serializer& ar_,T& val)
{
	SerializerReader& ar(ar_.reader());
	serial_helper_test<SerializerReader,T>::g(ar,val);
	return ar;
}

template<typename T>
SerializerWriter& operator <<(Serializer& ar_,T& val)
{
	SerializerWriter& ar(ar_.writer());
	serial_helper_test<SerializerWriter,T>::g(ar,val);
	return ar;
}

template<typename T>
SerializerDuplex& operator >>(SerializerDuplex& ar,T& val)
{
	ar.reader() >> val;
	return ar;
}

template<typename T>
SerializerDuplex& operator <<(SerializerDuplex& ar,T& val)
{
	ar.writer() << val;
	return ar;
}

EW_LEAVE
#endif
