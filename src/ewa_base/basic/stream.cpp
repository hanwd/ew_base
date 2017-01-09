#include "ewa_base/basic/stream.h"
#include "ewa_base/basic/system.h"
#include "ewa_base/basic/stringbuffer.h"
#include "ewa_base/basic/object_ex.h"
#include "ewa_base/net/socket.h"
//#include "ewa_base/serialization/serializer_stream.h"
//#include "ewa_base/serialization/serializer_socket.h"

EW_ENTER

IStreamData* get_invalid_stream_data()
{
	static DataPtrT<IStreamData> g_instance(new IStreamData);
	g_instance->flags.add(FLAG_READER_FAILBIT|FLAG_WRITER_FAILBIT);
	return g_instance.get();
}

IStreamData* IStreamData::invalid_stream_data(get_invalid_stream_data());

int IStreamData::send(const char* p,size_t n)
{
	return -1;
}

int IStreamData::recv(char* p,size_t n)
{
	return -1;
}

int64_t IStreamData::seekg(int64_t p,int t)
{
	return -1;
}

int64_t IStreamData::tellg()
{
	return -1;
}

int64_t IStreamData::sizeg()
{
	int64_t sz=tellg();
	if(sz<0) return -1;
	int64_t z1=seekg(0,SEEKTYPE_END);
	if(z1<0) return -1;
	int64_t z2=seekg(sz,SEEKTYPE_BEG);
	if(z1!=z2)
	{
		System::LogError("seekg failed");
	}
	return z1;
}

int64_t IStreamData::seekp(int64_t p,int t)
{
	return -1;
}

int64_t IStreamData::tellp()
{
	int64_t sz=tellp();
	if(sz<0) return -1;
	int64_t z1=seekp(0,SEEKTYPE_END);
	if(z1<0) return -1;
	int64_t z2=seekp(sz,SEEKTYPE_BEG);
	if(z1!=z2)
	{
		System::LogError("seekp failed");
	}
	return z1;
}

int64_t IStreamData::sizep()
{
	return -1;
}

bool IStreamData::send_all(const char* data,size_t size)
{
	while(size>0)
	{
		int n=send(data,size);
		if(n==size) return true;
		if(n<=0)
		{
			return false;
		}
		size-=n;
		data+=n;
	}
	return true;

}

bool IStreamData::recv_all(char* data,size_t size)
{
	while(size>0)
	{
		int n=recv(data,size);
		if(n==size) return true;
		if(n<=0)
		{
			return false;
		}
		size-=n;
		data+=n;
	}
	return true;
}


class DLLIMPEXP_EWA_BASE IStreamFile : public IStreamData
{
public:

	IStreamFile(){}
	IStreamFile(File fp):file(fp){}
	IStreamFile(const String& fp,int fg=FLAG_FILE_RD):file(fp,fg){}

	File file;

	int64_t seekg(int64_t p,int t){return file.seek(p,t);}
	int64_t seekp(int64_t p,int t){return file.seek(p,t);}

	void flush(){file.flush();}

	int64_t tellg(){return file.tell();}
	int64_t tellp(){return file.tell();}

	int send(const char* data,size_t size)
	{
		return file.write(data,size);
	}
	int recv(char* data,size_t size)
	{
		return file.read(data,size);
	}

	void close(){file.close();}
	
};

class DLLIMPEXP_EWA_BASE IStreamSocket : public IStreamData
{
public:

	IStreamSocket(){}
	IStreamSocket(Socket& sock):socket(sock){}

	void close()
	{
		socket.close();
	}

	int recv(char* data,size_t size)
	{
		return socket.recv(data,size);
	}

	int send(const char* data,int size)
	{
		return socket.send(data,size);
	}


	Socket socket;

};


inline void set_invalid_stream_error()
{
#ifdef EW_WINDOWS
	::SetLastError(6);
#endif

	errno=5;
}


//int SerializerReader::recv(char* data,int size)
//{
//	flags.add(FLAG_READER_FAILBIT);
//	set_invalid_stream_error();
//	return -1;
//}
//
//int SerializerWriter::send(const char* data,int size)
//{
//	flags.add(FLAG_WRITER_FAILBIT);
//	set_invalid_stream_error();
//	return -1;
//}


//
//bool SerializerReader::recv_all(char* data,int size)
//{
//	while(size>0)
//	{
//		int n=recv(data,size);
//		if(n==size) return true;
//		if(n<=0)
//		{
//			return false;
//		}
//		size-=n;
//		data+=n;
//	}
//	return true;
//}


