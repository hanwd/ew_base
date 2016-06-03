
#ifndef __H_EW_SERIALIZER_STREAM__
#define __H_EW_SERIALIZER_STREAM__

#include "ewa_base/serialization/serializer.h"
#include "ewa_base/basic/file.h"
#include <fstream>

EW_ENTER

class DLLIMPEXP_EWA_BASE SerializerFile : public SerializerDuplex
{
public:

	SerializerFile(){}
	SerializerFile(File fp):file(fp){}
	SerializerFile(const String& fp,int fg=FLAG_FILE_RD):file(fp,fg){}

	File file;

	int64_t seek(int64_t p,int t)
	{
		return file.Seek(p,t);
	}

	void flush(){file.Flush();}

	int64_t tell(){return file.Tell();}

	int send(const char* data,int size);
	int recv(char* data,int size);

	void close();
	bool good();
	
};


EW_LEAVE
#endif

