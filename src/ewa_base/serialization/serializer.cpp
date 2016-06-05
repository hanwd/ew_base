#include "ewa_base/serialization/serializer.h"
#include "ewa_base/basic/stringbuffer.h"
#include "ewa_base/basic/codecvt.h"

EW_ENTER


template<typename A>
void serial_helper_func<A,String>::g(SerializerReader &ar,type &val)
{
	int n=ar.size_count(val.size());

	arr_1t<char> vect;
	vect.resize(n);

	ar.recv_checked(vect.data(),n);
	val.assign(vect.data(),n);

}


template<typename A>
void serial_helper_func<A,String>::g(SerializerWriter &ar,type &val)
{
	const char *p=val.c_str();
	int n=ar.size_count(val.size());
	if(n>0)	ar.send_checked((char *)p,n);
}

template class DLLIMPEXP_EWA_BASE serial_helper_func<SerializerWriter,String>;
template class DLLIMPEXP_EWA_BASE serial_helper_func<SerializerReader,String>;


EW_LEAVE
