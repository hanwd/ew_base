#include "ewc_base/app/data_defs.h"

EW_ENTER

IDat_wndconfig::IDat_wndconfig()
{
	bmp_menu_size=16;
	bmp_tool_size=24;
}


IDat_search::IDat_search()
{

}

IDat_search& IDat_search::current()
{
	static IDat_search gInstance;
	return gInstance;
}


IFileNameData* IFileNameData::DoClone(ObjectCloneState&)
{
	return new IFileNameData(*this);
}


String IFileNameData::GetFilename()
{
	return m_sFilename;
}

void IFileNameData::SetFilename(const String& s)
{
	m_sFilename=s;
}

String IFileNameData::GetTempfile()
{
	if(m_sTempfile=="")
	{
		m_sTempfile=m_sFilename+".temp";
	}
	return m_sTempfile;
}

String IFileNameData::GetExts()
{
	return m_sExts;
}

void IFileNameData::SetExts(const String& exts)
{
	m_sExts = exts;
	flags.add(1);
}

bool IFileNameData::IsSavable()
{
	return flags.get(1);
}

EW_LEAVE
