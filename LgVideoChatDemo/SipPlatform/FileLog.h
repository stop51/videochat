

#ifndef _FILE_LOG_H_
#define _FILE_LOG_H_

#include "Log.h"


class CFileLog
{
public:
	CFileLog();
	~CFileLog();

public:
	bool Open( const char * pszDirName );
	bool Close( );

	bool Print( EnumLogLevel iLevel, const char * fmt, ... );

	int GetLevel( );
	void SetLevel( int iLevel );
	bool IsPrintLogLevel( EnumLogLevel iLevel );

	void SetMaxLogSize( int iSize );
	void SetMaxFolderSize( int64_t iSize );

private:
	void DeleteOldFile( );

	std::string	m_strDirName;		// �α� ������ ������ ���丮 �̸�
	char				m_szDate[9];		// ���� �������� �α� ���� �̸�
	FILE				* m_sttFd;			// �α� ���� ������

	CSipMutex		m_clsMutex;			// Thread Mutex
	
	int				m_iLevel;					// �α� ���Ͽ� ������ ����
	int				m_iMaxLogSize;		// �ϳ��� �α� ���Ͽ� ������ �� �ִ� �ִ� ũ��
	int				m_iLogSize;				// ������� ����� �α� ũ��
	int				m_iIndex;					// �α� ���� �ε���
	int64_t		m_iMaxFolderSize;	// �α� ������ �ִ� ũ��
	int64_t		m_iFolderSize;		// �α� ���� ũ��
};

#endif
