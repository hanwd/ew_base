#include "ewa_base/scripting.h"
#include "ewa_base/basic/fso.h"

EW_ENTER


#ifdef EW_WINDOWS
static void PushFindItem(arr_1t<FileItem>& files,WIN32_FIND_DATAW& p)
{
	FileItem item;
	item.filename=p.cFileName;
	item.filesize=(uint64_t(p.nFileSizeHigh)<<32)|p.nFileSizeLow;
	item.flags.set(FileItem::IS_FOLDER,(p.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)!=0);
	if(item.filename=="."||item.filename=="..") return;
	files.push_back(item);
}
#endif

bool FSLocal::FindFiles(const String& folder,arr_1t<FileItem>& items,const String& pattern)
{

#ifdef EW_WINDOWS

	String folder_pattern;
	if(pattern!="")
	{
		folder_pattern = System::AdjustPath(folder,true)+pattern;
	}
	else
	{
		folder_pattern = folder;
	}

	FILE_ATTRIBUTE_DIRECTORY;
	WIN32_FIND_DATAW p;
	HANDLE h = FindFirstFileW(IConv::to_wide(folder_pattern).c_str(), &p);

	if (h != INVALID_HANDLE_VALUE)
	{
		PushFindItem(items,p);
		while (FindNextFileW(h, &p))
		{
			PushFindItem(items,p);
		}
	}

	return !items.empty();

#endif

}

bool FSLocal::DownloadToFile(const String& fp,const String& localfile,int flag)
{
	if(flag&FLAG_FILE_APPEND)
	{
		return FSObject::DownloadToFile(fp,localfile,flag);
	}
	else
	{
		return ::CopyFileW(IConv::to_wide(fp).c_str(),IConv::to_wide(localfile).c_str(),(flag&FLAG_FILE_CR)?TRUE:FALSE)==TRUE;
	}

}

bool FSLocal::UploadFromFile(const String& localfile,const String& fp,int flag)
{
	if(flag&FLAG_FILE_APPEND)
	{
		return FSObject::UploadFromFile(localfile,fp,flag);
	}
	else
	{
		return ::CopyFileW(IConv::to_wide(localfile).c_str(),IConv::to_wide(fp).c_str(),(flag&FLAG_FILE_CR)?TRUE:FALSE)==TRUE;
	}
}

bool FSLocal::DownloadToBuffer(const String& fp,StringBuffer<char>& sb)
{
	return sb.load(fp,FILE_TYPE_BINARY);
}

bool FSLocal::UploadFromBuffer(StringBuffer<char>& sb,const String& fp,int flag)
{
	return sb.save(fp,FILE_TYPE_BINARY|flag);
}

bool FSLocal::Mkdir(const String& fp)
{

	BOOL ret=CreateDirectoryW(IConv::to_wide(fp).c_str(),NULL);
	return ret!=FALSE;

}

bool FSLocal::Rmdir(const String& fp,int flag)
{

	//BOOL ret=::RemoveDirectoryW(IConv::to_wide(fp).c_str());
	//return ret!=FALSE;

	if(fp.find('\"')>=0)
	{
		System::SetLastError("Invalid Directory");
		return false;
	}

	String dir(fp);

	StringBuffer<char> sb;
	if(flag==0)
	{
		System::Execute("cmd /c rmdir \""+dir+"\"",sb);
	}
	else
	{
		System::Execute("cmd /c rmdir /S /Q \""+dir+"\"",sb);
	}


	if(!sb.empty())
	{
		System::SetLastError(IConv::from_gbk(sb.c_str()));
		return false;
	}

	return true;
}

bool FSLocal::Rename(const String& fp_old,const String& fp_new,int flag)
{
	DWORD dwflag=MOVEFILE_COPY_ALLOWED ;
	if(flag!=0)	dwflag|=MOVEFILE_REPLACE_EXISTING;

	BOOL ret=::MoveFileExW(IConv::to_wide(fp_old).c_str(),IConv::to_wide(fp_new).c_str(),dwflag);
	return ret!=0;

	//int bRet=::rename(IConv::to_ansi(fp_old).c_str(),IConv::to_ansi(fp_new).c_str());
	//return bRet==0;
}

bool FSLocal::Remove(const String& fp)
{
	BOOL ret=DeleteFileW(IConv::to_wide(fp).c_str());
	return ret!=FALSE;

	//int bRet=::remove(IConv::to_ansi(fp).c_str());
	//return bRet==0;
}

Stream FSLocal::Download(const String& fp)
{
	Stream stream;
	stream.open(fp,FLAG_FILE_RD);
	return stream;
}

Stream FSLocal::Upload(const String& fp,int flag)
{
	Stream stream;
	stream.open(fp,FLAG_FILE_WC|flag);
	return stream;
}


FSLocal& FSLocal::current()
{
	static FSLocal g_instance;
	return g_instance;
}

EW_LEAVE
