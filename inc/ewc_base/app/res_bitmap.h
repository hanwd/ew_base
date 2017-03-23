#ifndef __H_EW_UI_APP_RES_BITMAP__
#define __H_EW_UI_APP_RES_BITMAP__

#include "ewc_base/config.h"

EW_ENTER

class CallableBitmapGroup;
class BitmapBundle;

class DLLIMPEXP_EWC_BASE BitmapHolder
{
public:

	BitmapHolder();

	operator bool();

	CallableData* GetData(){return m_refData.get();}
	void SetData(CallableData* d);

	bool LoadFile(const String& s);
	bool LoadFile(const arr_1t<String>& s);

	void AddBitmap(const wxBitmap& bmp);


	bool update(wxIconBundle& iconbundle) const;

	const wxBitmap& GetBitmap(int w=-1) const;

	const BitmapBundle& GetBundle(int w,int t) const;


	void Serialize(Serializer& ar);

private:
	DataPtrT<CallableData> m_refData;

};

class DLLIMPEXP_EWC_BASE BitmapGroupHolder
{
public:

	BitmapGroupHolder();

	void set(const String& id,const arr_1t<String>& bmps);
	void beg(const String& id);
	void add(const String& bmp);
	void end();

	BitmapHolder get(const String& id);
	BitmapHolder get(int id);

	int find(const String& id);

	const wxBitmap& get_bitmap(const String& id,int w=-1);

	wxImageList* get_imglist(int w);

	CallableBitmapGroup* GetData();

	void Serialize(Serializer& ar);

private:
	DataPtrT<CallableTableEx> m_refData;
};

EW_LEAVE
#endif
