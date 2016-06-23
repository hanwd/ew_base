#ifndef __H_EW_BASIC_FILE__
#define __H_EW_BASIC_FILE__

#include "ewa_base/basic/string.h"
#include "ewa_base/basic/bitflags.h"
#include "ewa_base/basic/platform.h"

EW_ENTER

class DLLIMPEXP_EWA_BASE FileItem
{
public:
	enum
	{
		IS_FOLDER=1<<0,
	};
	String filename;
	uint64_t filesize;
	BitFlags flags;
};

class DLLIMPEXP_EWA_BASE File
{
public:


	File();
	File(const String& filename_,int op=FLAG_FILE_RD);
	~File();

	bool open(const String& filename_,int op=FLAG_FILE_RD);
	void close();

	int64_t size();

	int32_t read(char* buf,size_t len);
	int32_t write(const char* buf,size_t len);

	int64_t seek(int64_t pos,int t);
	int64_t tell();

	void rewind();

	void flush();
	void truncate(size_t size_);

	bool eof();


	void swap(File& o)
	{
		impl.swap(o.impl);
		std::swap(flags,o.flags);
	}

	KO_Handle<KO_Policy_handle>::type native_handle()
	{
		return impl.get();
	}

	bool ok()
	{
		return impl.ok();
	}

	bool good()
	{
		return !flags.get(FLAG_READER_FAILBIT|FLAG_WRITER_FAILBIT);
	}

private:

	KO_Handle<KO_Policy_handle> impl;
	BitFlags flags;

};


EW_LEAVE
#endif
