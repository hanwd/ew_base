#ifndef __H_EW_BASIC_OBJECT_EX__
#define __H_EW_BASIC_OBJECT_EX__

#include "ewa_base/basic/object.h"
#include "ewa_base/basic/pointer.h"
#include "ewa_base/basic/bitflags.h"
#include "ewa_base/collection/arr_1t.h"

EW_ENTER


class DLLIMPEXP_EWA_BASE ObjectGroup
{
	typedef arr_1t<ObjectData*> impl_type;
public:

	typedef DataPtrT<ObjectData> value_proxy;
	typedef arr_1t<value_proxy> impl_proxy;

	ObjectGroup();
	ObjectGroup(const ObjectGroup&);
	ObjectGroup& operator=(const ObjectGroup&);
	~ObjectGroup();

	value_proxy& operator[](size_t n);
	const value_proxy& operator[](size_t n) const;

	value_proxy& back();
	const value_proxy& back() const;

	size_t size() const;

	void append(ObjectData*);
	void remove(ObjectData*);

	void clear();

	void swap(ObjectGroup& o);

	void Serialize(SerializerHelper sh);

	bool empty() const;

private:

	impl_proxy& __proxy(){ return *(impl_proxy*)&impl; }
	const impl_proxy& __proxy() const { return *(impl_proxy*)&impl; }

	impl_type impl;
};


template<typename T,typename B=Object>
class ObjectGroupT : public B
{
public:
	typedef arr_1t<DataPtrT<T> > impl_proxy;
	typedef DataPtrT<T> value_proxy;

	ObjectGroupT():B(){}
	ObjectGroupT(const ObjectGroupT& o):B(o),impl(o.impl){}

	template<typename X>
	ObjectGroupT(const X& o):B(o){}

	value_proxy& operator[](size_t n)
	{
		return proxy()[n];
	}

	const value_proxy& operator[](size_t n) const
	{
		return proxy()[n];		
	}

	value_proxy& back()
	{
		return proxy().back();	
	}

	const value_proxy& back() const
	{
		return proxy().back();	
	}

	size_t size() const
	{
		return impl.size();
	}

	void append(T* d)
	{
		impl.append(d);
	}

	template<typename T2>
	void append(DataPtrT<T2> p)
	{
		append(p.get());
	}

	void remove(T* d)
	{
		impl.remove(d);
	}

	void clear()
	{
		impl.clear();
	}

	void swap(ObjectGroupT& o)
	{
		impl.swap(o.impl);
	}

	bool empty() const
	{
		return impl.empty();
	}

	typedef typename impl_proxy::iterator iterator;
	typedef typename impl_proxy::reverse_iterator reverse_iterator;
	typedef typename impl_proxy::const_iterator const_iterator;
	typedef typename impl_proxy::const_reverse_iterator const_reverse_iterator;

	iterator begin(){return proxy().begin();}
	iterator end(){return proxy().end();}
	reverse_iterator rbegin(){return proxy().rbegin();}
	reverse_iterator rend(){return proxy().rend();}
	const_iterator begin() const {return proxy().begin();}
	const_iterator end() const {return proxy().end();}
	const_reverse_iterator rbegin() const {return proxy().rbegin();}
	const_reverse_iterator rend() const {return proxy().rend();}

	template<typename IT>
	void append(IT t1,IT t2){while(t1!=t2) append(*t1++);}

	void Serialize(SerializerHelper sh)
	{
		Serializer& ar(sh.ref(0));
		ar & proxy();
	}

	inline impl_proxy& proxy(){ return *(impl_proxy*)&impl; }
	inline const impl_proxy& proxy() const {return *(const impl_proxy*)&impl;}

protected:
	ObjectGroup impl;
};

class DLLIMPEXP_EWA_BASE Creator : public ObjectData
{
public:

	virtual Object* Create(const String&)=0;
};


class DLLIMPEXP_EWA_BASE ObjectCreator : public ObjectT<Creator>
{
public:
	ObjectCreator();
	static ObjectCreator& current();

	Object* Create(const String& name);

	template<typename T>
	T* CreateT(const String& name)
	{
		Object* pbase=Create(name);
		T* pderv=dynamic_cast<T*>(pbase);
		if(pderv) return pderv;
		if(pbase && pbase!=pbase->GetObjectInfo().GetCachedInstance())
		{
			delete pbase;
		}
		return NULL;
	}

};




EW_LEAVE
#endif

