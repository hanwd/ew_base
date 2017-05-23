#include "ewa_base/basic/stream.h"
#include "ewa_base/basic/system.h"
#include "ewa_base/basic/stringbuffer.h"
#include "ewa_base/basic/object_ex.h"
#include "ewa_base/basic/misc.h"
#include "ewa_base/net/socket.h"

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
	return seekg(0,SEEKTYPE_CUR);
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
	return seekp(0,SEEKTYPE_CUR);
}

int64_t IStreamData::sizep()
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


int64_t IStreamData2::seek(int64_t p,int t)
{
	return -1;
}

int64_t IStreamData2::tell()
{
	return seek(0,SEEKTYPE_CUR);
}

int64_t IStreamData2::size()
{
	int64_t sz=tell();
	if(sz<0) return -1;
	int64_t z1=seek(0,SEEKTYPE_END);
	if(z1<0) return -1;
	int64_t z2=seek(sz,SEEKTYPE_BEG);
	if(z1!=z2)
	{
		System::LogError("seekp failed");
	}
	return z1;
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


class DLLIMPEXP_EWA_BASE IStreamFile : public IStreamData2
{
public:

	IStreamFile(){}
	IStreamFile(File fp):file(fp){}
	IStreamFile(const String& fp,int fg=FLAG_FILE_RD):file(fp,fg){}

	File file;

	int64_t seek(int64_t p,int t){return file.seek(p,t);}
	int64_t tell(){return file.tell();}
	int64_t size(){return file.size();}

	void flush(){file.flush();}




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



class DLLIMPEXP_EWA_BASE IStreamMemory : public IStreamData
{
public:

	IStreamMemory(){}

	MemoryBuffer<char> mb;

	int send(const char* data,size_t size)
	{
		return mb.send(data,size);
	}
	int recv(char* data,size_t size)
	{
		return mb.recv(data,size);
	}

	void close()
	{
		mb.clear();
		mb.shrink();
	}
	
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

	char buffer[1024*8];
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

	char buffer[1024*8];
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

bool Stream::write_to_buffer(StringBuffer<char>& sb,int type)
{
	if(!reader_ok())
	{
		 set_invalid_stream_error();;
		return false;
	}

	if (type == FILE_TYPE_BINARY)
	{
		try
		{
			char buffer[1024*8];

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
			return false;
		}
	}


	unsigned char bom[4] = { 1, 1, 1, 1 };
	
	int n = recv((char*)bom, 4);

	if (n<2)
	{
		if (n < 0) return false;
		sb.append(bom, n);
		return true;
	}


	if ((bom[0] == 0xFE && bom[1] == 0xFF) || (bom[0] == 0xFF && bom[1] == 0xFE))
	{

		StringBuffer<char> kb;
		kb.append(bom, n - 2);
		if (!write_to_buffer(kb, FILE_TYPE_BINARY))
		{
			return false;
		}

		if (kb.size() % 2 != 0)
		{
			return false;
		}

		size_t nz = kb.size() >> 1;

		uint16_t tag = *reinterpret_cast<uint16_t*>(bom);
		if (tag == 0xFFFE) //BE
		{
			char *pc = (char*)kb.data();
			for (size_t i = 0; i<nz; i += 2)
			{
				std::swap(pc[i], pc[i + 1]);
			}
		}

		StringBuffer<char> zb;
		if (!IConv::unicode_to_utf8(zb, (uint16_t*)kb.data(), nz))
		{
			return false;
		}

		if (sb.empty())
		{
			sb.swap(zb);
		}
		else
		{
			sb << zb;
		}
		return true;
	}


	if (n>=3 && bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF) // UTF8
	{
		sb.append(bom, n - 3);
		return write_to_buffer(sb, FILE_TYPE_BINARY);
	}


	if (n==4 && ((bom[0] == 0xFF && bom[1] == 0xFE && bom[2] == 0 && bom[3] == 0) || (bom[0] == 0 && bom[1] == 0 && bom[2] == 0xFE && bom[3] == 0xFF)))
	{

		StringBuffer<char> kb;
		if (!write_to_buffer(kb, FILE_TYPE_BINARY))
		{
			return false;
		}

		if (kb.size() % 4 != 0)
		{
			return false;
		}

		size_t nz = kb.size() >> 2;

		uint32_t tag = *reinterpret_cast<uint32_t*>(bom);
		if (tag == 0xFFFE0000) //BE
		{
			char* pc = (char*)kb.data();
			for (size_t i = 0; i<nz; i += 4)
			{
				std::swap(pc[i + 0], pc[i + 3]);
				std::swap(pc[i + 1], pc[i + 2]);
			}
		}

		StringBuffer<char> zb;
		if (!IConv::unicode_to_utf8(zb, (uint32_t*)kb.data(), nz))
		{
			return false;
		}

		if (sb.empty())
		{
			sb.swap(zb);
		}
		else
		{
			sb << zb;
		}
		return true;
	
	}
	else
	{
		StringBuffer<char> kb;
		kb.append(bom, n);
		if (!write_to_buffer(kb, FILE_TYPE_BINARY))
		{
			return false;
		}

		if (!IConv::ensure_utf8(kb))
		{
			return false;
		}	

		if (sb.empty())
		{
			sb.swap(kb);
		}
		else
		{
			sb << kb;
		}

		return true;
	}
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

	char buffer[1024*8];
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

	char buffer[1024*8];
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


bool Stream::openuri(const String& fp, int fg)
{
	(*this) = FSObject::current().Open(fp, fg);
	return reader_ok() || writer_ok();
}

bool Stream::openfile(const String& fp,int fg)
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

Stream::Stream(File& file)
{
	assign(file);
}

Stream::Stream(Socket& socket)
{
	assign(socket);
}

Stream::Stream(DataPtrT<IStreamData> p)
{
	assign(p);
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

bool Stream::reader_ok()
{
	return !hReader->flags.get(FLAG_READER_FAILBIT);
}

bool Stream::writer_ok()
{
	return !hWriter->flags.get(FLAG_WRITER_FAILBIT);
}



void Stream::Serialize(SerializerHelper)
{
	Exception::XError("stream cannot be serialized!");
	//ar.errstr("stream cannot be serialized!");
}



EW_LEAVE
