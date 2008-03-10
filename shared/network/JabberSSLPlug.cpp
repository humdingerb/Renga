#include "JabberSSLPlug.h"


#define LOG(X) printf X;
#define STDOUT

JabberSSLPlug::JabberSSLPlug(BString forceserver, int32 port){

	bio = NULL;
	ctx = NULL;
	
	ffServer = forceserver;
	ffPort = port;
	
	 /* Set up the library */

    ERR_load_BIO_strings();
    SSL_load_error_strings();
    ERR_load_ERR_strings();
	ERR_load_crypto_strings();
    OpenSSL_add_all_algorithms();
    
    //if any?
    ERR_print_errors_fp(stderr); 
		
}


JabberSSLPlug::~JabberSSLPlug()
{
	if(bio != NULL & ctx !=NULL) 
		StopConnection();
}


int32
JabberSSLPlug::StartConnection(BString fServer, int32 fPort,void* cookie){
	
	BString fHost;
	
	fHost << fServer << ":" << fPort;
	
	LOG(("StartConnection to %s\n",fHost.String()));

	SSL * ssl = NULL;
	
    /* Set up the SSL context */
	
    ctx = SSL_CTX_new( SSLv23_client_method() );
	if( !ctx)
    {
        fprintf(stderr, "Error attempting to connect (SSL_CTX_new) %ld\n",ERR_get_error());
        ERR_print_errors_fp(stderr); 
       	return CONNECTING_TO_HOST;
    }

	bio = BIO_new_ssl_connect(ctx);
	if (!bio)
    {
        fprintf(stderr, "Error attempting to connect (BIO_new_ssl_connect)\n");
        ERR_print_errors_fp(stderr);
       	return CONNECTING_TO_HOST;
    }
    
    /* Set the SSL_MODE_AUTO_RETRY flag */
	
	BIO_get_ssl(bio, & ssl);
    if (!ssl)
    {
        fprintf(stderr, "Error attempting to connect (BIO_get_ssl)\n");
        ERR_print_errors_fp(stderr);
       	return CONNECTING_TO_HOST;
    }
    
    SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);

    /* Create and setup the connection */
	    
    BIO_set_conn_hostname(bio, fHost.String());

    if (BIO_do_connect(bio) <= 0)
    {
        fprintf(stderr, "Error attempting to connect (BIO_do_connect())\n");
        ERR_print_errors_fp(stderr);
       	return CONNECTING_TO_HOST;
    }
    
		
	LOG(("DONE: StartConnection to %s\n",fHost.String()));
	return 0; //connected.	
}


int32
JabberSSLPlug::ReceiveData(BMessage * mdata)
{
	
	char data[1024];
	int length = 0;

	if ((length = (int)BIO_read(bio, data, 1023) ) > 0) 
	{
		data[length] = 0;
		mdata->AddString("data", data);
					
		#ifdef STDOUT
			printf("\nSSLPlug<< %s\n", data);
		#endif
	} 
	else 
	{
		if(!BIO_should_retry(bio)) 
		{			
			//uhm really and error!
			#ifdef STDOUT
			 printf("\nError! NOT BIO_should_retry()!\n");
			 ERR_print_errors(bio);
			#endif	
			return -1;			
		}
	}
	
	mdata->AddInt32("length", length);
	return length;
}

void
JabberSSLPlug::ReceivedData(const char* data,int32 len)
{
}

int
JabberSSLPlug::Send(const BString & xml)
{
	#ifdef STDOUT
		printf("\nSSLPlug>> %s\n", xml.String());
	#endif
	
	return BIO_write(bio, xml.String(), xml.Length());
}

int		
JabberSSLPlug::StopConnection()
{
	BIO_free_all(bio);
    SSL_CTX_free(ctx);

	bio = NULL;
	ctx = NULL;
	return 0;
}


//--

