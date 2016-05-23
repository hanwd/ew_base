
#ifndef __H_EW_SERIALIZER_STREAM__
#define __H_EW_SERIALIZER_STREAM__

#include "ewa_base/serialization/serializer.h"
#include "ewa_base/basic/file.h"
#include <fstream>

EW_ENTER


class DLLIMPEXP_EWA_BASE SerializerFile : public SerializerDuplex
{
public:

	File file;

	bool seek(int64_t p){return -1!=file.Seek(p,SEEKTYPE_BEG);}
	int64_t tell(){return file.Tell();}

	size_t send(const char* data,size_t size);
	size_t recv(char* data,size_t size);

	void close();
	bool good();
	
};


class DLLIMPEXP_EWA_BASE SerializerReaderStream : public SerializerReader
{
public:
	SerializerReaderStream(std::istream& is):fs(is) {}

	bool good() const{return fs.good();}

protected:
	size_t recv(char* data,size_t size);
	std::istream& fs;
};


class DLLIMPEXP_EWA_BASE SerializerWriterStream : public SerializerWriter
{
public:

	SerializerWriterStream(std::ostream& os):fs(os) {}
	bool good() const{return fs.good();}

protected:
	size_t send(const char* data,size_t size);
	std::ostream& fs;
};

template<typename T>
class DLLIMPEXP_EWA_BASE StreamSerializer;

template<>
class DLLIMPEXP_EWA_BASE StreamSerializer<SerializerReader> : public SerializerReaderStream
{
public:
	StreamSerializer():SerializerReaderStream(fs) {}

	bool open(const String &file)
	{
		clear();
		fs.open(file.c_str(),std::ios::in|std::ios::binary);
		return fs.good();
	}

	void close()
	{
		fs.close();
	}

	std::ifstream fs;
};

template<>
class DLLIMPEXP_EWA_BASE StreamSerializer<SerializerWriter> : public SerializerWriterStream
{
public:

	StreamSerializer():SerializerWriterStream(fs) {}
	std::ofstream fs;

	bool open(const String &file)
	{
		clear();
		fs.open(file.c_str(),std::ios::out|std::ios::binary);
		return fs.good();
	}

	void close()
	{
		fs.close();
	}

};



EW_LEAVE
#endif

