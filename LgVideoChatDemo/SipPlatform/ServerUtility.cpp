

#include "SipPlatformDefine.h"
#include "ServerUtility.h"
#include "Log.h"
#include <stdio.h>
#include <string.h>

#ifndef WIN32
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#ifndef __APPLE__
#include <sys/prctl.h>
#endif

#include <signal.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <pwd.h>
#include <pthread.h>

#endif

#include "MemoryDebug.h"


void SetCoreDumpEnable()
{
#ifndef WIN32
#ifndef __APPLE__
	prctl(PR_SET_DUMPABLE, 1);
#endif
#endif
}


bool Fork( bool bIsFork )
{
#ifndef WIN32
	if( bIsFork )
	{
	  if( fork() != 0 ) exit(0);
	  
	  // 1��° �ڽ� ���μ��� ���� -> ���� ������ �����Ѵ�.
	  setsid();
	  signal( SIGHUP, SIG_IGN );
	  if( fork() != 0 ) exit(1);
	  
	  // 2��° �ڽ� ���μ��� ���� -> �۾� ���丮�� �����Ѵ�.
	  if( chdir( "/tmp" ) != 0 )
		{
		}
	}
#endif

	return true;
}


bool ChangeExecuteUser( const char * pszUserId )
{
#ifndef WIN32
	struct passwd sttPassWord, *psttPassWord = NULL;
	char szError[255], szPassWord[1024];

	if( getpwnam_r( pszUserId, &sttPassWord, szPassWord, sizeof(szPassWord), &psttPassWord ) != 0 || psttPassWord == NULL )
	{
		CLog::Print( LOG_ERROR, "getpwnam(%s) error(%d) - %s", pszUserId, errno, strerror_r( errno, szError, sizeof(szError) ) );
		return false;
	}

	if( setuid( psttPassWord->pw_uid ) != 0 )
	{
		CLog::Print( LOG_ERROR, "seteuid(%s) error(%d) - %s", pszUserId, errno, strerror_r( errno, szError, sizeof(szError) ) );
		return false;
	}

	if( psttPassWord->pw_dir )
	{
		if( chdir( psttPassWord->pw_dir ) != 0 )
		{
		}
#ifndef __APPLE__
		prctl(PR_SET_DUMPABLE, 1);
#endif
	}
#endif
	
	return true;
}


#ifdef WIN32
bool StartThread( const char * pszName, LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter )
{
	DWORD		dwThreadId;
	HANDLE	hThread;

	hThread = CreateThread( NULL, 0, lpStartAddress, lpParameter, 0, &dwThreadId );
	if( hThread == NULL )
	{
		return false;
	}

	CloseHandle( hThread );
	
	return true;
}
#else
bool StartThread( const char * pszName, void *(*lpStartAddress)(void*), void * lpParameter )
{
	pthread_t	iThread;

	if( pthread_create( &iThread, NULL, lpStartAddress, lpParameter ) != 0 )
	{
		return false;
	}

	pthread_detach( iThread );

	return true;
}
#endif
