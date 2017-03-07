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

bool FSObject::DownloadToBuffer(const String& fp,StringBuffer<char>& sb,int type)
{
	Stream stream=Download(fp);
	return stream.write_to_buffer(sb,type);
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

Stream FSObject::Open(const String& fp, int flag)
{
	if ((flag&FLAG_FILE_RW) ==FLAG_FILE_RD)
	{
		return Download(fp);
	}
	if ((flag&FLAG_FILE_RW) ==FLAG_FILE_WR)
	{
		return Upload(fp,flag);
	}
	return Stream();
}

class FSDispatch
{
public:


	FSDispatch(const String& fp_) :fs(NULL)
	{
		fs = &FSLocal::current();
		fp = fp_;

		int dd = fp.find(':');
		if (dd <= 1) return;
		
		String pre = fp.substr(0, dd);

		if (fp.c_str()[dd + 1] == '/')
		{
			++dd;
		}

		String sub = fp.substr(dd + 1);
		if (pre == "res")
		{
			fp = System::MakeResdataPath(sub);
		}
		else if (pre == "app")
		{
			fp = System::MakeAppdataPath(sub);
		}
		else
		{
			fs = NULL;
		}
	}


	FSObject* fs;
	String fp;

	operator bool()
	{
		return fs != NULL;
	}

	FSObject* operator->()
	{
		return fs;
	}
};

class FSObjectInfo : public ObjectInfo
{
public:

	FSObjectInfo(const String&) :ObjectInfo()
	{
		System::SetResdataPath("");
		System::SetAppdataPath("");
	}
};


class FSObjectReal : public FSObject
{
public:

	virtual Stream Download(const String& fp)
	{
		FSDispatch fsoptr(fp);
		return fsoptr?fsoptr->Download(fsoptr.fp):Stream();
	}

	virtual Stream Upload(const String& fp, int flag = FLAG_FILE_CR)
	{
		FSDispatch fsoptr(fp);
		return fsoptr ? fsoptr->Upload(fsoptr.fp) : Stream();
	}

	virtual bool FindFiles(const String& fp, arr_1t<FileItem>& items, const String& pattern)
	{
		FSDispatch fsoptr(fp);
		return fsoptr ? fsoptr->FindFiles(fsoptr.fp,items,pattern) : false;
	}
	
	virtual bool Mkdir(const String& fp)
	{
		FSDispatch fsoptr(fp);
		return fsoptr ? fsoptr->Mkdir(fsoptr.fp) : false;
	}

	virtual bool Rmdir(const String& fp, int flag)
	{
		FSDispatch fsoptr(fp);
		return fsoptr ? fsoptr->Rmdir(fsoptr.fp,flag) : false;
	}

	virtual bool Remove(const String& fp)
	{
		FSDispatch fsoptr(fp);
		return fsoptr ? fsoptr->Remove(fsoptr.fp) : false;
	}

	virtual bool Rename(const String& fp_old, const String& fp_new, int flag)
	{
		FSDispatch fsoptr(fp_old);
		return fsoptr ? fsoptr->Rename(fsoptr.fp,fp_new,flag) : false;
	}

	virtual Stream Open(const String& fp, int flag)
	{
		FSDispatch fsoptr(fp);
		return fsoptr ? fsoptr->Open(fsoptr.fp,flag) : Stream();
	}

	DECLARE_OBJECT_INFO_NO_CREATE(FSObjectReal,FSObjectInfo)

};

IMPLEMENT_OBJECT_INFO(FSObjectReal, FSObjectInfo);


FSObject& FSObject::current()
{
	return detail::StaticInstance<FSObjectReal>::current();
}

EW_LEAVE
