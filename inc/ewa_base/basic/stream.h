#ifndef __H_EW_STREAM__
#define __H_EW_STREAM__

#include "ewa_base/config.h"
#include "ewa_base/basic/object.h"
#include "ewa_base/basic/platform.h"
#include "ewa_base/basic/bitflags.h"
#include "ewa_base/basic/pointer.h"


EW_ENTER


class DLLIMPEXP_EWA_BASE SerializerReader;
class DLLIMPEXP_EWA_BASE SerializerWriter;

class DLLIMPEXP_EWA_BASE Serializer : private NonCopyable
{
protected:

	Serializer(int t);

public:

	struct internal_head{void Serialize(Serializer& ar){ar.handle_head();}};
	struct internal_tail{void Serialize(Serializer& ar){ar.handle_tail();}};

	static internal_head head;
	static internal_tail tail;

	enum
	{
		READER=1,
		WRITER=2,
	};

	enum
	{
		PTRTAG_CLS		=-3,		//class pointer (but NOT Object)
		PTRTAG_POD		=-2,		//POD pointer
		PTRTAG_OBJ		=-1,		//Object and its derived class pointer
		PTRTAG_NIL		= 0,		//NULL pointer
		PTRTAG_CACHED	= 1,
	};

	enum
	{

		FLAG_OFFSET_TABLE	=1<<8,
		FLAG_ENCODING_UTF8	=1<<9,
	};

	virtual ~Serializer() {}

	bool is_reader(){return type==READER;}
	bool is_writer(){return type==WRITER;}

	virtual void errstr(const String& msg);
	virtual void errver();

	virtual bool good() const{return !flags.get(FLAG_READER_FAILBIT|FLAG_WRITER_FAILBIT);}
	virtual void flush(){}

	virtual Serializer& tag(char ch)=0;
	virtual Serializer& tag(const char* msg)=0;

	virtual int local_version(int v)=0;
	virtual int size_count(int n)=0;

	virtual String object_type(const String& name)=0;

	int global_version();
	void global_version(int v);

	virtual void close() {}

	BitFlags flags;

	virtual SerializerWriter& writer();
	virtual SerializerReader& reader();

	virtual Serializer& handle_head()=0;
	virtual Serializer& handle_tail()=0;

protected:
	const int32_t type;
	int32_t gver;

};

class DLLIMPEXP_EWA_BASE CachedObjectManager : public ObjectData
{
public:

	CachedObjectManager(){clear();}

	class PtrOffset
	{
	public:
		int64_t lo;
		int64_t hi;
	};

	class PtrLoader
	{
	public:

		enum
		{
			IS_LOADED		=1<<0,
		};

		DataPtrT<ObjectData> m_ptr;
		BitFlags flags;
	};


	void clear();

	void save_ptr(SerializerWriter& ar,ObjectData* ptr,bool write_index=false);
	ObjectData* load_ptr(SerializerReader& ar,int pos);

	void handle_pending(SerializerWriter& ar);
	void handle_pending(SerializerReader& ar,bool use_seek=false);

	ObjectData* read_object(SerializerReader& ar,int val);

	Object* create(const String& name);

	arr_1t<int32_t> pendings;
	arr_1t<PtrOffset> aOffset;
	arr_1t<PtrLoader> aLoader;
	indexer_map<ObjectData*,int32_t> aObject;

};



class DLLIMPEXP_EWA_BASE SerializerEx : public Serializer
{
protected:

	SerializerEx(int t);

public:

	CachedObjectManager& cached_objects()
	{
		if(!p_cached_objects)
		{
			p_cached_objects.reset(new CachedObjectManager);
		}
		return *p_cached_objects;
	}

protected:
	DataPtrT<CachedObjectManager> p_cached_objects;

};



class DLLIMPEXP_EWA_BASE SerializerReader : public SerializerEx
{
public:
	SerializerReader():SerializerEx(READER) {}

	virtual int64_t seekg(int64_t p,int t){return-1;}
	virtual int64_t tellg(){return seekg(0,SEEKTYPE_CUR);}

	virtual int64_t sizeg();

	virtual int recv(char* data,int size);

