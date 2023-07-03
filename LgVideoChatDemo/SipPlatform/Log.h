

#ifndef _LOG_H_
#define _LOG_H_

#include "SipPlatformDefine.h"
#include <stdio.h>
#include "SipMutex.h"
#include "Directory.h"

#define LOG_MAX_SIZE								1024*8
#define FULLPATH_FILENAME_MAX_SIZE	1024
#define MIN_LOG_FILE_SIZE						1024*1024
#define MAX_LOG_FILE_SIZE						1024*1024*1024
#define DEFAULT_LOG_FILE_SIZE				1024*1024*10
#define DEFAULT_LOG_FOLDER_SIZE			3145728000UL


enum EnumLogLevel
{
	LOG_NONE = 0x0000,
	
	LOG_ERROR = 0x0001,
	
	LOG_INFO  = 0x0010,
	
	LOG_DEBUG = 0x0100,
	
	LOG_NETWORK = 0x0200,
	
	LOG_SYSTEM = 0x0400,
	
	LOG_HTTP2 = 0x0800,

	LOG_HTTP_HEADER = 0x1000,

	LOG_WEBRTC = 0x2000,
};


class ILogCallBack
{
public:
	virtual ~ILogCallBack(){};

	virtual void Print( EnumLogLevel eLevel, const char * fmt, ... ) = 0;
};


class CLog
{
private:
	static char				* m_pszDirName;		// �α� ������ ������ ���丮 �̸�
	static char				m_szDate[9];		// ���� �������� �α� ���� �̸�
	static FILE				* m_sttFd;			// �α� ���� ������

	static CSipMutex	* m_pThreadMutex;	// Thread Mutex
	
	static int				m_iLevel;				// �α� ���Ͽ� ������ ����
	static int				m_iMaxLogSize;	// �ϳ��� �α� ���Ͽ� ������ �� �ִ� �ִ� ũ��
	static int				m_iLogSize;			// ������� ����� �α� ũ��
	static int				m_iIndex;				// �α� ���� �ε���
	static int64_t		m_iMaxFolderSize;	// �α� ������ �ִ� ũ��
	static int64_t		m_iFolderSize;		// �α� ���� ũ��

	static ILogCallBack * m_pclsCallBack;	// �α� callback

public:
	static bool SetDirectory( const char * pszDirName );
	static void Release();

	static void SetCallBack( ILogCallBack * pclsCallBack );

	static int Print( EnumLogLevel iLevel, const char * fmt, ... );
	static void Print( void (* func)( FILE * fd ) );

	static int GetLevel( );
	static void SetLevel( int iLevel );
	static void SetNullLevel();

	static void SetDebugLevel( );
	static bool IsPrintLogLevel( EnumLogLevel iLevel );
	static void SetMaxLogSize( int iSize );
	static void SetMaxFolderSize( int64_t iSize );
	static int GetLogIndex();
	static void DeleteOldFile( );

	static void SortFileList( FILE_LIST & clsFileList );
};

#endif
