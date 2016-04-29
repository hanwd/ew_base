#ifndef __H_EW_UI_APP_RES_MANAGER__
#define __H_EW_UI_APP_RES_MANAGER__

#include "ewc_base/config.h"
#include "ewc_base/app/res_bitmap.h"

EW_ENTER


class DLLIMPEXP_EWC_BASE ResManager : public ObjectT<CallableTableEx,NonCopyable>
{
public:
	typedef ObjectT<CallableTableEx,NonCopyable> basetype;

	static ResManager& current();

	BitmapGroupHolder icons;

	BitmapHolder bmp_missing;
	

protected:
	ResManager();
	~ResManager();
};


class DLLIMPEXP_EWC_BASE ObjectImageList : public NonCopyable
{
public:

	int get(const String& s);
	wxImageList* get_imglist(size_t sz);

	static ObjectImageList& current();

protected:
	~ObjectImageList();
	typedef wxImageList* imgList_ptr;
	bst_map<size_t,imgList_ptr> imgLists;
	indexer_set<String> aImages;
};



EW_LEAVE
#endif
