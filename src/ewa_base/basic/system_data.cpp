#include "system_data.h"

#ifdef EW_MSVC
#include "windows.h"
#endif

EW_ENTER


#ifdef EW_MSVC

void SystemData::init_system_info()
{

	SYSTEM_INFO info;
	GetSystemInfo(&info);
	m_nPageSize=info.dwPageSize;
	m_nCpuCount=info.dwNumberOfProcessors;

	size_t line_size = 0;
	DWORD buffer_size = 0;
	DWORD i = 0;
	SYSTEM_LOGICAL_PROCESSOR_INFORMATION * buffer = 0;

	GetLogicalProcessorInformation(0, &buffer_size);

	EW_ASSERT(buffer_size<4096);

	char _local_buffer[4096];
	buffer = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION *) _local_buffer;

	GetLogicalProcessorInformation(&buffer[0], &buffer_size);

	for (i = 0; i != buffer_size / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION); ++i)
	{
		if (buffer[i].Relationship == RelationCache && buffer[i].Cache.Level == 1)
		{
			line_size = buffer[i].Cache.LineSize;
			break;
		}
	}

	m_nCacheLine=(int)line_size;


}

#elif defined(EW_WINDOWS)

#else

void SystemData::init_system_info()
{

	m_nPageSize=sysconf(_SC_PAGE_SIZE);;
	m_nCpuCount=sysconf(_SC_NPROCESSORS_ONLN);
	m_nCacheLine=-1;

	FILE * p = 0;
	p = fopen("/sys/devices/system/cpu/cpu0/cache/index0/coherency_line_size", "r");

	if (p)
	{
		fscanf(p, "%d", &m_nCacheLine);
		fclose(p);
	}

}

#endif


#ifdef EW_MSVC

void SystemData::update_system_info()
{
	MEMORYSTATUSEX ms;
	ms.dwLength=sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&ms);

	m_nMemTotalPhys=ms.ullTotalPhys;
	m_nMemAvailPhys=ms.ullAvailPhys;
	m_nMemTotalVirtual=ms.ullTotalVirtual;
	m_nMemAvailVirtual=ms.ullAvailVirtual;
	m_nMemTotalPageFile=ms.ullTotalPageFile;
	m_nMemAvailPageFile=ms.ullAvailPageFile;

}

#else

void SystemData::update_system_info()
{

}

#endif


EW_LEAVE
