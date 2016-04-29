#include "ewa_base/basic/bitflags.h"

EW_ENTER


void BitFlags::set(int32_t f,bool v)
{
	if(v)
	{
		add(f);
	}
	else
	{
		del(f);
	}
}

EW_LEAVE
