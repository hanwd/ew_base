#include "ewa_base/scripting/variant.h"

#include "ewa_base/scripting/callable_code.h"
#include "ewa_base/scripting/callable_table.h"
#include "ewa_base/scripting/callable_array.h"
#include "ewa_base/scripting/variant_op.h"

#include "ewa_base/scripting/executor.h"
#include "ewa_base/scripting/codegen.h"

#include "ewa_base/util/strlib.h"

EW_ENTER

DataPtrT<CallableData> Executor::parse(const String& ss)
{
	CodeGen cgen;
	try
	{
		if(!cgen.parse(ss))
		{
			return NULL;
		}
		return cgen.get();
	}
	catch(std::exception& e)
	{
		this_logger().LogError(e.what());
	}

	return NULL;
}

String Executor::make_path(const String& s)
{
	return System::MakePath(s,get_cwd());
}

bool Executor::set_cwd(const String& s)
{
	String value(ci1.nbp[StackState1::SBASE_CWD].ref<String>());
	String cwd=System::MakePath(s,value);
	ci1.nbp[StackState1::SBASE_CWD].reset(cwd);
	return true;
}

String Executor::get_cwd()
{
	return ci1.nbp[StackState1::SBASE_CWD].ref<String>();
}

class CallableExecutorState : public CallableData
{
public:
	CG_VariableGlobal var_import;
};

bool Executor::prepare(const String& ss, int t)
{
	CodeGen cgen;

	CallableExecutorState* p=dynamic_cast<CallableExecutorState*>(ci1.nbp[StackState1::SBASE_STATE].kptr());
	if(!p)
	{
		p=new CallableExecutorState;
		ci1.nbp[StackState1::SBASE_STATE].kptr(p);
	}

	cgen.cg_system.varmap = p->var_import.varmap;

	cgen.cg_global.add_globals(tb1);

	if (!cgen.prepare(ss, t))
	{
		return false;
	}

	push(cgen.get());

	while (cgen.cg_system.next)
	{
		CG_Variable* q = cgen.cg_system.next;
		cgen.cg_system.next = q->next;
		p->var_import.add(q);
	}


	return true;

}

bool Executor::execute(const String& ss,int pmc,int kep)
{
	if(!prepare(ss,DIRECT_STRING))
	{
		return false;
	}
	return callx(pmc,kep);
}


bool Executor::execute_file(const String& ss,int pmc,int kep)
{
	if(!prepare(ss,INDIRECT_FILE))
	{
		return false;
	}
	return callx(pmc,kep);
}

void Executor::kerror(const String& s)
{
	Exception::XError(s);
}

void Executor::kthrow(int n)
{
	if(!ci0.nip)
	{
		Exception::XError("invalid kthrow");
	}

	ci0.nbx=ci1.nsp-n;
	ci2.xop.op=XOP_TRY_THROW;
	ci2.xop.p1=n;
	ci0.nip=&ci2.xop;
}

Executor::Executor(size_t stack_size)
	:tb0(CG_GGVar::current())
	,tb1(tb1_internal)
{
	co_main.reset(new CallableCoroutine(stack_size));
	co_this=co_main;
	co_main->nState=CallableCoroutine::STATE_RUNNING;

	ci0=co_main->ci0;
	ci1=co_main->ci1;

	ci0.nip=NULL;

}

void Executor::reset()
{

	co_this=co_main;
	co_main->nState=CallableCoroutine::STATE_RUNNING;

	ci0=co_main->ci0;
	ci1=co_main->ci1;

	ci0.nip=NULL;
}

Executor::Executor(const Executor& o)
	:tb0(CG_GGVar::current())
	,tb1(tb1_internal)
{
	co_main.reset(new CallableCoroutine);
	co_this=co_main;
	co_main->nState=CallableCoroutine::STATE_RUNNING;

	ci0=co_main->ci0;
	ci1=co_main->ci1;

	ci0.nip=NULL;

	tb1=o.tb1;
}

Executor& Executor::operator=(const Executor& o)
{
	tb1=o.tb1;
	return *this;
}

Executor::~Executor()
{
	co_main.reset(NULL);
	co_this.reset(NULL);
	ci2.pfn.reset(NULL);
}


