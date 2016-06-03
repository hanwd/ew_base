#include "ewa_base/serialization/serializer_stream.h"

EW_ENTER



int SerializerFile::send(const char* data,int size)
{
	return file.Write(data,size);
}

int SerializerFile::recv(char* data,int size)
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

EW_LEAVE
