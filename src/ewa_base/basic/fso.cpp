#include "ewa_base/basic/fso.h"
#include "ewa_base/util/strlib.h"

EW_ENTER


int FSObject::FileExists(const String& fp_)
{
	String fp=string_replace(fp_,"\\","/");
	arr_1t<String> fd=string_split(fp,"/");
	String filename=fd.back();
	String folder=string_join(fd.begin(),fd.begin()+fd.size()-1,"/");

	arr_1t<FileItem> items=FindFilesEx(folder,filename);
	if(items.size()!=1)
	{
		return 0;
	}

	return items.back().flags.get(FileItem::IS_FOLDER)?2:1;
}

bool FSObject::DownloadToFile(const String& fp,const String& localfile,int flag)
{
	Stream stream=Download(fp);
	return stream.write_to_file(localfile,flag);
}

bool FSObject::UploadFromFile(const String& localfile,const String& fp,int flag)
{
	Stream stream=Upload(fp,flag);
	return stream.read_from_file(localfile);
}

bool FSObject::DownloadToBuffer(const String& fp,StringBuffer<char>& sb)
{
	Stream stream=Download(fp);
	return stream.write_to_buffer(sb);
}

bool FSObject::UploadFromBuffer(StringBuffer<char>& sb,const String& fp,int flag)
{
	Stream stream=Upload(fp,flag);
	return stream.read_from_buffer(sb);
}

arr_1t<FileItem> FSObject::FindFilesEx(const String& fp,const String& pattern)
{
	arr_1t<FileItem> items;
	FindFiles(fp,items,pattern);
	return items;
}


EW_LEAVE