//bool SerializerWriter::send_all(const char* data,int size)
//{
//	while(size>0)
//	{
//		int n=send(data,size);
//		if(n==size) return true;
//		if(n<=0)
//		{
//			return false;
//		}
//		size-=n;
//		data+=n;
//	}
//	return true;
//}


bool Stream::write_to_file(const String& fp,int flag)
{
	if(!hReader)
	{
		 set_invalid_stream_error();;
		return false;
	}

	File file;
	if(!file.open(fp,FLAG_FILE_WC|flag))
	{
		return false;
	}

	char buffer[1024*32];
	while(1)
	{
		int rc=hReader.get()->recv(buffer,sizeof(buffer));
		if(rc>0)
		{
			if(file.write(buffer,rc)!=rc)
			{
				return false;
			}
		}
		else if(rc==0)
		{
			return true;
		}
		else
		{
			break;
		}
	}

	return false;
}

bool Stream::write_to_writer(DataPtrT<IStreamData> wr)
{
	if(!hReader)
	{
		 set_invalid_stream_error();;
		return false;
	}

	char buffer[1024*32];
	while(1)
	{
		int rc=hReader.get()->recv(buffer,sizeof(buffer));
		if(rc>0)
		{
			if(!wr->send_all(buffer,rc))
			{
				return false;
			}
		}
		else if(rc==0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	return false;
}

bool Stream::write_to_buffer(StringBuffer<char>& sb)
{
	if(!hReader)
	{
		 set_invalid_stream_error();;
		return false;
	}

	try
	{
		char buffer[1024*32];
		while(1)
		{
			int rc=hReader.get()->recv(buffer,sizeof(buffer));
			if(rc>0)
			{
				sb.append(buffer,rc);
			}
			else if(rc==0)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
	}
	catch(std::exception& e)
	{
		EW_UNUSED(e);
	}

	return false;
}

bool Stream::read_from_file(const String& fp)
{

	if(!hWriter)
	{
		set_invalid_stream_error();
		return false;
	}

	File file;
	if(!file.open(fp,FLAG_FILE_RD))
	{
		return false;
	}

	char buffer[1024*32];
	while(1)
	{
		int rc=file.read(buffer,sizeof(buffer));
		if(rc>0)
		{
			if(!hWriter->send_all(buffer,rc))
			{
				return false;
			}
		}
		else if(rc==0)
		{
			return true;
		}
		else
		{
			break;
		}

	}

	return false;
}

bool Stream::read_from_reader(DataPtrT<IStreamData> rd)
{
	if(!hWriter)
	{
		 set_invalid_stream_error();;
		return false;
	}

	char buffer[1024*32];
	while(1)
	{
		int rc=rd->recv(buffer,sizeof(buffer));
		if(rc>0)
		{
			if(!hWriter->send_all(buffer,rc))
			{
				return false;
			}
		}
		else if(rc==0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	return false;
}

bool Stream::read_from_buffer(StringBuffer<char>& sb)
{
	if(!hWriter)
	{
		 set_invalid_stream_error();;
		return false;
	}

	return hWriter->send_all(sb.data(),sb.size());
}


bool Stream::open(const String& fp,int fg)
{
	File file;
	if(!file.open(fp,fg))
	{
		close();
		return false;
	}

	assign(file);
	return true;
}

bool Stream::connect(const String& ip,int port)
{
	Socket socket;
	if(!socket.connect(ip,port))
	{
		return false;
	}

	assign(socket);
	return true;
}

void Stream::assign(File& file)
{
	assign(new IStreamFile(file));
}

void Stream::assign(Socket& socket)
{
	assign(new IStreamSocket(socket));
}


Stream::Stream()
{
	assign(NULL);
}

void Stream::assign(DataPtrT<IStreamData> p)
{
	assign_reader(p);
	assign_writer(p);
}


void Stream::assign_reader(DataPtrT<IStreamData> p)
{
	hReader=p?p:IStreamData::invalid_stream_data;
}

void Stream::assign_writer(DataPtrT<IStreamData> p)
{
	hWriter=p?p:IStreamData::invalid_stream_data;
}



void Stream::close()
{
	hReader->close();
	hWriter->close();
}

void Stream::Serialize(Serializer& ar)
{
	Exception::XError("stream cannot be serialized!");
	//ar.errstr("stream cannot be serialized!");
}



EW_LEAVE
