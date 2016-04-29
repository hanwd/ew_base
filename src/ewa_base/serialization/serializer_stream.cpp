#include "ewa_base/serialization/serializer_stream.h"

EW_ENTER


size_t SerializerFile::send(const char* data,size_t size)
{
	return file.Write(data,size);
}

size_t SerializerFile::recv(char* data,size_t size)
{
	return file.Read(data,size);
}

void SerializerFile::close()
{
	file.Close();
}

bool SerializerFile::good()
{
	return file.Good();
}

size_t SerializerReaderStream::recv(char* data,size_t size)
{
	fs.read(data,size);
	if(!fs.good())
	{
		errstr("read failed");
	}
	return fs.gcount();
}

size_t SerializerWriterStream::send(const char* data,size_t size)
{
	fs.write(data,size);
	if(!fs.good())
	{
		errstr("write failed");
	}
	return size;
}

EW_LEAVE
