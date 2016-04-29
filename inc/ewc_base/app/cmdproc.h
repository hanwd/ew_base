#ifndef __H_EW_UI_APP_CMDPROC__
#define __H_EW_UI_APP_CMDPROC__

#include "ewc_base/config.h"

EW_ENTER
	
class DLLIMPEXP_EWC_BASE CmdProc : public ObjectData
{
public:

	enum
	{
		CP_CUT,
		CP_COPY,
		CP_PASTE,
		CP_DELETE,
		CP_CLEAR,
		CP_UNDO,
		CP_REDO,
		CP_DIRTY,
		CP_NEXT,
		CP_PREV,
		CP_PLAY,
		CP_STOP,
		CP_LOAD,
		CP_SAVE,
		CP_SAVEAS,
		CP_SAVE_TEMP,
		CP_SAVE_FILE,
		CP_FILEEXT,
		CP_SELECTALL,

		CP_FIND,
		CP_REPLACE,
		CP_REPLACEALL,

		CP_BUFFER,

	};

	virtual bool DoExecId(ICmdParam& cmd);
	virtual bool DoTestId(ICmdParam& cmd);

	bool ExecId(int id,String& p2);
	bool TestId(int id,String& p2);

	bool ExecId(int id,int p2=-1);
	bool TestId(int id,int p2=-1);

};

template<typename B>
class DLLIMPEXP_EWC_BASE CmdProcHolderT : public ObjectT<CmdProc,B>
{
public:

	virtual bool DoExecId(ICmdParam& cmd)
	{
		return m_refData && m_refData->DoExecId(cmd);
	}

	virtual bool DoTestId(ICmdParam& cmd)
	{
		return m_refData && m_refData->DoTestId(cmd);
	}

	bool ExecId(int id,int p2=-1)
	{
		return m_refData && m_refData->ExecId(id,p2);
	}

	bool TestId(int id,int p2=-1)
	{
		return m_refData && m_refData->TestId(id,p2);
	}

	bool ExecId(int id,String& p2)
	{
		return m_refData && m_refData->ExecId(id,p2);
	}

	bool TestId(int id,String& p2)
	{
		return m_refData && m_refData->TestId(id,p2);
	}
};




EW_LEAVE

#endif