inline void Executor_update_idx(Executor& ewsl,XopInst* cip)
{
	ewsl.ci2.pfn.reset(ewsl.ci1.nsp[0].kptr());
	if(!ewsl.ci2.pfn)
	{
		ewsl.kerror("invalid object");
	}

	ewsl.ci2.idx=ewsl.ci0.dat[cip->p1].ptr<String>();
	if(!ewsl.ci2.idx)
	{
		ewsl.kerror("invalid string index");
	}
}

inline void Executor_update_pmc(Executor& ewsl,int pm)
{
	ewsl.ci2.pfn.reset(ewsl.ci1.nsp[0].kptr());

	if(!ewsl.ci2.pfn)
	{
		ewsl.kerror("invalid object");
	}

	if(pm<0)
	{
		ewsl.ci2.pmc=ewsl.ci1.nsp[-1].get<int64_t>()-pm;
		ewsl.ci1.nsp-=2;
	}
	else
	{
		ewsl.ci2.pmc=pm;
		ewsl.ci1.nsp-=1;
	}

	ewsl.ci0.nbx=ewsl.ci1.nsp-ewsl.ci2.pmc;
}



inline bool Executor_stackframe_leave(Executor& ewsl,int pm)
{

	Variant* sp1=ewsl.ci0.sp1;
	Variant* sp2=ewsl.ci0.sp2;

	int ret=ewsl.ci1.nsp-sp2;
	if(pm<0)
	{
		ret=ret-1;
	}

	ewsl.ci0=ewsl.co_this->aFrame.back();
	ewsl.co_this->aFrame.pop_back();

	int kep=ewsl.ci0.kep;

	if(kep>=0)
	{
		if(ret>=kep)
		{
			for(int i=1;i<=kep;i++)
			{
				sp1[i]=sp2[i];
			}
		}
		else
		{
			for(int i=1;i<=ret;i++)
			{
				sp1[i]=sp2[i];
			}
			for(int i=ret+1;i<=kep;i++)
			{
				sp1[i].clear();
			}
		}
		ewsl.ci1.nsp=sp1+kep;
	}
	else
	{
		for(int i=1;i<=ret;i++)
		{
			sp1[i]=sp2[i];
		}
		ewsl.ci1.nsp=sp1+ret;
		ewsl.push(ret-1);
	}

	return ewsl.ci0.nip!=NULL;

}


static XopInst xop_inst_throw_exception(XOP_TRY_THROW,1);

int Executor_swap_context(Executor& ewsl,int pm);

class ExceptionEwsl : public Exception
{
public:
	ExceptionEwsl(Variant* bx,int pm):nbx(bx),pmc(pm){}

	Variant* nbx;
	int pmc;

};

void Executor::_vm_handle_exception(std::exception &e)
{

	StringBuffer<char> msg;

	if(ExceptionEwsl* p=dynamic_cast<ExceptionEwsl*>(&e))
	{		
		ExceptionEwsl& v(*p);

		msg<<"EWSL_Exception:";	
		msg<<String::Format("%d param(s)",v.pmc);

		for(int i=1;i<=v.pmc;i++)
		{
			if(i==6 && v.pmc>10)
			{
				msg << "\n --> " << v.pmc - 10 << " params ... ";
				i+=v.pmc-10;
			}

			msg<<String::Format("\n --> param[%d] : ",i);
			msg<<variant_cast<String>(v.nbx[i]);
		}
	}
	else
	{
		msg<<"CPP_Exception:" << e.what();	
	}

	while(1)
	{
		if (co_this->aCatch.size()>0)
		{
			ci0.nbx = ci1.nsp;
			push(msg.c_str());
			ci0.nip = (&xop_inst_throw_exception);
			return;
		}

		int n2 = (int)co_this->aFrame.size() - 1;
		int n1 = std::max(0, n2 - 10);

		while (ci0.frm)
		{
			String line;

			size_t n = ci0.nip - ci0.frm->aInsts.data() - 1;
			if (n < ci0.frm->aInfos.size())
			{
				tokInfo item(ci0.frm->aInfos[n]);
				if (item.line>0)
				{
					line = String::Format(" at %d,%d", item.line, item.cpos);
					if(ci0.frm->pSource)
					{
						arr_1t<String>& lines(ci0.frm->pSource->aLines);
						if(item.line-1<(int)lines.size())
						{
							int b1=std::max(0,item.cpos-15);
							line=line+":"+string_trim(string_trim(lines[item.line-1].substr(b1,item.cpos+15-b1)),TRIM_B,'\t');
						}
					}
				}
			}

			String name = ci0.frm->GetName();
			if (name == "") name = "[unamed]";

			msg << "\n --> called by " << name << line;

			if (n2 < 0)	break;
			if (n2 < n1 && n2>13)
			{
				msg << "\n --> " << n2 - 10 << " frames ... ";
				n2 = 10;
			}
			ci0 = co_this->aFrame[n2--];
		}

		if(co_this==co_main)
		{
			break;
		}

		co_last=co_main;		
		Executor_swap_context(*this,1);
	}


	ci0.nip = NULL;
	ci1.nbp = co_main->ci1.nbp;
	ci1.nsp = co_main->ci1.nbp + StackState1::SBASE_KEEP;
	ci1.end = co_main->ci1.end;
	co_last.reset(NULL);
	co_this = co_main;
	co_main->aCatch.clear();
	co_main->aFrame.clear();

	Exception::XError(msg);

}

