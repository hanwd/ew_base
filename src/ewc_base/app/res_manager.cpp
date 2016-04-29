
#include "ewc_base/app/res_manager.h"
#include <wx/imaglist.h>

EW_ENTER



ResManager::ResManager()
{
	SetData(new CallableTableEx());
	m_refData->value["Icons"].kptr((CallableData*)icons.GetData());

	bmp_missing.LoadFile("wxART_MISSING_IMAGE");
}

ResManager::~ResManager()
{

}

ResManager& ResManager::current()
{
	static ResManager gInstance;
	return gInstance;
}



ObjectImageList::~ObjectImageList()
{
	for(bst_map<size_t,wxImageList*>::iterator it=imgLists.begin();it!=imgLists.end();++it)
	{
		delete (*it).second;
	}
}

int ObjectImageList::get(const String& s)
{
	return aImages.find2(s);
}

	
wxImageList* ObjectImageList::get_imglist(size_t sz)
{
	imgList_ptr &pimg(imgLists[sz]);
	if(!pimg)
	{

		ResManager& kr(ResManager::current());

		pimg=new wxImageList(sz,sz,true,0);		
		for(size_t i=0;i<aImages.size();i++)
		{
			const wxBitmap& bmp(kr.icons.get_bitmap(aImages.get(i), sz));
			if (bmp.IsOk())
			{
				pimg->Add(bmp);
			}
			else
			{
				pimg->Add(kr.bmp_missing.GetBitmap(sz));
			}
		}
	}
	return pimg;
}

ObjectImageList& ObjectImageList::current()
{
	static ObjectImageList gInstance;
	return gInstance;
}

EW_LEAVE
