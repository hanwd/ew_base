#ifndef __H_EW_BASIC_FILE__
#define __H_EW_BASIC_FILE__

#include "ewa_base/basic/string.h"
#include "ewa_base/basic/stream.h"

EW_ENTER

class DLLIMPEXP_EWA_BASE File
{
public:


	class impl_type : public KO_Handle<KO_Policy_handle>
	{
	public:
		typedef KO_Handle<KO_Policy_handle> basetype;
		void swap(impl_type& o)
		{
			basetype::swap(o);
			std::swap(flags,o.flags);
		}
		BitFlags flags;
	};

	static bool Rename(const String& oldname_,const String& newname_);
	static bool Remove(const String& filename_);
	static bool Mkdir(const String& dir_);
	static bool Rmdir(const String& dir_);

	File();
	File(const String& filename_,int op=FileAccess::FLAG_RD);
	~File();

	bool Open(const String& filename_,int op=FileAccess::FLAG_RD);
	void Close();

	int64_t Size();

	int32_t Read(char* buf,size_t len);
	int32_t Write(const char* buf,size_t len);

	int64_t Seek(int64_t pos,int t);
	int64_t Tell();

	void Rewind();

	void Flush();
	void Truncate(size_t size_);

	bool Eof();

	void swap(impl_type& o){impl.swap(o);}
	void swap(File& o){impl.swap(o.impl);}

	impl_type::type native_handle()
	{
		return impl;
	}

	bool Ok()
	{
		return impl.ok();
	}

	bool Good()
	{
		return !impl.flags.get(StreamData::FLAG_FAIL_BITS);
	}

private:

	impl_type impl;

};


EW_LEAVE
#endif
