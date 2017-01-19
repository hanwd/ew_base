#include "ewa_base/net/session_http.h"
#include "ewa_base/scripting/callable_buffer.h"
#include "ewa_base/scripting/executor.h"
#include "ewa_base/util/strlib.h"
#include "ewa_base/basic/system.h"
#include "ewa_base/basic/scanner_helper.h"

EW_ENTER



typedef void SSL_CTX;
typedef void SSL;
typedef void SSL_METHOD;
typedef void BIO_METHOD;
typedef void BIO;

const SSL_METHOD *(*SSLv23_server_method)(void);
const SSL_METHOD *(*SSLv23_client_method)(void);
SSL_CTX *(*SSL_CTX_new)(const SSL_METHOD *meth);

void (*SSL_set_connect_state)(SSL *s);
void (*SSL_set_accept_state)(SSL *s);

int (*SSL_read)(SSL *ssl, void *buf, int num);
int (*SSL_write)(SSL *ssl, const void *buf, int num);
SSL *(*SSL_new)(SSL_CTX *ctx);
void (*SSL_set_bio)(SSL *s, BIO *rbio, BIO *wbio);
int (*SSL_connect)(SSL *ssl);
int (*SSL_accept)(SSL *ssl);
int (*SSL_shutdown)(SSL *s);
//void (*SSL_library_init)();

const BIO_METHOD *(*BIO_s_mem)(void);
BIO *(*BIO_new)(const BIO_METHOD *type);
size_t (*BIO_ctrl_pending)(BIO *b);
int (*BIO_read)(BIO *b, void *data, int len);
int (*BIO_write)(BIO *b, const void *data, int len);

int (*SSL_CTX_use_PrivateKey_file)(SSL_CTX *ctx, const char *file, int type);
int (*SSL_CTX_check_private_key)(const SSL_CTX *ctx);

int (*SSL_CTX_use_certificate_file)(SSL_CTX *ctx, const char *file, int type);
int (*SSL_check_private_key)(const SSL *ctx);

int	(*SSL_set_fd)(SSL *s, int fd);

int (*OPENSSL_init_ssl)(uint64_t opts, const void *settings);

class ModuleSSL
{
public:


	template<typename T>
	T* GetSymbolEx(const ew::String& name, T*& func)
	{
		void* p=NULL;
		if(p=s1.GetSymbol(name))
		{

		}
		else if(p=s2.GetSymbol(name))
		{

		}
		else
		{
			s1.Close();
			s2.Close();
		}
		func=(T*)p;
		return func;
	}

	ew::DllModule s1;
	ew::DllModule s2;

	bool init()
	{

		s1.Open("libssl-1_1.dll");
		s2.Open("libcrypto-1_1.dll");

		GetSymbolEx("SSL_CTX_new",SSL_CTX_new);
		GetSymbolEx("BIO_s_mem",BIO_s_mem);
		GetSymbolEx("SSL_set_connect_state",SSL_set_connect_state);
		GetSymbolEx("SSL_set_accept_state",SSL_set_accept_state);
		GetSymbolEx("BIO_ctrl_pending",BIO_ctrl_pending);
		GetSymbolEx("BIO_read",BIO_read);
		GetSymbolEx("BIO_write",BIO_write);
		GetSymbolEx("SSL_read",SSL_read);
		GetSymbolEx("SSL_write",SSL_write);

		GetSymbolEx("SSL_new",SSL_new);
		GetSymbolEx("BIO_new",BIO_new);
		GetSymbolEx("SSL_set_bio",SSL_set_bio);
		GetSymbolEx("SSL_connect",SSL_connect);
		GetSymbolEx("SSL_accept",SSL_accept);
		GetSymbolEx("TLS_server_method",SSLv23_server_method);
		GetSymbolEx("TLS_client_method",SSLv23_client_method);



		GetSymbolEx("SSL_shutdown",SSL_shutdown);

		GetSymbolEx("SSL_CTX_use_certificate_file",SSL_CTX_use_certificate_file);
		GetSymbolEx("SSL_CTX_use_PrivateKey_file",SSL_CTX_use_PrivateKey_file);
		GetSymbolEx("SSL_CTX_check_private_key",SSL_CTX_check_private_key);

		GetSymbolEx("SSL_set_fd",SSL_set_fd);

		GetSymbolEx("OPENSSL_init_ssl",OPENSSL_init_ssl);

		if(OPENSSL_init_ssl) OPENSSL_init_ssl(0,NULL);  
		

		return s1.IsOk();
	
	}

	bool IsOk()
	{
		return OPENSSL_init_ssl!=NULL;
	}

