
#ifndef __H_EW_UI_APP_CMDPROC_STK__
#define __H_EW_UI_APP_CMDPROC_STK__

#include "ewc_base/app/cmdproc.h"

EW_ENTER
	

class DLLIMPEXP_EWC_BASE CmdBase : public ObjectData
{
public:

	virtual bool Undo()=0;
	virtual bool Redo()=0;

	virtual const String& GetName() const =0;
};

class DLLIMPEXP_EWC_BASE CmdEx : public CmdBase
{
public:

	virtual bool Undo(){return true;}
	virtual bool Redo(){return true;}

	CmdEx(){}
	CmdEx(const String& s):m_sName(s){}

	virtual const String& GetName() const {return m_sName;}

	void SetName(const String& s){m_sName=s;}

protected:
	String m_sName;
};

class DLLIMPEXP_EWC_BASE CmdGroup : public CmdEx
{
public:

	virtual bool Undo();
	virtual bool Redo();

	void add(CmdBase* cmd){if(cmd) aCommands.push_back(cmd);}

protected:
	arr_1t<DataPtrT<CmdBase> > aCommands;
};

class DLLIMPEXP_EWC_BASE CmdProcStk : public CmdProc
{
public:

	virtual bool DoExecId(ICmdParam&);
	virtual bool DoTestId(ICmdParam&);

	virtual bool DoLoad(){return true;}
	virtual bool DoSave(const String&){return true;}


	CmdProcStk(){nLast=nIndex=0;}

	bool add_cmd(CmdBase* c);

protected:

	size_t nLast;
	size_t nIndex;
	arr_1t<DataPtrT<CmdBase> > aCommands;
	String m_sFilename;
};



EW_LEAVE
#endif
