#ifndef __H_EW_BASIC_FSO__
#define __H_EW_BASIC_FSO__

#include "ewa_base/basic/string.h"
#include "ewa_base/basic/stream.h"

EW_ENTER


class DLLIMPEXP_EWA_BASE FSObject : public Object
{
public:

	virtual bool DownloadToFile(const String& fp,const String& localfile,int flag=0);

	virtual bool UploadFromFile(const String& fp,const String& localfile,int flag=0);

	virtual bool DownloadToBuffer(const String& fp,StringBuffer<char>& sb);

	virtual bool UploadFromBuffer(const String& fp,StringBuffer<char>& sb,int flag);

	virtual bool FindFiles(const String& fp,arr_1t<FileItem>& items,const String& pattern="")=0;

	virtual arr_1t<FileItem> FindFilesEx(const String& fp,const String& pattern="");

	virtual int FileExists(const String& fp);

	virtual bool Mkdir(const String& fp)=0;
	virtual bool Rmdir(const String& fp)=0;

	virtual bool Rename(const String& fp_old,const String& fp_new)=0;

	virtual bool Remove(const String& fp_del)=0;

	virtual Stream Download(const String& fp)=0;

	virtual Stream Upload(const String& fp,int flag=FLAG_FILE_CR)=0;

};

class DLLIMPEXP_EWA_BASE FSLocal : public FSObject
{
public:

	virtual bool FindFiles(const String& fp,arr_1t<FileItem>& items,const String& pattern);

	virtual bool DownloadToFile(const String& fp,const String& localfile,int flag);

	virtual bool UploadFromFile(const String& fp,const String& localfile,int flag);

	virtual bool DownloadToBuffer(const String& fp,StringBuffer<char>& sb);

	virtual bool UploadFromBuffer(const String& fp,StringBuffer<char>& sb,int flag);

	virtual bool Mkdir(const String& fp);

	virtual bool Rmdir(const String& fp);

	virtual bool Rename(const String& fp_old,const String& fp_new);

	virtual bool Remove(const String& fp_del);

	virtual Stream Download(const String& fp);

	virtual Stream Upload(const String& fp,int flag);

	static FSLocal& current();

};

EW_LEAVE
#endif
