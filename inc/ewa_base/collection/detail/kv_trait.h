
#ifndef __H_EW_COLLECTION_KV_TRAIT__
#define __H_EW_COLLECTION_KV_TRAIT__

#include "ewa_base/config.h"
#include <utility>

EW_ENTER


template<typename K,typename V=void>
class kv_trait
{
public:

	typedef K key_type;
	typedef V mapped_type;
	typedef std::pair<key_type,mapped_type> value_proxy;
	typedef std::pair<const key_type,mapped_type> value_type;

	static const key_type& key(const key_type& v)
	{
		return v;
	}
	static const key_type& key(const value_proxy& v)
	{
		return v.first;
	}

	static const mapped_type& value(const value_proxy& v)
	{
		return v.second;
	}

	static value_proxy pair(const key_type& k,const mapped_type& v=mapped_type())
	{
		return value_proxy(k,v);
	}

	static const value_proxy& pair(const value_proxy& k)
	{
		return k;
	}

	static void set_value(value_proxy&, const key_type&){}

	static void set_value(value_proxy& v,const value_proxy& k)
	{
		v.second=k.second;
	}

#ifdef EW_C11

	static void set_value(value_proxy& v,value_proxy&& k)
	{
		v.second=k.second;
	}

	static value_proxy pair(value_proxy&& k)
	{
		return std::forward<value_proxy>(k);
	}

	static value_proxy pair(key_type&& k)
	{
		return value_proxy(std::forward<key_type>(k),mapped_type());
	}
#endif
};


template<typename K>
class kv_trait<K,void>
{
public:

	typedef K key_type;
	typedef K mapped_type;
	typedef K value_proxy;
	typedef const K value_type;

	static const key_type& key(const value_proxy& v)
	{
		return v;
	}

	static const mapped_type& value(const value_proxy& v)
	{
		return v;
	}

	static const value_proxy& pair(const key_type& k)
	{
		return k;
	}

	static void set_value(value_proxy&, const key_type&){}

#ifdef EW_C11
	static value_proxy&& pair(value_proxy&& k)
	{
		return std::forward<value_proxy>(k);
	}
#endif
};


EW_LEAVE

#endif
