#include "ewc_base/wnd/impl_wx/iwnd_stc.h"
#include "plugin_ewsl_vartable.h"

EW_ENTER

class EwslCaret;

class EwslIface : public Thread
{
public:

	MvcModel& Target;

	EwslIface(MvcModel& target);
	~EwslIface();	

	void OnExecuteDone(int param1);
	void PendingExecute(const String& cmd);

	void reset(EwslCaret* face);

	LitePtrT<EwslCaret> iface;

	bool busy;

	LitePtrT<DataModelTable> model;

protected:

	void DoExecuteCmds();

	arr_1t<String> m_aPendingCmd;
	arr_1t<DataPtrT<ObjectData> > m_aPendingRes;

	AtomicSpin spin;
	Executor ewsl;
	DataPtrT<ExecutorState> res;

};

EW_LEAVE
