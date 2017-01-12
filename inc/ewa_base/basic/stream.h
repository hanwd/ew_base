#ifndef __H_EW_STREAM__
#define __H_EW_STREAM__

#include "ewa_base/config.h"
#include "ewa_base/basic/object.h"
#include "ewa_base/basic/platform.h"
#include "ewa_base/basic/bitflags.h"
#include "ewa_base/basic/pointer.h"


EW_ENTER

class DLLIMPEXP_EWA_BASE IStreamData : public ObjectData
{
public:

	virtual int send(const char* p,size_t n);
	virtual int recv(char* p,size_t n);

	virtual int64_t seekg(int64_t p,int t);
	virtual int64_t tellg();
	virtual int64_t sizeg();

	virtual int64_t seekp(int64_t p,int t);
	virtual int64_t tellp();
	virtual int64_t sizep();

	virtual void close(){}

	virtual void flush(){}

	bool send_all(const char* p,size_t n);
	bool recv_all(char* p,size_t n);

	BitFlags flags;

	static IStreamData* invalid_stream_data;

};


class DLLIMPEXP_EWA_BASE File;
class DLLIMPEXP_EWA_BASE Socket;

class DLLIMPEXP_EWA_BASE Stream : public Object
{
public:

	Stream();

	bool open(const String& fp,int fg=FLAG_FILE_RD);
	bool connect(const String& ip,int port);

	void assign(File& file);
	void assign(Socket& socket);

	void assign(DataPtrT<IStreamData> p);
	void assign_reader(DataPtrT<IStreamData> p1);
	void assign_writer(DataPtrT<IStreamData> p2);

	int64_t seekg(int64_t p,int t){return hReader->seekg(p,t);}
	int64_t tellg(){return hReader->tellg();}
	int64_t sizeg(){return hReader->sizeg();}

	int64_t seekp(int64_t p,int t){return hWriter->seekp(p,t);}
	int64_t tellp(){return hWriter->tellp();}
	int64_t sizep(){return hWriter->sizep();}

	int send(const char* buf,size_t len){return hWriter->send(buf,len);}
	int recv(char* buf,size_t len){return hReader->recv(buf,len);}

	bool send_all(const char* buf,size_t len){return hWriter->send_all(buf,len);}
	bool recv_all(char* buf,size_t len){return hReader->recv_all(buf,len);}

	DataPtrT<IStreamData> get_reader(){return hReader;}
	DataPtrT<IStreamData> get_writer(){return hWriter;}

	void close();

	bool write_to_file(const String& fp,int flag=0);
	bool write_to_buffer(StringBuffer<char>& sb);
	bool write_to_stream(Stream& stream){return write_to_writer(stream.hWriter);}
	bool write_to_writer(DataPtrT<IStreamData> wr);

	bool read_from_file(const String& fp);
	bool read_from_buffer(StringBuffer<char>& sb);
	bool read_from_stream(Stream& stream){return read_from_reader(stream.hReader);}
	bool read_from_reader(DataPtrT<IStreamData> rd);

	void Serialize(Serializer& ar);

protected:
	
	DataPtrT<IStreamData> hReader;
	DataPtrT<IStreamData> hWriter;
};

template<> class hash_t<Stream>
{
public:
	inline uint32_t operator()(const Stream&){return -1;}
};

inline bool operator==(const Stream& lhs,const Stream& rhs){return &lhs==&rhs;}

DEFINE_OBJECT_NAME(Stream,"Stream");

EW_LEAVE

#endif