void Executor::_vm_run1(int d)
{

	while (ci0.nip)
	{
		try
		{
			_vm_run2(d);

			EW_ASSERT(co_this==co_main);
			EW_ASSERT(co_this->aFrame.empty());
		}
		catch(std::exception& e)
		{
			_vm_handle_exception(e);
		}
	}

}


void Executor::_vm_run2(int k)
{

	Executor& ewsl(*this);
	int ret;

	XopInst* cip;
	while(1)
	{
		cip=ci0.nip++;

		switch(cip->op)
		{
		case XOP_NOOP:
			break;
		case XOP_EXIT:
			{
				co_main->aFrame[0].sp2=ewsl.ci0.nbx;
				co_main->aFrame.resize(1);
				co_this=co_main;
				co_last.reset(NULL);
			}
			while(Executor_stackframe_leave(ewsl,cip->p1)) cip=ewsl.ci0.nip++;
			return;
		case XOP1_NEG:
			pl1_call<pl_neg>::g(*this);
			break;
		case XOP1_BITWISE_NOT:
			pl1_call<pl_bw_not>::g(*this);
			break;
		case XOP1_NOT:
			ci1.nsp[0].reset(!pl_cast<bool>::g(ci1.nsp[0]));
			break;
		case XOP2_ADD:
			pl2_call<pl_add>::k(*this);
			break;
		case XOP2_SUB:
			pl2_call<pl_sub>::k(*this);;
			break;
		case XOP2_DOT_MUL:
			pl2_call<pl_arr_mul>::k(*this);;
			break;
		case XOP2_MUL:
			pl2_call<pl_mat_mul>::k(*this);;
			break;
		case XOP2_DOT_DIV:
			// int div int may return double if needed, so do NOT use fast version (pl2_call<...>::k)
			pl2_call<pl_arr_div>::g(*this);;
			break;
		case XOP2_DIV:
			// int div int may return double if needed, so do NOT use fast version (pl2_call<...>::k)
			pl2_call<pl_mat_div>::g(*this);;
			break;
		case XOP2_MOD:
			pl2_call<pl_mod>::k(*this);;
			break;
		case XOP2_DOT_POW:
			pl2_call<pl_arr_pow>::g(*this);;
			break;
		case XOP2_POW:
			pl2_call<pl_mat_pow>::g(*this);;
			break;
		case XOP2_BITWISE_AND:
			pl2_call<pl_bw_and>::k(*this);;
			break;
		case XOP2_BITWISE_OR:
			pl2_call<pl_bw_or>::k(*this);;
			break;
		case XOP2_BITWISE_XOR:
			pl2_call<pl_bw_xor>::k(*this);;
			break;
		case XOP2_SHIFT_L:
			pl2_call<pl_bw_shl>::k(*this);;
			break;
		case XOP2_SHIFT_R:
			pl2_call<pl_bw_shr>::k(*this);;
			break;
		case XOP2_AND:
			ci1.nsp[-1].reset<bool>(pl_cast<bool>::k(ci1.nsp[-1])&&pl_cast<bool>::k(ci1.nsp[0]));
			--ci1.nsp;
			break;
		case XOP2_OR:
			ci1.nsp[-1].reset<bool>(pl_cast<bool>::k(ci1.nsp[-1])||pl_cast<bool>::k(ci1.nsp[0]));
			--ci1.nsp;
			break;
		case XOP2_XOR:
			ci1.nsp[-1].reset<bool>(pl_cast<bool>::k(ci1.nsp[-1])!=pl_cast<bool>::k(ci1.nsp[0]));
			--ci1.nsp;
			break;
		case XOP2_LT:
			pl2_call<pl_lt>::k(*this);;
			break;
		case XOP2_GT:
			pl2_call<pl_gt>::k(*this);;
			break;
		case XOP2_LE:
			pl2_call<pl_le>::k(*this);;
			break;
		case XOP2_GE:
			pl2_call<pl_ge>::k(*this);;
			break;
		case XOP2_EQ:
			pl2_call<pl_eq>::g(*this);;
			break;
		case XOP2_SAME:
			ci1.nsp[-1].reset<bool>(ci1.nsp[-1].kptr() !=NULL && ci1.nsp[-1].kptr()==ci1.nsp[0].kptr());
			--ci1.nsp;
			break;
		case XOP2_NE:
			pl2_call<pl_ne>::g(*this);;
			break;
		case XOP2_CAT:
			ci1.nsp[-1].reset(pl_cast<String>::g(ci1.nsp[-1])+pl_cast<String>::g(ci1.nsp[0]));
			--ci1.nsp;
			break;
		case XOP_PUSH_NIL:
			for(int i=0;i<cip->p1;i++){push();}
			break;
		case XOP_PUSH_BOOL:
			push(cip->p1!=0);
			break;
		case XOP_PUSH_INT:
			push(cip->p1);
			break;
		case XOP_GET_STACK:
			push(ci1.nsp[cip->p1]);
			break;
		case XOP_GET_GGVAR:
			push(tb0.get(cip->p2).second);
			break;
		case XOP_GET_SBASE:
			push(ci1.nbp[cip->p1]);
			break;
		case XOP_GET_LLVAR:
			push(ci0.dat[cip->p1]);
			break;
		case XOP_GET_LOCAL:
			push(ci0.sp1[cip->p1]);
			break;
		case XOP_GET_TABLE:
			push(tb1[variant_handler<String>::raw(ci0.dat[cip->p1])]);
			break;
		case XOP_GET_INDEX_N:
			{
				arr_xt<Variant>* q=ci1.nsp[0].ptr< arr_xt<Variant> >();
				if(!q) kerror("invalid getindex_n");
				ci1.nsp[0]=(*q)[cip->p1];
			}
			break;
		case XOP_SET_INDEX_N:
			{
				arr_xt<Variant>* q=ci1.nsp[0].ptr< arr_xt<Variant> >();
				if(!q) kerror("invalid setindex_n");
				ewsl.popq();
				ewsl.popq((*q)[cip->p1]);
			}
			break;
		case XOP_SET_INDEX_N_REF:
			{
				arr_xt<Variant>* q=ci1.nsp[0].ptr< arr_xt<Variant> >();
				if(!q) kerror("invalid setindex_n");
				(*q)[cip->p1]=ci1.nsp[-1];
				ci1.nsp-=2;
			}
			break;
		case XOP_GET_INDEX:

			Executor_update_idx(ewsl,cip);
			ret=ci2.pfn->__getindex(ewsl,*ci2.idx);
			ci1.nbp[StackState1::SBASE_THIS].kptr(ci2.pfn.get());

			if(ret==CallableData::STACK_BALANCED){}
			else if(ret==1)
			{

			}
			else
			{
				kerror("invalid return value of getindex");
			}

			break;
		case XOP_GET_ARRAY:

			Executor_update_pmc(ewsl,cip->p1);
			ret=ci2.pfn->__getarray(ewsl,ci2.pmc);

			if(ret==CallableData::STACK_BALANCED){}
			else if(ret==1)
			{
				ci1.nsp=ci0.nbx+1;
			}
			else
			{
				kerror("invalid return value of getarray");
			}

			break;
		case XOP_SET_SBASE:
			popq(ci1.nbp[cip->p1]);
			break;
		case XOP_SET_LLVAR:
			popq(ci0.dat[cip->p1]);
			break;
		case XOP_SET_LOCAL:
			popq(ci0.sp1[cip->p1]);
			break;
		case XOP_SET_LOCAL_REF:
			popq_ref(ci0.sp1[cip->p1]);			
			break;
		case XOP_SET_TABLE:
			popq(tb1[variant_handler<String>::raw(ci0.dat[cip->p1])]);
			break;
		case XOP_SET_GGVAR:
			popq(const_cast<CG_GGVar&>(tb0).get(cip->p2).second);
			break;
		case XOP_SET_INDEX:

			Executor_update_idx(ewsl,cip);
			ret=ci2.pfn->__setindex(ewsl,*ci2.idx);

			if(ret==CallableData::STACK_BALANCED){}
			else if(ret==0)
			{

			}
			else
			{
				kerror("invalid return value of setindex");
			}

			break;
		case XOP_SET_ARRAY:

			Executor_update_pmc(ewsl,cip->p1);
			ret=ci2.pfn->__setarray(ewsl,ci2.pmc);

			if(ret==CallableData::STACK_BALANCED){}
			else if(ret==0)
			{
				ewsl.ci1.nsp=ewsl.ci0.nbx-1;
			}
			else
			{
				kerror("invalid return value of setarray");
			}

			break;
		case XOP_SADJ:
			ci1.nsp+=cip->p1;
			break;
		case XOP_JUMP:
			ci0.nip+=cip->p1;
			break;
		case XOP_JUMP_N:
			ci0.nip+=(*ci1.nsp--).get<int64_t>();
			break;
		case XOP_JUMP_0:
			if(!pl_cast<bool>::k((*ci1.nsp--)))
			{
				ci0.nip+=cip->p1;
			}
			break;
		case XOP_JUMP_1:
			if(pl_cast<bool>::k((*ci1.nsp--)))
			{
				ci0.nip+=cip->p1;
			}
			break;
		case XOP_INC:
			if(ci1.nsp[0].type()==type_flag<int64_t>::value)
			{
				++static_cast<CallableWrapT<int64_t>*>(ci1.nsp[0].kptr())->value;
				break;
			}
			if(ci1.nsp[0].type()==type_flag<double>::value)
			{
				++static_cast<CallableWrapT<double>*>(ci1.nsp[0].kptr())->value;
				break;
			}
			Exception::XBadCast();
			break;
		case XOP_DEC:
			if(ci1.nsp[0].type()==type_flag<int64_t>::value)
			{
				--static_cast<CallableWrapT<int64_t>*>(ci1.nsp[0].kptr())->value;
				break;
			}
			if(ci1.nsp[0].type()==type_flag<double>::value)
			{
				--static_cast<CallableWrapT<double>*>(ci1.nsp[0].kptr())->value;
				break;
			}
			Exception::XBadCast();
			break;

		case XOP_CALL:
			ci1.nbp[ci1.SBASE_THIS].clear();
		case XOP_THIS_CALL:

			Executor_update_pmc(ewsl,cip->p1);
			ci0.kep=cip->p2;

			ret=ci2.pfn->__fun_call(ewsl,ci2.pmc);

			if(ret==CallableData::STACK_BALANCED){}
			else if(ret>=0)
			{
				if(ci0.kep>=0)
				{
					for(int i=ret+1;i<=ci0.kep;i++)
					{
						ci0.nbx[i].clear();
					}
					ci1.nsp=ci0.nbx+ci0.kep;
				}
				else
				{
					ci1.nsp=ci0.nbx+ret;
					push(ret-1);
				}
			}
			else
			{
				kerror("invalid return value of fun_call");
			}

			break;
		case XOP_TRY_ENTER:
			{
				CallableCoroutine::tagCatch cit;
				cit.ci0=ci0;
				cit.ci0.nip+=cip->p1;
				cit.ci0.nbx=ci1.nsp;
				cit.frm=ewsl.co_this->aFrame.size();
				ewsl.co_this->aCatch.push_back(cit);
			}
			break;
		case XOP_TRY_LEAVE:
			ewsl.co_this->aCatch.pop_back();
			break;
		case XOP_TRY_THROW:
			{

				Variant* sp1=ci0.nbx;
				int pmc=cip->p1;

				if(pmc<0)
				{
					pmc=(*ci1.nsp--).get<int64_t>()-pmc;
				}

				if(ewsl.co_this->aCatch.empty())
				{
					throw ExceptionEwsl(sp1,pmc);
				}

				CallableCoroutine::tagCatch& cit(ewsl.co_this->aCatch.back());
				ci0=cit.ci0;
				ci1.nsp=ci0.nbx;

				ewsl.co_this->aFrame.resize(cit.frm);
				ewsl.co_this->aCatch.pop_back();

				ci0.nbx=ci1.nsp;
				for(int i=0;i<pmc;i++)
				{
					(*++ci1.nsp)=*++sp1;
				}

				(*++ci1.nsp).ref<int64_t>()=pmc-1;
				break;
			}

		case XOP_HALT:
			if(!Executor_stackframe_leave(ewsl,cip->p1))
			{
				return;
			}
			break;
		case XOP_FAIL:
			kerror("xop_fail");
		default:
			kerror("unknown instruction");
		}
	}
}

