

#ifndef _TCP_THREAD_LIST_H_
#define _TCP_THREAD_LIST_H_

#include "SipPlatformDefine.h"
#include "SipTcp.h"
#include "SipMutex.h"
#include <vector>
#include <string>
#include "MonitorString.h"
#include "TcpSessionList.h"

class CTcpStack;


class CTcpComm
{
public:
	CTcpComm() : m_hSocket(INVALID_SOCKET), m_iPort(0), m_psttSsl(NULL), m_bClient(false)
	{
		m_szIp[0] = '\0';
	}

	void Close()
	{
		if( m_psttSsl )
		{
			SSLClose( m_psttSsl );
			m_psttSsl = NULL;
		}

		if( m_hSocket != INVALID_SOCKET )
		{
			closesocket( m_hSocket );
			m_hSocket = INVALID_SOCKET;
		}
	}

	Socket	m_hSocket;
	char		m_szIp[INET6_ADDRSTRLEN];	// ��Ŷ���� ���۵ǹǷ� std::string �� ����� �� ����.
	int			m_iPort;
	SSL			* m_psttSsl;

	
	bool		m_bClient;
};


class CTcpThreadInfo
{
public:
	CTcpThreadInfo();
	~CTcpThreadInfo();

	void Close();

	int				m_iIndex;				// ������ �ε���
	Socket		m_hSend;				// �۽� pipe
	Socket		m_hRecv;				// ���� pipe

	CTcpSessionList	m_clsSessionList;
	CTcpStack	* m_pclsStack;
};

typedef std::vector< CTcpThreadInfo * > THREAD_LIST;


class CTcpThreadList
{
public:
	CTcpThreadList();
	~CTcpThreadList();

	bool Create( CTcpStack * pclsStack );
	void Destroy();

	bool SendCommand( const char * pszData, int iDataLen );
	bool SendCommand( const char * pszData, int iDataLen, int iThreadIndex );
	void SendCommandAll( const char * pszData, int iDataLen );
	static int RecvCommand( Socket hSocket, char * pszData, int iDataSize );

	bool Send( int iThreadIndex, int iSessionIndex, const char * pszPacket, int iPacketLen );
	bool SendAll( const char * pszPacket, int iPacketLen, ITcpStackCallBack * pclsCallBack );
	bool SendAllExcept( const char * pszPacket, int iPacketLen, ITcpStackCallBack * pclsCallBack, int iThreadIndex, int iSessionIndex );

	void DeleteNoUseThread();
	bool DeleteThread( int iThreadIndex );

	void GetString( CMonitorString & strBuf );

private:
	THREAD_LIST	m_clsList;
	int					m_iMaxSocketPerThread;
	int					m_iThreadIndex;
	CSipMutex		m_clsMutex;
	CTcpStack * m_pclsStack;

	bool AddThread();
	bool _SendCommand( Socket hSocket, const char * pszData, int iDataLen );
	int GetCount();
	int GetThreadIndex();
	bool SelectThreadIndex( int iThreadIndex );
};

#endif
