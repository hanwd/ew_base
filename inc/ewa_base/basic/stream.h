#ifndef __H_EW_STREAM__
#define __H_EW_STREAM__

#include "ewa_base/config.h"
#include "ewa_base/basic/object.h"
#include "ewa_base/basic/platform.h"


EW_ENTER

enum
{
	SEEKPOS_BEG,
	SEEKPOS_CUR,
	SEEKPOS_END
};

class DLLIMPEXP_EWA_BASE StreamData : public ObjectData
{
public:

	enum
	{
		FLAG_FAILBIT=1<<0,
	};

	virtual int32_t Read(char* buf,size_t len);
	virtual int32_t Write(const char* buf,size_t len);

	virtual void Flush();
	virtual void Close();

	virtual int64_t Size();
	virtual int64_t Seek(int64_t pos,int t);

	virtual int64_t Tell();
	virtual void Rewind();

	BitFlags flags;

};

class DLLIMPEXP_EWA_BASE Stream : public Object
{
public:
	Stream();
	Stream(StreamData* streamdata);

	virtual int32_t Read(char* buf,size_t len);
	virtual int32_t Write(const char* buf,size_t len);

	virtual void Flush();

	virtual void Close();

	virtual int64_t Size();
	virtual int64_t Seek(int64_t pos,int t);

	virtual int64_t Tell();
	virtual void Rewind();

	bool Good();

	void swap(Stream& other);

	bool Open(const String& filename_,int op=FileAccess::FLAG_RD);

protected:
	DataPtrT<StreamData> m_refData;
};



class DLLIMPEXP_EWA_BASE StreamDataHandle : public StreamData
{
public:
	StreamDataHandle();
	StreamDataHandle(KO_Handle<KO_Policy_handle> impl);

	virtual int32_t Read(char* buf,size_t len);
	virtual int32_t Write(const char* buf,size_t len);

	void Close();
	void Flush();

	void native_handle(KO_Handle<KO_Policy_handle>::const_reference v);

	KO_Handle<KO_Policy_handle>::type native_handle();

protected:
	KO_Handle<KO_Policy_handle> m_pHandle;
};



EW_LEAVE

#endif
