

#include "SipPlatformDefine.h"
#include <time.h>
#include "SipUdp.h"
#include "Log.h"

#ifndef WIN32
#include <sys/ioctl.h>
#include <net/if.h>
#endif

#include "MemoryDebug.h"


Socket UdpSocket( bool bIpv6 )
{
	Socket iFd;
	
	iFd = socket( bIpv6 ? AF_INET6 : AF_INET, SOCK_DGRAM, IPPROTO_UDP );
	if( iFd == INVALID_SOCKET )
	{
		return INVALID_SOCKET;
	}

	return iFd;
}


Socket UdpListen( unsigned short iPort, const char * pszIp, bool bIpv6 )
{
	if( iPort == 0 )
	{
		return INVALID_SOCKET;
	}
	
	Socket iFd;
	int n;
	
	iFd = socket( bIpv6 ? AF_INET6 : AF_INET, SOCK_DGRAM, IPPROTO_UDP );
	if( iFd == INVALID_SOCKET )
	{
		return INVALID_SOCKET;
	}
	
#ifndef WINXP
	if( bIpv6 )
	{
		struct sockaddr_in6 addr;
		memset((char*) &(addr),0, sizeof((addr)));
		addr.sin6_family = AF_INET6;
		addr.sin6_port = htons(iPort);
		
		if( pszIp )
		{
			inet_pton( AF_INET6, pszIp, &addr.sin6_addr );
		}
		else
		{
			addr.sin6_addr = in6addr_any;
		}

		n = bind( iFd,(struct sockaddr*)&addr, sizeof(addr));
	}
	else
#endif
	{
		struct sockaddr_in addr;
		memset((char*) &(addr),0, sizeof((addr)));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(iPort);
		
		if( pszIp )
		{
#ifdef WINXP
			addr.sin_addr.s_addr = inet_addr(pszIp);
#else
		  inet_pton( AF_INET, pszIp, &addr.sin_addr.s_addr );
#endif
		}
		else
		{
			addr.sin_addr.s_addr = htonl(INADDR_ANY);
		}
	
		n = bind( iFd,(struct sockaddr*)&addr, sizeof(addr));
	}
	
	if( n != 0 )
	{
	  closesocket( iFd );
	  return INVALID_SOCKET;
	}
	
	return iFd;
}


bool UdpRecv( Socket iFd, char * pszBuf, int * piLen, char * pszIp, int iIpSize, unsigned short* piPort, bool bIpv6 )
{
	if( iFd == INVALID_SOCKET ) return false;
	
	int iBufSize = *piLen;
	if( iBufSize <= 0 ) return false;
	
#ifndef WINXP
	if( bIpv6 )
	{
		struct sockaddr_in6 sttAddr {};
		int iAddrSize = sizeof(sttAddr);
		
		*piLen = recvfrom( iFd, pszBuf, iBufSize, 0, (struct sockaddr *)&sttAddr, (socklen_t*)&iAddrSize );
		if( *piLen <= 0 )
		{
			return false;
		}
		
		if( piPort ) *piPort = ntohs( sttAddr.sin6_port );
		if( pszIp )
		{
			inet_ntop( AF_INET6, &sttAddr.sin6_addr, pszIp, iIpSize );
		}
	}
	else
#endif
	{
		struct sockaddr_in sttAddr {};
		int iAddrSize = sizeof(sttAddr);
		
		*piLen = recvfrom( iFd, pszBuf, iBufSize, 0, (struct sockaddr *)&sttAddr, (socklen_t*)&iAddrSize );
		if( *piLen <= 0 )
		{
		  return false;
		}
		
		if( piPort ) *piPort = ntohs( sttAddr.sin_port );
		if( pszIp )
		{
#ifdef WINXP
			snprintf( pszIp, iIpSize, "%s", inet_ntoa( sttAddr.sin_addr ) );
#else
			inet_ntop( AF_INET, &sttAddr.sin_addr, pszIp, iIpSize );
#endif
		}
	}

	if( (*piLen)+1 >= iBufSize )
	{
	  return false;
	}
	pszBuf[*piLen] = 0;
	
	return true;
}


bool UdpRecv( Socket iFd, char * pszBuf, int * piLen, unsigned int * piIp, unsigned short* piPort )
{
	if( iFd == INVALID_SOCKET ) return false;
	
	int iBufSize = *piLen;
	if( iBufSize <= 0 ) return false;
	
	struct sockaddr_in sttAddr {};
	int iAddrSize = sizeof(sttAddr);
	
	*piLen = recvfrom( iFd, pszBuf, iBufSize-1, 0, (struct sockaddr *)&sttAddr, (socklen_t*)&iAddrSize );
	if( *piLen <= 0 )
	{
	  return false;
	}
	
	if( piPort ) *piPort = sttAddr.sin_port;
	if( piIp ) *piIp = sttAddr.sin_addr.s_addr;

	if( (*piLen)+1 > iBufSize )
	{
	  return false;
	}
	pszBuf[*piLen] = 0;
	
	return true;
}


