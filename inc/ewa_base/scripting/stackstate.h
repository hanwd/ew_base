#ifndef __EW_SCRIPTING_STACKSTATE_H__
#define __EW_SCRIPTING_STACKSTATE_H__


#include "ewa_base/scripting/callable_code.h"

EW_ENTER

class StackState0
{
public:

	XopInst* nip;		//pc
	Variant* dat;		//llvar base

	Variant* sp1;		//stackframe base
	Variant* sp2;		//stackframe top
	Variant* nbx;		//function param base	
	
	intptr_t kep;		//keep return_value_count, -1 for keep all

	DataPtrT<CallableCode> frm;

	int tcc;

	StackState0();

};

class StackState1
{
public:

	Variant* nbp;	//stack base pointer
	Variant* nsp;	//stack pointer
	Variant* end;	//stack end


	enum
	{
		SBASE_TEMP,
		SBASE_THIS,
		SBASE_META,
		SBASE_CWD,
		SBASE_STATE,
		SBASE_KEEP=16,
	};


};

class StackState2
{
public:

	DataPtrT<CallableData> pfn;	//self
	intptr_t pmc;				//function param count
	const String* idx;			//string index

	XopInst xop;

};

EW_LEAVE
#endif
