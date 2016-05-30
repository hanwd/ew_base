#ifndef __H_EW_EWC_BASE_CONFIG__
#define __H_EW_EWC_BASE_CONFIG__

#include "ewa_base.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifdef EWC_BASE_DLL
#ifdef _MSC_VER
#define EWC_BASE_EXPORT __declspec(dllexport)
#define EWC_BASE_IMPORT __declspec(dllimport)
#else
#define EWC_BASE_EXPORT __attribute__((dllexport))
#define EWC_BASE_IMPORT __attribute__((dllimport))
#endif
#else
#define EWC_BASE_EXPORT
#define EWC_BASE_IMPORT
#endif

#ifdef EWC_BASE_BUILDING
#define DLLIMPEXP_EWC_BASE EWC_BASE_EXPORT
#else
#define DLLIMPEXP_EWC_BASE EWC_BASE_IMPORT
#endif


class wxString;
class wxWindow;
class wxBitmap;
class wxMenuItem;
class wxToolBarToolBase;
class wxMenu;
class wxToolBar;
class wxSizer;
class wxVariant;
class wxIconBundle;
class wxImageList;

EW_ENTER

typedef wxWindow* IWindowPtr;
typedef wxBitmap* IBitmapPtr;
typedef wxMenuItem* IMenuItemPtr;
typedef wxToolBarToolBase* IToolItemPtr;
typedef wxMenu* IMenuPtr;
typedef wxToolBar* ITbarPtr;
typedef wxSizer* ISizerPtr;


class WndManager;
class WndModel;
class WndUpdator;
class MvcBook;

class EvtListener;
class EvtBase;
class EvtCommand;
class EvtGroup;
class Validator;
class ValidatorTop;
class ValidatorGroup;

class EvtManager;
class ResManager;

class HeMenuImpl;
class HeTbarImpl;

class Plugin;
class PluginManager;

class ICmdParam;
class IStdParam;
class IWndParam;

class MvcView;
class MvcModel;
class MvcBookImpl;

class WndMaker;


template<typename T>
class WxImpl
{
public:	
	static T get(const wxVariant& v);
	static void set(wxVariant& v,const T& s);
};


class DLLIMPEXP_EWC_BASE IDefs
{
public:

	enum
	{

		BTN_MIN			=1<<0,
		BTN_CANCEL		=BTN_MIN<<0,
		BTN_YES			=BTN_MIN<<1,
		BTN_NO			=BTN_MIN<<2,
		BTN_OK			=BTN_MIN<<3,
		BTN_YES_TO_ALL	=BTN_MIN<<4,
		BTN_NO_TO_ALL	=BTN_MIN<<5,
		BTN_MAX			=BTN_MIN<<6,

		ICON_MIN		=BTN_MAX,
		ICON_MESSAGE	=ICON_MIN<<0,
		ICON_WARNING	=ICON_MIN<<1,
		ICON_ERROR		=ICON_MIN<<2,
		ICON_INFO		=ICON_MIN<<3,
		ICON_QUESTION	=ICON_MIN<<4,

		FD_MIN			=1<<0,
		FD_OPEN			=FD_MIN<<0,
		FD_SAVE			=FD_MIN<<1,
		FD_DIR			=FD_MIN<<2,
		FD_MULTI		=FD_MIN<<3,
		FD_MUST_EXIST	=FD_MIN<<4,

	};


	enum
	{
		ACTION_NOOP,
		ACTION_WINDOW_INIT,
		ACTION_WINDOW_BEFORE_SHOW,
		ACTION_WINDOW_BEFORE_HIDE,
		ACTION_WINDOW_AFTER_SHOW,
		ACTION_WINDOW_AFTER_HIDE,
		ACTION_WINDOW_FINI,

		ACTION_VALUE_CHANGING,
		ACTION_VALUE_CHANGED,
		ACTION_SELECTION_CHANGING,
		ACTION_SELECTION_CHANGED,
		ACTION_VALIDATE,
		ACTION_TEST_NOT_DIRTY,
		ACTION_TRANSFER2WINDOW,
		ACTION_TRANSFER2MODEL,

		ACTION_APPLY,
		//ACTION_ENTER,

		ACTION_ONE_PHASE,
		ACTION_UPDATECTRL,

		PHASE_PRECALL	=-1,
		PHASE_UPDATE	= 0,
		PHASE_POSTCALL	=+1,

	};


	enum
	{
		WND_ATTACH,
		WND_DETACH,
		WND_SHOW,
		WND_HIDE,
	};

	enum
	{
		CMD_HANDLED_OK=1<<0,
	};
	

	static const int IWND_EXPAND		=1<<0;
	static const int IWND_READONLY		=1<<1;
	static const int IWND_DISABLED		=1<<2;
	static const int IWND_MULTILINE		=1<<3;
	static const int IWND_WITHCOLON		=1<<4;
	static const int IWND_PAGEONLY		=1<<5;
	static const int IWND_HORIZONTAL	=1<<6;
	static const int IWND_VERTICAL		=1<<7;
	static const int IWND_NOHEADER		=1<<8;

	static const int IWND_TE_READONLY	=1<<9;
	static const int IWND_NO_RESIZABLE	=1<<10;
	static const int IWND_NO_CAPTION	=1<<11;
	static const int IWND_NO_SYS_MENU	=1<<12;
	static const int IWND_NO_CLOSE_BOX	=1<<13;

	static const int IWND_NO_CLOSE		=1<<14;
	static const int IWND_AUTO_FIT		=1<<15;
	static const int IWND_WINDOW_INITED	=1<<16;

	static const int ALIGN_LEFT		=1;
	static const int ALIGN_CENTER	=2;
	static const int ALIGN_RIGHT	=3;

	static const int ALIGN_TOP		=4;
	static const int ALIGN_MIDDLE	=5;
	static const int ALIGN_BOTTOM	=6;

	static const int NB_BOTTOM	=ALIGN_BOTTOM;
	static const int NB_TOP		=ALIGN_TOP;
	static const int NB_LEFT	=ALIGN_LEFT;
	static const int NB_RIGHT	=ALIGN_RIGHT;


};



EW_LEAVE

#endif


