#include "ewa_base/basic/scanner_helper.h"

EW_ENTER




template<typename B>
bool ScannerHelper<B>::read_uint(B &p, int64_t& v)
{
	v = 0;

	char ch = p[0];
	if (ch == '0')
	{

		ch = rebind<lkt2uppercase,0>::type::test(*++p);
		if (ch == 'X')
		{
			return read_uint_t<16>(++p, v);
		}
		else if (ch == 'B')
		{
			return read_uint_t<2>(++p, v);
		}
		else if (ch >= '1'&&ch <= '9')
		{
			return read_uint_t<8>(++p, v);
		}
		else if (ch == '0')
		{
			return false;
		}
		else
		{
			return true;
		}
	}
	else
	{
		return read_uint_t<10>(p, v);
	}
}

template<typename B>
bool ScannerHelper<B>::read_sign(B &p)
{
	if (p[0] == '-')
	{
		++p;
		return true;
	}
	else if (p[0] == '+')
	{
		++p;
	}
	return false;
}


template<typename B>
bool ScannerHelper<B>::read_number(B &p, int64_t& v)
{
	while (p[0] == ' ' || p[0] == '\t') ++p;
	bool sign = read_sign(p);
	if (!read_uint(p,v))
	{
		return false;
	}
	if (sign)
	{
		v = -v;
	}
	return true;
}

template<typename B>
bool ScannerHelper<B>::read_number(B &p, double& v)
{
	while (p[0] == ' ' || p[0] == '\t') ++p;

	bool sign = read_sign(p);

	pointer p0 = p;

	int64_t ipart(0);
	read_uint(p, ipart);

	double dpart(ipart);

	if (p[0] == '.')
	{
		if (*++p == '#')
		{
			if (p[1] == 'I'&&p[2] == 'N'&&p[3] == 'F')
			{
				v = sign ? -std::numeric_limits<double>::infinity() : std::numeric_limits<double>::infinity();
				return true;
			}
			if ((p[1] == 'I'&&p[2] == 'N'&&p[3] == 'D') || (p[1] == 'N'&&p[2] == 'A'&&p[3] == 'N') || (p[1] != 0 && p[2] == 'N'&&p[3] == 'A'))
			{
				v = sign ? -std::numeric_limits<double>::quiet_NaN() : std::numeric_limits<double>::quiet_NaN();
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			int64_t v2(0);
			pointer p0 = p;
			read_uint_t<10>(p, v2);
			dpart += double(v2)*::pow(0.1, p - p0);
		}
	}
	else if (p == p0)
	{
		return false;
	}

	if (p[0] == 'e' || p[0] == 'E')
	{
		bool sign2 = read_sign(++p);
		int64_t v3(0);
		read_uint_t<10>(p, v3);
		dpart *= ::pow(10.0, sign2 ? -v3 : v3);
	}

	v = sign ? -dpart : dpart;
	return true;

}

template class ScannerHelper <const char* > ;
template class ScannerHelper <char_pos_t<char> >;
template class ScannerHelper <char_pos_t<uint8_t> >;
template class ScannerHelper <char_pos_t<uint16_t> >;
template class ScannerHelper <char_pos_t<uint32_t> >;


EW_LEAVE
