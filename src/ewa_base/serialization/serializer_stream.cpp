#include "ewa_base/serialization/serializer_stream.h"

EW_ENTER



int SerializerFile::send(const char* data,int size)
{
	return file.write(data,size);
}

int SerializerFile::recv(char* data,int size)
{
	return file.read(data,size);
}

void SerializerFile::close()
{
	file.close();
}

bool SerializerFile::good()
{
	return file.good();
}

EW_LEAVE
