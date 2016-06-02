#ifndef __H_EW_STREAM__
#define __H_EW_STREAM__

#include "ewa_base/config.h"
#include "ewa_base/basic/object.h"
#include "ewa_base/basic/platform.h"
#include "ewa_base/basic/bitflags.h"
#include "ewa_base/basic/pointer.h"

EW_ENTER

enum
{
	SEEKTYPE_BEG,
	SEEKTYPE_CUR,
	SEEKTYPE_END
};

class DLLIMPEXP_EWA_BASE StreamData : public ObjectData
{
public:

	enum
	{
		FLAG_READ_FAIL_BIT=1<<0,
		FLAG_WRITE_FAIL_BIT=1<<1,
		FLAG_FAIL_BITS=FLAG_READ_FAIL_BIT|FLAG_WRITE_FAIL_BIT,
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


class DLLIMPEXP_EWA_BASE Stream : public ObjectT<StreamData>
{
public:
	typedef ObjectT<StreamData> basetype;

	Stream();
	Stream(StreamData* p);

	virtual int32_t Read(char* buf,size_t len);
	virtual int32_t Write(const char* buf,size_t len);

	virtual void Flush();

	virtual void Close();

	virtual int64_t Size();
	virtual int64_t Seek(int64_t pos,int t);

	virtual int64_t Tell();
	virtual void Rewind();

	bool Good();

	bool Open(const String& filename_,int op=FileAccess::FLAG_RD);

	bool SaveToFile(const String& filename_);
	bool LoadFromFile(const String& filename_);

	bool SaveToBuffer(StringBuffer<char>& sb);
	bool LoadFromBuffer(StringBuffer<char>& sb);

};



class DLLIMPEXP_EWA_BASE StreamDataHandle : public StreamData
{
public:

	StreamDataHandle();
	StreamDataHandle(KO_Policy_handle::type v);
	~StreamDataHandle();
	StreamDataHandle(const StreamDataHandle&);
	StreamDataHandle& operator=(const StreamDataHandle&);

	virtual int32_t Read(char* buf,size_t len);
	virtual int32_t Write(const char* buf,size_t len);

	void Close();
	void Flush();

	void native_handle(KO_Policy_handle::type v);
	KO_Policy_handle::type native_handle();

protected:
	KO_Policy_handle::type m_pHandle;

};



EW_LEAVE

#endif
