#include "ewa_base/net/overlapped.h"
#include "ewa_base/memory/mempool.h"
#include "ewa_base/basic/stringbuffer.h"
#include "ewa_base/basic/system.h"

EW_ENTER

MyOverLapped::MyOverLapped(int t):type(t)
{
#ifdef EW_WINDOWS
	memset(&olap,0,sizeof(olap));
#endif
	dbuf[0].buf=NULL;
	dbuf[0].len=0;
	dbuf[1].buf=NULL;
	dbuf[1].len=0;
}

void MyOverLappedRb::done()
{
	if(flag.exchange(0)!=1)
	{
		System::LogTrace("flag!=1 in done");
	}
}

void MyOverLappedRb::done_send()
{
	if(size>=0)
	{
		buff.rd_flip(size);
	}
	else
	{
		System::LogTrace("size<0 in done_send");
	}

	done();
}

void MyOverLappedRb::done_recv()
{

	if(size>=0)
	{
		buff.wr_flip(size);
	}
	else
	{
		System::LogTrace("size<0 in done_recv");
	}

	done();
}

bool MyOverLappedRb::init_send()
{
	if(flag.exchange(1)!=0)
	{
		return false;
	}

	MyOverLapped& olap_send(*this);

	int32_t mask=buff.pHeader->rb_mask;
	int32_t rd=buff.pHeader->rd_pos;
	int32_t wr=buff.pHeader->wr_pos;
	int32_t sz=(wr-rd)&mask;

	if(sz==0)
	{
		done();
		return false;
	}

	if(sz+rd>mask)
	{
		olap_send.dbuf[0].buf=buff.pBuffer+rd;
		olap_send.dbuf[0].len=mask+1-rd;
		olap_send.dbuf[1].buf=buff.pBuffer;
		olap_send.dbuf[1].len=sz+rd-mask-1;
	}
	else
	{
		olap_send.dbuf[0].buf=buff.pBuffer+rd;
		olap_send.dbuf[0].len=sz;
		olap_send.dbuf[1].buf=NULL;
	}

	size=sz;

	return true;
}

bool MyOverLappedRb::init_recv()
{
	if(flag.exchange(1)!=0)
	{
		return false;
	}

	MyOverLapped& olap_recv(*this);

	int32_t mask=buff.pHeader->rb_mask;
	int32_t rd=buff.pHeader->rd_pos;
	int32_t wr=buff.pHeader->wr_pos;
	int32_t sz=(rd-wr-1)&mask;

	if(sz==0)
	{
		done();
		return false;
	}

	if(sz>1024*4)
	{
		sz=1024*4;
	}

	if(sz+wr>mask)
	{
		olap_recv.dbuf[0].buf=buff.pBuffer+wr;
		olap_recv.dbuf[0].len=mask+1-wr;
		olap_recv.dbuf[1].buf=buff.pBuffer;
		olap_recv.dbuf[1].len=sz+wr-mask-1;
	}
	else
	{
		olap_recv.dbuf[0].buf=buff.pBuffer+wr;
		olap_recv.dbuf[0].len=sz;
		olap_recv.dbuf[1].buf=NULL;
	}

	size=sz;
	return true;
}



MyOverLappedEx::MyOverLappedEx()
{
	buffer=(char*)MemPoolPaging::current().allocate(IPacket::MAX_PACKET_SIZE);

	dbuf[0].buf=buffer;
	dbuf[0].len=0;
	dbuf[1].buf=NULL;
	dbuf[1].len=0;

	flags = 0;
}

MyOverLappedEx::~MyOverLappedEx()
{
	MemPoolPaging::current().deallocate(buffer);
}


void PerIO_socket::swap(PerIO_socket& sk)
{
	sock.swap(sk.sock);
	peer.swap(sk.peer);
	addr.swap(sk.addr);
}



bool IPacket::update()
{
	if(size>MAX_PACKET_SIZE||size<MIN_PACKET_SIZE)
	{
		return false;
	}
	stamp=Clock::now();

	kcrc=crc32(((char*)this)+4,size-4);
	return true;
}


bool IPacket::check()
{
	if(size>MAX_PACKET_SIZE||size<MIN_PACKET_SIZE)
	{
		return false;
	}
	uint32_t n=crc32(((char*)this)+4,size-4);
	if(n!=kcrc)
	{
		return false;
	}

	return true;

}


bool ISession::InitSend()
{

	if(state.get()!=STATE_OK)
	{
		return false;
	}

	if(olap_send.init_send())
	{
		return true;
	}
	else
	{
		return false;
	}

}

bool ISession::InitRecv()
{
	if(state.get()!=STATE_OK)
	{
		return false;
	}

	if(olap_recv.init_recv())
	{
		return true;
	}
	else
	{
		return false;
	}
}


EW_LEAVE
