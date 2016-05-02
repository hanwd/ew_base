#include "ewa_base/scripting/codegen_generator.h"
#include "ewa_base/scripting/parser_nodes.h"
#include "ewa_base/scripting/executor.h"
#include "ewa_base/scripting/callable_table.h"

EW_ENTER

TNodeVisitorCG_Generator::TNodeVisitorCG_Generator(CodeGen& g)
:TNodeVisitorCG(g)
,lvis(*this)
,rvis(*this)
{

}


TNodeVisitorCG_GeneratorUnit::TNodeVisitorCG_GeneratorUnit(TNodeVisitorCG_Generator& cg_)
	:TNodeVisitorCG(cg_.cgen),lvis(cg_.lvis),rvis(cg_.rvis)
{

}


VisReturnType TNodeVisitorCG_GeneratorUnit::visit_arr(TNode_expression_arr* node,VisExtraParam visp,int xop)
{
	cgen.back().cg_local.enter2(node->tScope);

	if(node->exp_flag.empty())
	{
		VisReturnType n=node->exp_list->accept(rvis,-1);
		node->tbl->accept(rvis,1);
		cgen.emit(xop,n);
	}
	else
	{

		int n=(int)node->exp_list->aList.size();
		int f=1;

		node->tbl->accept(rvis,1);
		cgen.emit_setref("#arr");

		for(int i=0;i<n;i++)
		{
			int k=(i+1==n?-1:+1);

			if(node->exp_flag[i]!=0)
			{
				cgen.emit_getvar("#arr");
				cgen.emit_push(n==1?-1:i);
				cgen.emit_getsys("#arr_getindex");
				cgen.emit_call(2,2);
				cgen.emit_setvar("end");
				cgen.emit_setvar("begin");

				f=node->exp_list->aList[i]->accept(rvis,k);
			}
			else
			{
				f=node->exp_list->aList[i]->accept(rvis,k);
			}
		}	

		cgen.emit_getvar("#arr");
		cgen.emit(xop,f<0?-n:+n);
	}

	
	cgen.back().cg_local.leave2();

	return 1;

}

void TNodeVisitorCG_Generator::parse(TNode* node)
{
	node->accept(rvis,1);
}



void TNodeVisitorCG_Generator::kerror(const String& s)
{
	Exception::XError(String::Format("CodeGen:Generator:%s",s));
}

EW_LEAVE