void Executor::_vm_check_ret(int ret)
{

	if(ret==ci0.kep)
	{
		return;
	}
	else if(ret==CallableData::STACK_BALANCED)
	{
		if(ci0.nip!=NULL)
		{
			_vm_run1(ci0.kep);
		}
	}
	else
	{
		kerror("invalid return value");
	}
}

void Executor::get_array(int pmc)
{
	Executor& ewsl(*this);

	Executor_update_pmc(*this,pmc);
	ci0.kep=1;

	int ret=ci2.pfn->__getarray(ewsl,ci2.pmc);

	_vm_check_ret(ret);
}

void Executor::set_array(int pmc)
{
	Executor& ewsl(*this);

	Executor_update_pmc(*this,pmc);
	ci0.kep=0;

	int ret=ci2.pfn->__setarray(ewsl,ci2.pmc);
	_vm_check_ret(ret);
}

int Executor::callx_raw(int n,int k)
{
	Executor_update_pmc(*this,n);
	ci0.kep=k;
	int ret=ci2.pfn->__fun_call(*this,ci2.pmc);
	return ret;
}

bool Executor::callx(int pmc,int kep)
{
	Executor& ewsl(*this);
	try
	{
		Executor_update_pmc(*this,pmc);

		DataPtrT<ObjectData> obj(ci2.pfn);
		ci0.kep=kep;

		int ret=ci2.pfn->__fun_call(ewsl,ci2.pmc);

		if(ret>=0)
		{
			if(ci0.kep>=0)
			{
				for(int i=ret+1;i<=ci0.kep;i++)
				{
					ewsl.ci0.nbx[i].clear();
				}
				ewsl.ci1.nsp=ewsl.ci0.nbx+ci0.kep;
			}
			else
			{
				ewsl.ci1.nsp=ewsl.ci0.nbx+ret;
				ewsl.push(ret-1);
			}
		}
		else if(ret==CallableData::STACK_BALANCED)
		{
			if(ci0.nip!=NULL)
			{
				_vm_run1(kep);
			}
		}
		else
		{
			ewsl.kerror("invalid return value of fun_call");
		}

		return true;
	}
	catch(std::exception& e)
	{
		if (ci0.nip)
		{
			EW_ASSERT(co_this->aCatch.empty());

			try
			{
				_vm_handle_exception(e);
			}
			catch (std::exception &e)
			{
				this_logger().LogError(e.what());
			}
		}
		else
		{
			this_logger().LogError(e.what());
		}
		return false;
	}

}

int Executor::move_return_values(int n)
{
	EW_ASSERT(n>=0);
	Variant* bp1=ci1.nsp-n;
	for(int i=1;i<=n;i++)
	{
		ci0.nbx[i]=bp1[i];
	}
	return n;
}


void Executor::set_index(const String& s)
{
	CallableData* pfn=ci1.nsp[0].kptr();
	if(!pfn)
	{
		kerror("invalid object");
	}

	ci0.kep=0;
	int ret=pfn->__setindex(*this,s);
	_vm_check_ret(ret);
}

void Executor::get_index(const String& s)
{
	CallableData* pfn=ci1.nsp[0].kptr();
	if(!pfn)
	{
		kerror("invalid object");
	}

	ci1.nbp[StackState1::SBASE_THIS].kptr(pfn);
	ci0.kep=1;

	int ret=pfn->__getindex(*this,s);

	_vm_check_ret(ret);

}

EW_LEAVE
