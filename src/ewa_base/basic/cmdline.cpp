#include "ewa_base/basic/cmdline.h"
#include "ewa_base/logging/logger.h"
#include "ewa_base/basic/codecvt.h"
EW_ENTER

CmdLineParser::CmdLineParser()
{

}

void CmdLineParser::AddSwitch(const String& s)
{
	m_aCmdLineOptions[s].set(CMDLINE_SWITCH);
}

void CmdLineParser::AddOption(const String& s)
{
	m_aCmdLineOptions[s].set(CMDLINE_OPTION);
}

void CmdLineParser::AddOption(const String& s,const String& d)
{
	m_aCmdLineOptions[s].set(CMDLINE_OPTION);
	m_aOptionsDefault[s]=d;
}

void CmdLineParser::AddParam(int n)
{
	m_aCmdLineOptions[""].set(CMDLINE_PARAM,n);
}

bool CmdLineParser::parse(const arr_1t<String>& argv)
{
	typedef bst_map<String, OptionType>::iterator iterator;

	m_aParams.clear();
	m_aSwitchs.clear();
	m_aOptions=m_aOptionsDefault;

	int argc=argv.size();

	int nParamCount=m_aCmdLineOptions[""].tags;

	for (int i = 1; i < argc; i++)
	{
		const char* pa = argv[i].c_str();

		if (pa[0] == '-')
		{
			if (pa[1] == 0)
			{
				this_logger().PrintLn("empty option");
				return false;
			}

			String ps = pa + 1;

			iterator it = m_aCmdLineOptions.find(ps);
			if (it == m_aCmdLineOptions.end())
			{
				this_logger().PrintLn("unknown option or switch");
				return false;
			}

			OptionType opt((*it).second);

			if (opt.type == CMDLINE_SWITCH)
			{
				m_aSwitchs.insert(ps);
			}

			else if (opt.type == CMDLINE_OPTION)
			{
				if (++i == argc)
				{
					this_logger().PrintLn("option value expected");
					return false;
				}
				m_aOptions[ps] = argv[i];
				m_aSwitchs.insert(ps);
			}
			else
			{
				this_logger().PrintLn("unknown option type");
				return false;
			}
		}
		else
		{
			m_aParams.push_back(pa);


		}
	}

	if(nParamCount>=0 && (int)m_aParams.size()!=nParamCount)
	{
		if (nParamCount == 0)
		{
			this_logger().PrintLn("no_param_allown");
		}
		else
		{
			this_logger().PrintLn("invalid_param_count:%d needed",nParamCount);
		}

		return false;
	}

	return true;

}

bool CmdLineParser::parse(int argc, char** argv)
{
	arr_1t<String> vargv;

	if(argc<0)
	{
		this_logger().PrintLn("argc<0");
		return false;
	}
	else if(argc==0)
	{
		return true;
	}

	for(int i=0;i<argc;i++)
	{
		if(argv[i]==0)
		{
			this_logger().PrintLn("empty option");
			return false;
		}
		vargv.push_back(IConv::from_ansi(argv[i]));
	}

	return parse(vargv);
}

bool CmdLineParser::Found(const String& s)
{
	return m_aSwitchs.find(s) != m_aSwitchs.end();
}

bool CmdLineParser::Found(const String& s, String& v)
{
	bst_map<String, String>::iterator it = m_aOptions.find(s);
	if (it == m_aOptions.end()) return false;
	v = (*it).second;
	return true;
}

bool CmdLineParser::Found(const String& s, int32_t& v)
{
	String tmp;
	if (!Found(s, tmp)) return false;
	return tmp.ToNumber(&v);
}

bool CmdLineParser::Found(const String& s, int64_t& v)
{
	String tmp;
	if (!Found(s, tmp)) return false;
	return tmp.ToNumber(&v);
}

size_t CmdLineParser::GetParamCount()
{
	return m_aParams.size();
}

String& CmdLineParser::GetParam(size_t i)
{
	if (i >= m_aParams.size())
	{
		Exception::XInvalidIndex();
	}
	return m_aParams[i];
}

EW_LEAVE
