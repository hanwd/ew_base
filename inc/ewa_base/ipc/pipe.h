
#ifndef __H_EW_IPC_PIPE__
#define __H_EW_IPC_PIPE__

#include "ewa_base/config.h"
#include "ewa_base/basic/platform.h"
#include "ewa_base/basic/string.h"

EW_ENTER

class pipe_detail;

class DLLIMPEXP_EWA_BASE Pipe
{
public:
	class impl_type : public KO_Handle<KO_Policy_handle>
	{
	public:
		KO_Handle<KO_Policy_handle> serv;
		String name;
	};

	Pipe();
	~Pipe();

	int Send(const char* buf,int len);
	int Recv(char* buf,int len);

	void Close();

	static bool LinkPipe(Pipe& reader,Pipe& writer,int buflen=0);

protected:
	impl_type impl;

};

class DLLIMPEXP_EWA_BASE NamedPipe : public Pipe
{
public:
	friend class pipe_detail;

	NamedPipe() {}

// server
	bool Create(const String& name);
	bool Accept();
	void Disconnect();

//client
	bool Connect(const String& name);
	bool Connect(const String& name,int t);

};

EW_LEAVE

#endif
