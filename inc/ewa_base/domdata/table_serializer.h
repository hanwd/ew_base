#ifndef __H_EW_DOMDATA_TABLE_SERIALIZER__
#define __H_EW_DOMDATA_TABLE_SERIALIZER__


#include "ewa_base/basic.h"
#include "ewa_base/domdata/dobject.h"
#include "ewa_base/math/math_def.h"

EW_ENTER

template<typename T>
struct VariantSerializable
{
	static void serialize(Variant& v, T& value)
	{
		v[i].SerializeVariant(v, value);
	}
};


class DLLIMPEXP_EWA_BASE TableSerializer : public Object
{
public:

	enum
	{
		READER,
		WRITER,
	};


	bool is_reader(){return type==READER;}
	bool is_writer(){return type==WRITER;}

	VariantTable& value;

	const int type;

	TableSerializer(int t,VariantTable& v);

	void link(const String& s,double& v);
	void link(const String& s,int& v);
	void link(const String& s,BitFlags& v,int m);

	void link(const String& s,String& v);
	void link(const String& s,String& v,const String& d);

	void link(const String& s,DWhen& v)
	{
		link(s+".t_begin",v.t_begin);
		link(s+".t_end",v.t_end);
		link(s+".t_step",v.t_step);

	}

	template<typename T>
	struct linker
	{
		static void g(Variant& var, T& value, int dir)
		{
			value.SerializeVariant(var, dir);
		}
	};



	template<typename T, int N>
	struct linker < tiny_storage<T, N> >
	{
		static void g(Variant& var, tiny_storage<T, N>& value, int dir)
		{
			arr_xt<Variant>& a(var.ref<arr_xt<Variant> >());

			if (dir == 1)
			{
				a.resize(N);
				for (int i = 0; i < N; ++i)
				{
					a[i].reset(value[i]);
				}
			}
			else
			{
				int d = std::min(N, (int)a.size());
				for (int i = 0; i < d; ++i)
				{
					value[i] = variant_cast<T>(a[i]);
				}
			}


		}
	};

	template<typename T>
	void link_t(const String& s,arr_1t<T>& v)
	{
		// read:  var a   --> value v
		// write: value v --> var a
		arr_xt<Variant> &a(value[s].ref<arr_xt<Variant> >());
		if(is_reader())
		{
			v.resize(a.size());
		}
		else
		{
			a.resize(v.size());
		}

		int dir = is_reader() ? -1 : +1;

		for (size_t i = 0; i < v.size(); i++)
		{
			// v[i].SerializeVariant(a[i],dir);
			linker<T>::g(a[i], v[i], dir);
		}
	}


	template <typename T, int N>
	void link(const String& s, tiny_vec<T, N>& v)
	{
		if (N > 0) {
			link(s + ".x", v[0]);
		}
		if (N > 1) {
			link(s + ".y", v[1]);
		}
		if (N > 2) {
			link(s + ".z", v[2]);
		}
		if (N > 3) {
			link(s + ".val3", v[3]);
		}
		if (N > 4) {
			link(s + ".val4", v[4]);
		}
	}

	template <typename T, int N>
	void link(const String& s, tiny_box<T, N>& v)
	{
		link(s + ".lo", v.lo);
		link(s + ".hi", v.hi);
	}

	template <typename T, int N>
	void link(const String& s, tiny_storage<T, N>& v)
	{
		if (N > 0) {
			link(s + ".x", v[0]);
		}
		if (N > 1) {
			link(s + ".y", v[1]);
		}
		if (N > 2) {
			link(s + ".z", v[2]);
		}
		if (N > 3) {
			link(s + ".val3", v[3]);
		}
		if (N > 4) {
			link(s + ".val4", v[4]);
		}
	}

	template <int N>
	void link(const String& s, tiny_box<String, N>& v)
	{
		link(s + ".lo", v.lo);
		link(s + ".hi", v.hi);
	}
};

class DLLIMPEXP_EWA_BASE TableSerializerReader : public TableSerializer
{
public:
	TableSerializerReader(VariantTable& v):TableSerializer(READER,v){}
};

class DLLIMPEXP_EWA_BASE TableSerializerWriter : public TableSerializer
{
public:
	TableSerializerWriter(VariantTable& v):TableSerializer(WRITER,v){}
};


EW_LEAVE
#endif
