
#ifndef __H_EW_NET_Q2PACKET__
#define __H_EW_NET_Q2PACKET__


#include "ewa_base/net/overlapped.h"
#include "ewa_base/collection/arr_1t.h"

EW_ENTER

template<typename P>
class Q2Packet
{
public:

	Q2Packet(){q1=q2=0;}

	// process buffer, after update, you can call size() to get the number of cached packets.
	// NOTE: old cached packets will be DROP! n must less than or equal to P::MAX_PACKET_SIZE
	bool update(const char* p,size_t n);

	bool update(TempOlapPtr& q)
	{
		return update(q->buffer,q->size);
	}

	// number of cached packets
	inline size_t size() const
	{
		return ap.size();
	}

	// get cached packet by index 0~size()-1
	inline P& operator[](size_t i)
	{
		return *ap[i];
	}

	// drop cached packets but keep partial data
	void done();

	// drop cached packets and partial data
	void reset()
	{
		q1=q2=0;
		ap.clear();
	}

private:

	char pk[P::MAX_PACKET_SIZE*2];
	arr_1t<P*> ap;
	size_t q1,q2;

};




template<typename P>
void Q2Packet<P>::done()
{

	if(q1==0) return;

	ap.clear();
	q2-=q1;
	for(size_t i=0;i<q2;i++)
	{
		pk[i]=pk[q1+i];
	}
	q1=0;
}

template<typename P>
bool Q2Packet<P>::update(const char* p1,size_t nz)
{
	if(nz==0)
	{
		return true;
	}

	if(nz>P::MAX_PACKET_SIZE)
	{
		System::LogTrace("buffer too large");
		return false;
	}

	done();

	EW_ASSERT(q1==0);

	const char* p2=p1+nz;
	size_t n0=q2;

	const size_t nh=P::MIN_PACKET_SIZE;

	if(n0<nh)
	{
		size_t nl=nh-n0;
		if(nz<nl)
		{
			memcpy(pk+q2,p1,nz);
			q2+=nz;
			return true;
		}

		memcpy(pk+q2,p1,nl);
		q2=nh;
		p1+=nl;

	}

	while(1)
	{

		size_t sz_real=(*(P*)(pk+q1)).size;
		size_t sz_left=q2-q1;

		if(sz_real>sz_left)
		{
			if(sz_real>P::MAX_PACKET_SIZE)
			{
				System::LogTrace("invalid packet size");
				return false;
			}

			size_t sz_more=sz_real-sz_left;
			size_t n1=p2-p1;

			if(sz_more>n1)
			{
				memcpy(pk+q2,p1,n1);
				q2+=n1;
				return true;
			}

			memcpy(pk+q2,p1,sz_more);

			p1+=sz_more;
			q2+=sz_more;
		}

		ap.push_back((P*)(pk+q1));
		q2=(q2+sizeof(void*)-1)&~(sizeof(void*)-1); // adjust q1,q2;
		q1=q2;

		if((size_t)(p2-p1)>=nh)
		{
			memcpy(pk+q2,p1,nh);
			q2+=nh;
			p1+=nh;
		}
		else
		{
			size_t nl=p2-p1;
			memcpy(pk+q2,p1,nl);
			q2+=nl;
			return true;
		}
	}


	// never goes here
	return true;

}

EW_LEAVE
#endif
