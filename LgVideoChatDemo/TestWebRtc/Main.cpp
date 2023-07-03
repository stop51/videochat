

#include "WebRtcServer.h"
#include "Directory.h"
#include "Log.h"
#include "MemoryDebug.h"

CHttpStack gclsStack;

int main( int argc, char * argv[] )
{
	if( argc < 2 )
	{
		printf( "[Usage] %s {Document root path} {RSA key/cert file path}\n", argv[0] );
		return 0;
	}

#ifdef WIN32
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_ALWAYS_DF );
#endif

	CWebRtcServer clsServer;
	CTcpStackSetup clsSetup;

#ifdef WIN32
	CLog::SetDirectory( "c:\\temp\\http" );
	CLog::SetLevel( LOG_WEBRTC );
#endif

	const char * pszDocumentRoot = argv[1];
	const char * pszCertFile = NULL;

	if( argc >= 3 )
	{
		pszCertFile = argv[2];
	}

	// HTTP ���� ��Ʈ ��ȣ�� �����Ѵ�.
	clsSetup.m_iListenPort = 8080;
	clsSetup.m_iMaxSocketPerThread = 1;
	clsSetup.m_iThreadMaxCount = 0;
	clsSetup.m_bUseThreadPipe = false;
	
	

	if( pszCertFile )
	{
		clsSetup.m_iListenPort = 443;
		clsSetup.m_bUseTls = true;
		clsSetup.m_strCertFile = pszCertFile;
	}

	// HTTP �������� ����� Document root ������ �����Ѵ�.
	clsServer.m_strDocumentRoot = pszDocumentRoot;

	if( CDirectory::IsDirectory( clsServer.m_strDocumentRoot.c_str() ) == false )
	{
		printf( "[%s] is not directory\n", clsServer.m_strDocumentRoot.c_str() );
		return 0;
	}

	// HTTP ������ �����Ѵ�. HTTP ��û�� ���ŵǸ� �̿� ���� �̺�Ʈ�� CSimpleHttpServer ��ü�� �����Ѵ�.
	if( gclsStack.Start( &clsSetup, &clsServer ) == false )
	{
		printf( "clsStack.Start error\n" );
		return 0;
	}

	while( clsServer.m_bStop == false )
	{
		sleep(1);
	}

	gclsStack.Stop();

	// ��� �����尡 ����� ������ ����Ѵ�.
	sleep(2);

#ifdef WIN32
	CLog::Release();
#endif

	return 0;
}