bool UdpRecv( Socket iFd, char * pszBuf, int * piLen, IN6_ADDR * psttIp, unsigned short* piPort )
{
	if( iFd == INVALID_SOCKET ) return false;
	
	int iBufSize = *piLen;
	if( iBufSize <= 0 ) return false;
	
	struct sockaddr_in6 sttAddr {};
	int iAddrSize = sizeof(sttAddr);
	
	*piLen = recvfrom( iFd, pszBuf, iBufSize-1, 0, (struct sockaddr *)&sttAddr, (socklen_t*)&iAddrSize );
	if( *piLen <= 0 )
	{
	  return false;
	}
	
	if( piPort ) *piPort = sttAddr.sin6_port;
	if( psttIp ) memcpy( psttIp, &sttAddr.sin6_addr, sizeof(IN6_ADDR) );

	if( (*piLen)+1 > iBufSize )
	{
	  return false;
	}
	pszBuf[*piLen] = 0;
	
	return true;
}


bool UdpSend( Socket iFd, const char * pszBuf, int iBufLen, const char * pszIp, unsigned short iPort )
{
	if( iFd == INVALID_SOCKET || pszBuf == NULL || iBufLen <= 0 || pszIp == NULL || iPort == 0 ) return false;
	
	int n;

#ifndef WINXP
	if( strstr( pszIp, ":" ) )
	{
		struct sockaddr_in6 sttAddr;
	  
		memset( &sttAddr, 0, sizeof(sttAddr) );
		sttAddr.sin6_family = AF_INET6;
		sttAddr.sin6_port = htons( iPort );
		inet_pton( AF_INET6, pszIp, &sttAddr.sin6_addr );

		n = sendto( iFd, pszBuf, iBufLen, 0, (sockaddr*)&sttAddr, sizeof(sttAddr) );
	}
	else
#endif
	{
	  struct sockaddr_in sttAddr;
	  
	  memset( &sttAddr, 0, sizeof(sttAddr) );
	  sttAddr.sin_family = AF_INET;
	  sttAddr.sin_port = htons( iPort );

#ifdef WINXP
		sttAddr.sin_addr.s_addr = inet_addr( pszIp );
#else
		inet_pton( AF_INET, pszIp, &sttAddr.sin_addr.s_addr );
#endif
  
		n = sendto( iFd, pszBuf, iBufLen, 0, (sockaddr*)&sttAddr, sizeof(sttAddr) );
	}
  
  if( n != iBufLen )
	{
		CLog::Print( LOG_ERROR, "%s sendto error(%d)", __FUNCTION__, GetError() );
	  return false;
	}
	
	return true;
}


bool UdpSend( Socket iFd, const char * pszBuf, int iBufLen, unsigned int iIp, unsigned short iPort )
{
	if( iFd == INVALID_SOCKET || pszBuf == NULL || iBufLen <= 0 || iPort == 0 ) return false;
	
  struct sockaddr_in sttAddr;
  
  memset( &sttAddr, 0, sizeof(sttAddr) );
  sttAddr.sin_family = AF_INET;
  sttAddr.sin_port = iPort;
  sttAddr.sin_addr.s_addr = iIp;
  
  if( sendto( iFd, pszBuf, iBufLen, 0,(sockaddr*)&sttAddr, sizeof(sttAddr) ) != iBufLen )
	{
	  return false;
	}
	
	return true;
}


bool UdpSend( Socket iFd, const char * pszBuf, int iBufLen, IN6_ADDR * psttIp, unsigned short iPort )
{
	if( iFd == INVALID_SOCKET || pszBuf == NULL || iBufLen <= 0 || iPort == 0 ) return false;
	
  struct sockaddr_in6 sttAddr;
  
  memset( &sttAddr, 0, sizeof(sttAddr) );
  sttAddr.sin6_family = AF_INET6;
  sttAddr.sin6_port = iPort;
	memcpy( &sttAddr.sin6_addr, psttIp, sizeof(sttAddr.sin6_addr) );
  
  if( sendto( iFd, pszBuf, iBufLen, 0,(sockaddr*)&sttAddr, sizeof(sttAddr) ) != iBufLen )
	{
	  return false;
	}
	
	return true;
}