	ModuleSSL()
	{
		init();		
	}

	static ModuleSSL& current()
	{
		static ModuleSSL gi;
		return gi;
	}

};



class SessionHttpSSL : public SessionHttp
{
public:

	typedef SessionHttp basetype;
	SessionHttpSSL(SessionManager& t);
	~SessionHttpSSL();

	virtual void OnRecvCompleted(TempOlapPtr& q);

	bool AsyncSend(const char* data,size_t size,int flag);

	bool InitSSL(SessionHttpSSLServer&);

protected:

	void* ssl;
	void* bioIn;
	void* bioOut;

	bool SSLAccept();
};


class SSLContext : public Object
{
public:

	void* ctx0;

	bool Init(const String& cert,const String& key)
	{
		const SSL_METHOD* meth=SSLv23_server_method();
		void* ctx = SSL_CTX_new(meth);
		if(SSL_CTX_use_certificate_file(ctx,cert.c_str(),1)<0)
		{
			return false;
		}
		if (SSL_CTX_use_PrivateKey_file(ctx,key.c_str(),1)<0)
		{
			return false;
		}
		if (!SSL_CTX_check_private_key(ctx))
		{
			return false;
		}
		ctx0=ctx;	
		return true;
	}


	SSLContext()
	{
		ctx0=NULL;		

	}

};

void* SessionHttpSSLServer::GetSSLContext()
{
	if(!m_pSSLContext) return NULL;
	return static_cast<SSLContext*>(m_pSSLContext.get())->ctx0;
}

bool SessionHttpSSLServer::InitSSL(const String& cert,const String& key)
{
	if(!ModuleSSL::current().IsOk())
	{
		this_logger().LogError("openssl init failed");
		return false;
	}

	AutoPtrT<SSLContext> p(new SSLContext());
	if(!p->Init(cert,key))
	{
		return false;
	}

	m_pSSLContext.reset(p.release());
	return true;

}


bool SessionHttpSSL::InitSSL(SessionHttpSSLServer& t)
{
	void* p=t.GetSSLContext();
	if(!p) return false;
	ssl=SSL_new(p);
	if(!ssl) return false;
	bioIn = BIO_new(BIO_s_mem());
	bioOut = BIO_new(BIO_s_mem());

	if(!bioIn||!bioOut)
	{
		return false;
	}

	SSL_set_bio(ssl,bioIn,bioOut);
	return true;
}

SessionHttpSSL::SessionHttpSSL(SessionManager& t):SessionHttp(t)
{
	ssl=NULL;
	phase=-1;

}

SessionHttpSSL::~SessionHttpSSL()
{
	if(ssl) SSL_shutdown(ssl);
}

bool SessionHttpSSL::SSLAccept()
{
	int ret=SSL_accept(ssl);
	if(ret==0)
	{
		Disconnect();
		return false;
	}

	AsyncSend(NULL,0,0);
	if(ret==1)
	{
		phase=0;
		return true;
	}
	else
	{
		return false;
	}
}


void SessionHttpSSL::OnRecvCompleted(TempOlapPtr& q)
{
	size_t s1=sb.size();
	BIO_write(bioIn,q->buffer,q->size);

	if(phase==-1 && !SSLAccept())
	{
		AsyncRecv(q);
		return;
	}

	char buf[1024*4];
	int ret;
	while(ret=SSL_read(ssl,buf,sizeof(buf)))
	{
		if(ret>0) sb.append(buf,ret);
		else
		{
			break;
		}
	}

	_HandleData(q,s1);
}

bool SessionHttpSSL::AsyncSend(const char* data,size_t size,int flag)
{
	if(size>0)
	{
		SSL_write(ssl,data,size);	
	}

	char buf[1024*4];

	while(BIO_ctrl_pending(bioOut))
	{
		int len = BIO_read(bioOut,buf,sizeof(buf));
		if(len<=0) break;

		int f=BIO_ctrl_pending(bioOut)?0:flag;
		if(!basetype::AsyncSend(buf,len,f))
		{
			return false;
		}
	
	}
	return true;
}



void SessionHttpSSLServer::NewSession(PerIO_socket& sk)
{
	DataPtrT<SessionHttpSSL> kjobd_worker(new SessionHttpSSL(*Target));
	if(!kjobd_worker->InitSSL(*this))
	{
		System::LogTrace("session_http_ssl initssl failed");
		return;
	}

	sk.sock.block(false);
	kjobd_worker->sk_local.swap(sk);
	StartSession(kjobd_worker.get(),hiocp);
}


EW_LEAVE
