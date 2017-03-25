
#include "ewa_base/basic/string.h"

EW_ENTER



class SystemData : public NonCopyableAndNonNewable
{
public:

	SystemData();

	void init_system_info();
	void update_system_info();
	void update_language();

	int64_t m_nMemTotalPhys;
	int64_t m_nMemAvailPhys;
	int64_t m_nMemTotalVirtual;
	int64_t m_nMemAvailVirtual;
	int64_t m_nMemTotalPageFile;
	int64_t m_nMemAvailPageFile;

	int m_nCpuCount;
	int m_nCacheLine;
	int m_nPageSize;

	String m_sAppdataPath;
	String m_sResdataPath;


	int nLangId;
	String sLanguage;




	static SystemData& current()
	{
		static SystemData gInstance;
		return gInstance;
	}
};

EW_LEAVE
