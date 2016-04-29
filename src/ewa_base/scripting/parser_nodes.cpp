#include "ewa_base/scripting/parser_nodes.h"

EW_ENTER

CG_Variable::CG_Variable(int t,const String& s):type(t),name(s),target(-1),index(-1)
{

}

CG_Variable::~CG_Variable()
{

}

void CG_Block::insert(CG_Variable* q)
{
	EW_ASSERT(q!=NULL);

	q->next = next;
	next = q;
}

CG_Block::CG_Block() :CG_Variable(VAR_BREAKER)
{
	
}

CG_Block::~CG_Block()
{
	CG_Variable* q=next;
	CG_Variable* v=tail;
	while(q!=v)
	{
		CG_Variable *d=q;
		q=q->next;
		delete d;
	}
}


bool TNode_val_variant::update()
{
	if(token.type==TOK_INTEGER)
	{
		int64_t d(0);
		if(token.word.ToNumber(&d))
		{
			value.reset(d);
			return true;
		}
	}
	else if(token.type==TOK_DOUBLE)
	{
		double d;
		if(token.word.ToNumber(&d))
		{
			value.reset(d);
			return true;
		}
	}
	else if(token.type==TOK_STRING)
	{
		value.reset(token.word);
		return true;
	}
	else if(token.type==TOK_IMAGPART)
	{
		double d=0.0;
		if(token.word.ToNumber(&d))
		{
			value.reset(dcomplex(0,d));
			return true;
		}
	}

	return false;
}

bool TNode_val_boolean::update()
{
	value.reset<bool>(token.word.c_str()[0]=='t'||token.word.c_str()[0]=='T');
	return true;
}


bool TNode_val_function::add_capture(int d)
{
	for(size_t i=0;i<aCaptured.size();i++)
	{
		if(aCaptured[i]==d) return false;
	}
	aCaptured.push_back(d);
	return true;
}

EW_LEAVE
