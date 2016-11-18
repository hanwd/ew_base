#include "ewa_base/math/random.h"
#include "ewa_base/basic/clock.h"
#include <limits>
EW_ENTER


class randgen_data : public ObjectData
{
public:

	static const int N=32;
	static const int M=N-1;

	uint32_t data[N];
	uint32_t nidx;

	void seed(uint32_t v)
	{
		seed(&v,1);
	}

	void seed(uint32_t* p,int n)
	{
		std::fill_n(data,N,0x9908b0df);

		for(int i=0;i<N;i++)
		{
			uint32_t tmp=p[i%n];
			data[i&M]^=((tmp*0x98765431)^(i*0x12345679))+(tmp>>8);
		}

		for(int i=0;i<128;i++)
		{
			refill();
		}
	}

	uint32_t gen()
	{
		if(nidx==(uint32_t)N) refill();
		uint32_t res = data[nidx++];
		return res;
	}

	void refill()
	{
		for(int i=0;i<N;i++)
		{
			uint32_t tmp=((data[(i+1)&M]&0x0000FFFF)|(data[(i+3)&M]&0xFFFF0000))+i;
			data[i]=(tmp>>1)^(tmp&1?0x9908b0df:0)^data[(i+7)&M];
		}
		nidx=0;
	}

	void nextp(uint32_t v1,uint32_t v2,uint32_t v3)
	{
		uint32_t vp[3]={v1,v2,v3};

		for(int i=0;i<N;i++)
		{
			uint32_t tmp=(data[(i+1)&M]&0x0000FFFF)|(data[(i+3)&M]&0xFFFF0000);
			tmp=(tmp^vp[i%3])+vp[(i+1)%3]-vp[(i+2)%3];
			data[i]=(tmp>>1)^(tmp&1?0x9908b0df:0)^data[(i+7)&M];
		}
	}
};


rand_gen::rand_gen()
{
	m_pData1.reset(new randgen_data);
	seed();
}

uint32_t rand_gen::random_int(uint32_t v)
{
	uint32_t n=v*(std::numeric_limits<uint32_t>::max()/v);
	while(1)
	{
		uint32_t r=((randgen_data*)m_pData1.get())->gen();
		if(r<n)
		{
			return r%v;
		}
	}

	EW_ASSERT(false);
	return -1;
}

double rand_gen::random_double()
{
	int32_t k=1000000000;
	int32_t r=random_int(2*k+1);
	double v= double(r-k)/double(k);
	return v;
}

uint32_t rand_gen::operator()()
{
	return ((randgen_data*)m_pData1.get())->gen();
}

void rand_gen::seed()
{
	uint64_t v=Clock::now().val;
	((randgen_data*)m_pData1.get())->seed((uint32_t*)&v,2);
}

void rand_gen::seed(double v)
{
	((randgen_data*)m_pData1.get())->seed((uint32_t*)&v,2);
}

void rand_gen::seed(uint32_t v)
{
	((randgen_data*)m_pData1.get())->seed(&v,1);
}

void rand_gen::seed(uint32_t* p,int n)
{
	((randgen_data*)m_pData1.get())->seed(p,n);
}

void rand_gen::save_state()
{
	m_pData2.reset(new randgen_data(*((randgen_data*)m_pData1.get())));
}

void rand_gen::load_state()
{
	if(!m_pData2) return;
	*m_pData1=*m_pData2;
}

void rand_gen::load_state(uint32_t i,uint32_t j,uint32_t k)
{
	load_state();
	((randgen_data*)m_pData1.get())->nextp(i,j,k);
}


EW_LEAVE
