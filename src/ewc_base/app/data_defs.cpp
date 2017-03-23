#include "ewc_base/app/data_defs.h"

EW_ENTER



IDat_search::IDat_search()
{

}

IDat_search& IDat_search::current()
{
	static IDat_search gInstance;
	return gInstance;
}

AppData::AppData()
{
	toolbitmap_size=24;
	menubitmap_size=16;
	toolbar_type="aui_bar";
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