	bool recv_all(char* data,int size);
	void recv_checked(char* data,int size);

	virtual String object_type(const String& name);

	virtual SerializerReader& tag(char ch);
	virtual SerializerReader& tag(const char* msg);

	virtual int local_version(int v);
	virtual int size_count(int);

	virtual bool good(){return !flags.get(FLAG_READER_FAILBIT);}


	ObjectData* read_object(int val);

	virtual SerializerReader& handle_head();
	virtual SerializerReader& handle_tail();

};

class DLLIMPEXP_EWA_BASE SerializerWriter : public SerializerEx
{
public:

	SerializerWriter():SerializerEx(WRITER) {}

	virtual int64_t seekp(int64_t p,int t){return-1;}
	virtual int64_t tellp(){return seekp(0,SEEKTYPE_CUR);}
	virtual int64_t sizep();

	virtual int send(const char* data,int size);

	bool send_all(const char* data,int size);
	void send_checked(const char* data,int size);

	String object_type(const String& name);

	virtual SerializerWriter& tag(char ch);
	virtual SerializerWriter& tag(const char* msg);

	virtual int local_version(int v);
	virtual int size_count(int);

	virtual bool good(){return !flags.get(FLAG_WRITER_FAILBIT);}

	virtual SerializerWriter& handle_head();
	virtual SerializerWriter& handle_tail();

};

class DLLIMPEXP_EWA_BASE SerializerDuplex : protected SerializerReader, protected SerializerWriter
{
public:
	SerializerWriter& writer(){return *this;}
	SerializerReader& reader(){return *this;}

	virtual int64_t seek(int64_t p,int t){return-1;}
	virtual int64_t tell(){return seek(0,SEEKTYPE_CUR);}
	virtual int64_t size(){return seek(0,SEEKTYPE_END);}

	virtual int64_t seekg(int64_t p,int t){return seek(p,t);}
	virtual int64_t tellg(){return tell();}
	virtual int64_t sizeg(){return size();}

	virtual int64_t seekp(int64_t p,int t){return seek(p,t);}
	virtual int64_t tellp(){return tell();}
	virtual int64_t sizep(){return size();}

	static SerializerDuplex& invalid_duplex_serializer();

};



class DLLIMPEXP_EWA_BASE File;
class DLLIMPEXP_EWA_BASE Socket;

class DLLIMPEXP_EWA_BASE Stream
{
public:

	bool open(const String& fp,int fg=FLAG_FILE_RD);
	bool connect(const String& ip,int port);

	void assign(File& file);
	void assign(Socket& socket);

	void assign(SerializerDuplex* p);
	void assign(SerializerReader* p1,SerializerWriter* p2);

	int64_t seekg(int64_t p,int t);
	int64_t tellg();
	int64_t sizeg();

	int64_t seekp(int64_t p,int t);
	int64_t tellp();
	int64_t sizep();

	int send(const char* buf,int len);
	int recv(char* buf,int len);

	bool send_all(const char* buf,int len);
	bool recv_all(char* buf,int len);


	SerializerReader& reader();
	SerializerWriter& writer();

	void close();

	bool write_to_file(const String& fp,int flag=0);
	bool write_to_buffer(StringBuffer<char>& sb);
	bool write_to_stream(Stream& stream){return write_to_writer(stream.writer());}
	bool write_to_writer(SerializerWriter& wr);

	bool read_from_file(const String& fp);
	bool read_from_buffer(StringBuffer<char>& sb);
	bool read_from_stream(Stream& stream){return read_from_reader(stream.reader());}
	bool read_from_reader(SerializerReader& rd);

	void Serialize(Serializer& ar){ar.errstr("stream cannot be serialized!");}

protected:
	KO_Handle<KO_Policy_pointer<SerializerReader> > hReader;
	KO_Handle<KO_Policy_pointer<SerializerWriter> > hWriter;
};

template<> class hash_t<Stream>
{
public:
	inline uint32_t operator()(const Stream&)
	{
		return -1;
	}
};

inline bool operator==(const Stream& lhs,const Stream& rhs){return &lhs==&rhs;}

DEFINE_OBJECT_NAME(Stream,"Stream");

EW_LEAVE

#endif
