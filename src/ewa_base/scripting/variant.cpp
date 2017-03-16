#include "ewa_base/scripting/variant.h"
#include "ewa_base/scripting/callable_code.h"
#include "ewa_base/scripting/callable_array.h"
#include "ewa_base/scripting/callable_table.h"
#include "ewa_base/scripting/callable_class.h"
#include "ewa_base/scripting/executor.h"
#include "ewa_base/util/strlib.h"
EW_ENTER


template<unsigned N> 
struct pl1_equal_fk
{
	typedef typename flag_type<N>::type type;
	static bool value(const Variant& lhs,const Variant& rhs)
	{
		return variant_handler<type>::raw(lhs)==variant_handler<type>::raw(rhs);
	}
};

template<> 
struct pl1_equal_fk<0>
{
	static bool value(const Variant& lhs,const Variant& rhs)
	{
		const CallableData* p1=variant_handler<CallableData*>::raw(lhs);
		const CallableData* p2=variant_handler<CallableData*>::raw(rhs);
		if(p1==NULL) return p2==NULL;
		if(p2==NULL) return false;
		return p1->IsEqualTo(p2);
	}
};

bool Variant::operator==(const Variant& v2) const
{
	int t1=type();
	int t2=v2.type();
	if(t1!=t2)
	{
		return false;
	}

	typedef bool (*fn)(const Variant&,const Variant&);
	typedef lookup_table_4bit<pl1_equal_fk,fn> lk;

	return lk::test(t1)(*this,v2);
}

bool Variant::operator!=(const Variant& v2) const
{
	return !((*this)==v2);
}


void VariantTable::Serialize(SerializerHelper sh)
{
	Serializer& ar(sh.ref(0));

	indexer_map<String,Variant>& tmp_value(*this);
	ar & tmp_value;
}

void Variant::Serialize(SerializerHelper sh)
{
	Serializer& ar(sh.ref(0));

	if(ar.is_reader())
	{
		clear();
		ar & flag;
		if(flag==-1)
		{
			flag=0;
			DataPtrT<CallableData> pval;
			ar & pval;

			if(CallableModule* pmodule=pval?pval->ToModule():NULL)
			{
				String libname=pmodule->m_sClassName;
				arr_1t<String> ap=string_split(libname,".");
				CG_GGVar::current().import(ap[0]);

				pmodule=dynamic_cast<CallableModule*>(CG_GGVar::current()[ap[0]].kptr());

				size_t n=ap.size();
				if(n==1)
				{
					kptr(pmodule);
					return;
				}

				for(size_t i=1;pmodule && i<n-1;i++)
				{
					pmodule=dynamic_cast<CallableModule*>(pmodule->value[ap[i]].kptr());						
				}

				if(pmodule && pmodule->value.find1(ap[n-1])>=0)
				{
					(*this)=pmodule->value[ap[n-1]];
				}
				else
				{
					this_logger().LogWarning("unknown module %s",libname);
				}
			
			}
			else
			{
				kptr(pval.get());
			}
		}
		else
		{
			ar.reader().recv((char*)&data,sizeof(data));
		}
		return;
	}
	
	if(ar.is_writer())
	{
		ar & flag;
		if(flag==-1)
		{
			DataPtrT<CallableData> pval(kptr());
			ar & pval;
		}
		else
		{
			ar.writer().send((char*)&data,sizeof(data));
		}
		return;
	}

}



EW_LEAVE
