#include "ewa_base/scripting.h"

EW_ENTER



template<>
CallableMetatableT<VariantTable>::CallableMetatableT()
:CallableMetatable("table")
{
	CG_GGVar::current().sm_meta[type_flag<type>::value].reset(this);
}

template<>
int CallableMetatableT<VariantTable>::__fun_call(Executor& ewsl, int pm)
{
	ewsl.check_pmc(this, pm, 0);
	ewsl.ci0.nbx[1].kptr(new CallableWrapT<VariantTable>);
	return 1;

}
template class CallableMetatableT<VariantTable>;




class CallableFunctionClass : public CallableFunction
{
public:
	CallableFunctionClass():CallableFunction("#new_metatable"){}

	virtual int __fun_call(Executor& ewsl,int pm)
	{
		ewsl.check_pmc(this,pm,0);
		CallableMetatable* pcls=new CallableMetatable;
		ewsl.ci0.nbx[1].kptr(pcls);
		ewsl.ci0.nbx[2].kptr(new CallableTableProxy(pcls->table_self,pcls));
		ewsl.ci0.nbx[3].kptr(new CallableTableProxy(pcls->table_meta,pcls));
		return 3;
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionClass, ObjectInfo);
};

IMPLEMENT_OBJECT_INFO(CallableFunctionClass, ObjectInfo);

class DLLIMPEXP_EWA_BASE CallableFunctionClosure : public CallableFunction
{
public:
	CallableFunctionClosure():CallableFunction("#new_closure"){}

	virtual int __fun_call(Executor& ewsl,int pm)
	{
		if(pm<2) ewsl.kerror("invalid call");

		DataPtrT<CallableClosure> cls=new CallableClosure;

		cls->pInst.reset(dynamic_cast<CallableCode*>(ewsl.ci0.nbx[1].kptr()));
		if(!cls->pInst)
		{
			ewsl.kerror("invalid param");
		}

		cls->aCapture.assign(ewsl.ci0.nbx+2,pm-1);
		ewsl.ci0.nbx[1].kptr(cls.get());
		return 1;

	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionClosure, ObjectInfo);
};

IMPLEMENT_OBJECT_INFO(CallableFunctionClosure, ObjectInfo);

class DLLIMPEXP_EWA_BASE CallableFunctionTarget : public CallableFunction
{
public:
	CallableFunctionTarget():CallableFunction("#new_target"){}

	virtual int __fun_call(Executor& ewsl,int pm)
	{
		ewsl.check_pmc(this,1,1);
		int64_t n=ewsl.ci1.nsp[0].get<int64_t>();
		ewsl.ci1.nsp[0].ref_unique<arr_xt<Variant> >().resize(n);
		return 1;
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionTarget, ObjectInfo);
};

IMPLEMENT_OBJECT_INFO(CallableFunctionTarget, ObjectInfo);

void init_module_table()
{

	CG_GGVar& gi(CG_GGVar::current());

	gi.add_inner<CallableMetatableT<VariantTable> >();

	gi.add_inner<CallableFunctionClass>();
	gi.add_inner<CallableFunctionClosure>();
	gi.add_inner<CallableFunctionTarget>();

}

EW_LEAVE