void InitNetwork()
{
#ifdef WIN32
	static bool bInitNetwork = false;

	if( bInitNetwork == false )
	{
		WORD 		wVersionRequested = MAKEWORD( 2, 2 );
		WSADATA wsaData;
		
		if( WSAStartup( wVersionRequested, &wsaData ) != 0 ) 
		{
			return;
		}

		bInitNetwork = true;
	}
#endif
}


void TcpSetPollIn( struct pollfd & sttPollFd, Socket hSocket )
{
	sttPollFd.fd = hSocket;
	sttPollFd.events = POLLIN;
	sttPollFd.revents = 0;
}

#ifdef WIN32

int poll( struct pollfd *fds, unsigned int nfds, int timeout )
{
	fd_set	rset;
	unsigned int		i;
	int		n, iCount = 0;
	struct timeval	sttTimeout;

	sttTimeout.tv_sec = timeout / 1000;
	sttTimeout.tv_usec = ( timeout % 1000 ) * 1000;

	FD_ZERO(&rset);
	for( i = 0; i < nfds; ++i )
	{
		if( fds[i].fd != -1 ) FD_SET( fds[i].fd, &rset );
	}

	n = select( 0, &rset, NULL, NULL, &sttTimeout );
	if( n <= 0 )
	{
		return n;
	}

	for( i = 0; i < nfds; ++i )
	{
		if( FD_ISSET( fds[i].fd, &rset ) )
		{
			fds[i].revents = POLLIN;
			++iCount;
		}
		else
		{
			fds[i].revents = 0;
		}
	}

	return iCount;
}
#endif


bool GetLocalIp( std::string & strIp )
{
#ifdef WIN32
	char szHostName[128], szIpAddr[INET6_ADDRSTRLEN];
	struct sockaddr_in sttAddr;
	struct hostent     *psttHost = NULL;

	InitNetwork();

	memset( szHostName, 0, sizeof(szHostName) );
	if( gethostname(szHostName, sizeof(szHostName)) )	return false;

	psttHost = gethostbyname( szHostName );
	if( !psttHost ) return false;
	
	for( int i = 0; psttHost->h_addr_list[i]; i++ )
	{
		memcpy( &sttAddr.sin_addr, psttHost->h_addr_list[i], psttHost->h_length );

#ifdef WINXP
		snprintf( szIpAddr, sizeof(szIpAddr), "%s", inet_ntoa(sttAddr.sin_addr) );
#else
		inet_ntop( AF_INET, &sttAddr.sin_addr, szIpAddr, sizeof(szIpAddr) );
#endif

		if( strcmp( szIpAddr, "127.0.0.1" ) )
		{
			strIp = szIpAddr;
			break;
		}
	}
#else
	struct ifconf ifc;
	
	int hSocket = socket( AF_INET, SOCK_DGRAM, 0 );
	int len = 100 * sizeof(struct ifreq);
	
	char buf[ len ];
	char szError[255];
	
	ifc.ifc_len = len;
	ifc.ifc_buf = buf;
	
	int e = ioctl( hSocket, SIOCGIFCONF, &ifc );
	if( e == -1 ) return false;

	char *ptr = buf;
	int tl = ifc.ifc_len;
	char	szIpAddr[INET6_ADDRSTRLEN];
	
	while ( tl > 0 )
	{
		struct ifreq * ifr = (struct ifreq *)ptr;
		int si;
		
#ifdef __APPLE__
		si = sizeof(ifr->ifr_name) + sizeof(ifr->ifr_addr);
#else
		si = sizeof(ifr->ifr_name) + sizeof(ifr->ifr_map);
#endif

		tl -= si;
		ptr += si;
		
		struct ifreq ifr2;
		ifr2 = *ifr;
		
		e = ioctl( hSocket, SIOCGIFADDR, &ifr2 );
		if( e == -1 )
		{
			printf( "ioctl error(%d) - %s\n", errno, strerror_r( errno, szError, sizeof(szError) ) );
			continue;
		}
		
		struct sockaddr a = ifr2.ifr_addr;
		struct sockaddr_in* addr = (struct sockaddr_in*) &a;
		
		unsigned int ai = ntohl( addr->sin_addr.s_addr );

		// 127.0.0.1 ÁÖ¼Ò´Â Ãâ·ÂÇÏÁö ¾Ê´Â´Ù.
		if( int((ai>>24)&0xFF) == 127 ) continue;

		snprintf( szIpAddr, sizeof(szIpAddr), "%d.%d.%d.%d", int((ai>>24)&0xFF)
			, int((ai>>16)&0xFF)
			, int((ai>> 8)&0xFF) 
		  , int((ai    )&0xFF) );
		strIp = szIpAddr;
		break;
	}
	
	close(hSocket);
#endif

	return true;
}
