
#ifndef __H_EW_IPC_SHM__
#define __H_EW_IPC_SHM__

#include "ewa_base/config.h"
#include "ewa_base/basic/platform.h"
#include "ewa_base/basic/string.h"
#include "ewa_base/basic/file.h"
#include "ewa_base/basic/bitflags.h"
#include "ewa_base/basic/pointer.h"

EW_ENTER

template<typename P1>
class KO_Handle_shm
{
public:

	KO_Handle_shm()
	{
		m_pAddr=NULL;
		m_nSize=0;
	}

	char* m_pAddr;
	size_t m_nSize;
	String m_sName;
	KO_Handle<P1> m_pHandle;
};


class DLLIMPEXP_EWA_BASE SharedMem : private NonCopyable
{
public:

	SharedMem();
	~SharedMem();

	typedef KO_Handle_shm<KO_Policy_handle> impl_type;

	// get the size of the shared memory.
	inline size_t size() const
	{
		return impl.m_nSize;
	}

	char* data()
	{
		return impl.m_pAddr;
	}
	const char* data() const
	{
		return impl.m_pAddr;
	}
	// Create/Open shared memory with given name and given size.
	bool Create(const String& name_,size_t size_,int flag_=FLAG_FILE_RW);
	bool Open(const String& name_,size_t size_,int flag_=FLAG_FILE_RW);

	// mapping a file to memory, zero size_ means using the real file size, if not zero, the file will be truncated to size_.
	// if need to create a new file, size_ must be specified and FLAG_CR flag must be provided.
	bool OpenFile(const String& file_,size_t size_=0,int flag_=FLAG_FILE_RW);

	// just allocate memory, not mapping to any file.
	bool Alloc(size_t size,int flag_=FLAG_FILE_RW);

	// Close shared memory.
	void Close();

protected:
	impl_type impl;
};



class DLLIMPEXP_EWA_BASE IShmMemory : public NonCopyable
{
public:

	class IShmHeader
	{
	public:
		uint64_t size;
		AtomicInt32 state;
		BitFlags flags;
	};

	~IShmMemory();

protected:

	bool Create(const String& name,size_t sz);

	char* allocate(size_t s,size_t al=16);

	void Close();

	LitePtrT<IShmHeader> pHeader;

	SharedMem mem;
	size_t m_nShift;
};



EW_LEAVE
#endif
