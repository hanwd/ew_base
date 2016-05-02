#include "ewa_base/scripting/callable_code.h"
#include "ewa_base/scripting/executor.h"
#include "ewa_base/util/strlib.h"



EW_ENTER

bool CallableCode::ToValue(String& v,int) const
{
	v.Printf("def_function:%s(0x%p)",GetName(),this);
	return true;
}

void CallableCode::__set_helpdata(const String& s)
{ 
	m_pHelp.reset(new HelpData); 
	m_pHelp->parse(m_sName, s);

	if(tok_beg.line>0 && tok_end.line>0)
	{
		String *p=pSource->aLines.data();
		String src_code=string_join(p+tok_beg.line-1,p+tok_end.line,"\n");
		const char* p1=src_code.c_str();
		const char* p2=p1+src_code.size();

		p1+=tok_beg.cpos-1;
		p2-=p[tok_end.line-1].size()-tok_end.cpos;

		m_pHelp->helpmap["source"]=String(p1,p2);
	}
}


int CallableCode::__getindex(Executor& ewsl,const String& index)
{
	return CallableData::__getindex(ewsl,index);
}


void CallableCode::Serialize(Serializer& ar)
{
	int v = ar.local_version(0);
	if (v == 0)
	{
		ar & m_sName & m_pHelp & flags;
		ar & nLocal & nParam & nShift;
		ar & aInsts;
		ar & aLLVar;
		ar & aDepends;

		ar & aInfos;
	}
	else
	{
		ar.errver();
	}
}



CallableCode::CallableCode()
{
	nParam=0;
	nLocal=0;
	nShift=0;
}

CallableCode::~CallableCode()
{

}


int CallableCode::__fun_call(Executor& ewsl,int pm)
{
	EW_ASSERT(ewsl.ci0.nbx==ewsl.ci1.nsp-pm);

	ewsl.co_this->aFrame.push_back(ewsl.ci0);

	StackState0& ci0(ewsl.ci0);

	ci0.frm.reset(this);
	ci0.nip = aInsts.data();
	ci0.dat = aLLVar.data();

	ci0.sp1=ci0.nbx;
	ci0.nbx=ci0.sp2=ewsl.ci1.nsp=ci0.sp1+nLocal;

	if(ewsl.ci1.nsp>=ewsl.ci1.end)
	{
		ewsl.ci0=ewsl.co_this->aFrame.back();
		ewsl.co_this->aFrame.pop_back();
		ewsl.kerror("stack overflow");
	}


	// 系统变量的载入指令(XOP_GET_GGVAR)，保存的是变量名称，在实际执行中使用偏移
	// 第一次执行时，需要计算出偏移，以后执行不需要再次计算
	// p1=名称在aLLVar中的偏移，p2为计算得到的偏移
	if (!state.get(STATE_SYSTEM_VARIABLES_LOADED))
	{
		for (bst_set<String>::iterator it=aDepends.begin(); it!=aDepends.end(); ++it)
		{
			CG_GGVar::current().import(*it);
		}

		for (size_t i = 0; i < aInsts.size(); i++)
		{
			if (aInsts[i].op != XOP_GET_GGVAR && aInsts[i].op != XOP_SET_GGVAR) continue;

			String name = variant_handler<String>::raw(aLLVar[aInsts[i].p1]);
			aInsts[i].p2 = CG_GGVar::current().find1(name);

			if (aInsts[i].p2 < 0)
			{
				ci0.nip = &aInsts[i+1];
				Exception::XError(name+" not found!");
			}

		}
		state.add(STATE_SYSTEM_VARIABLES_LOADED);
	}

	for(size_t i=pm+1;i<=nParam;i++)
	{
		ci0.sp1[i].clear();
	}

	// ... is used
	if(flags.get(TNode_val_function::FLAG_DOT3_USED))
	{
		int d=pm-nParam+1;
		arr_xt<Variant> vv;
		vv.assign(ci0.sp1+nParam,d>0?d:0);
		ci0.sp1[nParam].ref_unique<arr_xt<Variant> >().swap(vv);
	}

	int np=nParam;

	// this is used
	if(flags.get(TNode_val_function::FLAG_THIS_USED))
	{
		ci0.sp1[++np]=ewsl.ci1.nbp[StackState1::SBASE_THIS];
	}

	if(flags.get(TNode_val_function::FLAG_SELF_USED))
	{
		ci0.sp1[++np].kptr(ewsl.ci2.pfn.get());
	}	

	return STACK_BALANCED;

}

int CallableClosure::__fun_call(Executor& ewsl,int pm)
{
	int k=pInst->__fun_call(ewsl,pm);
	if(k==STACK_BALANCED)
	{
		// setup captured targets
		for(int i=0;i<(int)aCapture.size();i++)
		{
			ewsl.ci0.sp1[pInst->nShift+i]=aCapture[i];
		}
	}		
	return k;
}

void CallableClosure::Serialize(Serializer& ar)
{
	ar & pInst;
	ar & aCapture;
}


CallableSource::CallableSource(){}
CallableSource::CallableSource(const String& s)
{
	aLines = string_lines(s);
}

void CallableSource::Serialize(Serializer& ar)
{
	int v = ar.local_version(0);
	if (v == 0)
	{
		ar & aLines;
	}
	else
	{
		ar.errver();
	}
}

IMPLEMENT_OBJECT_INFO(CallableCode,ObjectInfo);
IMPLEMENT_OBJECT_INFO(CallableClosure,ObjectInfo);
IMPLEMENT_OBJECT_INFO(CallableSource, ObjectInfo);

EW_LEAVE
