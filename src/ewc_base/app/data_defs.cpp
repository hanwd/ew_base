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


const String& IFileNameData::GetFilename()
{
	return m_sFilename;
}

void IFileNameData::SetFilename(const String& s)
{
	m_sFilename=s;
}

const String& IFileNameData::GetTempfile()
{
	if(m_sTempfile=="")
	{
		m_sTempfile=m_sFilename+".temp";
	}
	return m_sTempfile;
}

EW_LEAVE
