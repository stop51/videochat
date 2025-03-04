

#include "Http2Define.h"
#include "Http2Header.h"
#include <stdlib.h>
#include "Log.h"
#include "MemoryDebug.h"

CHttp2Header::CHttp2Header() : m_pszPacket(NULL), m_iPacketLen(0), m_iPacketSize(0)
{
}

CHttp2Header::~CHttp2Header()
{
	Delete();
}


bool CHttp2Header::AddIndex( uint32_t iIndex )
{
	int iIndexLen = AddInt( 0x80, 1, iIndex );
	if( iIndexLen == 0 ) return false;

	m_iPacketLen += iIndexLen;

	return true;
}


bool CHttp2Header::AddIndexValue( uint32_t iIndex, const char * pszValue, bool bIndexing )
{
	int iIndexLen;
	
	if( bIndexing )
	{
		iIndexLen = AddInt( 0x40, 2, iIndex );
		if( iIndexLen == 0 ) return false;
	}
	else
	{
		iIndexLen = AddInt( 0x00, 4, iIndex );
		if( iIndexLen == 0 ) return false;
	}

	m_iPacketLen += iIndexLen;

	int iValueLen = (int)strlen( pszValue );
	int iIntLen = AddInt( 0x00, 1, iValueLen );
	if( iIntLen == 0 )
	{
		m_iPacketLen -= iIndexLen;
		return false;
	}

	if( ( m_iPacketLen + iIntLen + iValueLen ) > m_iPacketSize )
	{
		m_iPacketLen -= iIndexLen;
		return false;
	}

	m_iPacketLen += iIntLen;
	memcpy( m_pszPacket + m_iPacketLen, pszValue, iValueLen );
	m_iPacketLen += iValueLen;

	return true;
}


bool CHttp2Header::AddNameValue( const char * pszName, const char * pszValue, bool bIndexing )
{
	int iIntLen;
	
	if( bIndexing )
	{
		iIntLen = AddInt( 0x40, 2, 0 );
		if( iIntLen == 0 ) return false;
	}
	else
	{
		iIntLen = AddInt( 0x00, 4, 0 );
		if( iIntLen == 0 ) return false;
	}

	int iPacketLen = iIntLen;
	m_iPacketLen += iIntLen;
	
	int iNameLen = (int)strlen( pszName );
	iIntLen = AddInt( 0x00, 1, iNameLen );
	if( iIntLen == 0 )
	{
		m_iPacketLen -= iPacketLen;
		return false;
	}

	if( ( m_iPacketLen + iPacketLen + iIntLen + iNameLen ) > m_iPacketSize )
	{
		m_iPacketLen -= iPacketLen;
		return false;
	}

	m_iPacketLen += iIntLen;
	iPacketLen += iIntLen;

	memcpy( m_pszPacket + m_iPacketLen, pszName, iNameLen );
	m_iPacketLen += iNameLen;
	iPacketLen += iNameLen;

	int iValueLen = (int)strlen( pszValue );
	iIntLen = AddInt( 0x00, 1, iValueLen );
	if( iIntLen == 0 )
	{
		m_iPacketLen -= iPacketLen;
		return false;
	}

	if( ( m_iPacketLen + iPacketLen + iIntLen + iValueLen ) > m_iPacketSize )
	{
		m_iPacketLen -= iPacketLen;
		return false;
	}

	m_iPacketLen += iIntLen;
	memcpy( m_pszPacket + m_iPacketLen, pszValue, iValueLen );
	m_iPacketLen += iValueLen;

	return true;
}


void CHttp2Header::Clear()
{
	m_iPacketLen = 0;
}


void CHttp2Header::Delete()
{
	if( m_pszPacket )
	{
		free( m_pszPacket );
		m_pszPacket = NULL;
	}

	m_iPacketSize = 0;
	m_iPacketLen = 0;
}


bool CHttp2Header::Create()
{
	if( m_pszPacket == NULL )
	{
		m_pszPacket = (uint8_t *)malloc( HTTP2_HEADER_SIZE );
		if( m_pszPacket == NULL )
		{
			CLog::Print( LOG_ERROR, "%s malloc error", __FUNCTION__ );
			return false;
		}

		m_iPacketSize = HTTP2_HEADER_SIZE;
		m_iPacketLen = 0;
	}

	return true;
}


int CHttp2Header::AddInt( uint8_t cPrefix, uint8_t cPrefixBit, uint32_t iIndex )
{
	if( Create() == false ) return 0;

	if( ( m_iPacketLen + 1 ) > m_iPacketSize )
	{
		return 0;
	}

	uint8_t cMax = 0xFF >> cPrefixBit;
	int iPos = 1;

	if( iIndex < cMax )
	{
		m_pszPacket[m_iPacketLen] = cPrefix | iIndex;
		return 1;
	}

	m_pszPacket[m_iPacketLen] = cPrefix | cMax;
	iIndex -= cMax;

	cMax = 0xFF >> 1;

	for( iPos = 1; ; ++iPos )
	{
		if( ( m_iPacketLen + iPos ) > m_iPacketSize )
		{
			return 0;
		}

		if( iIndex < cMax )
		{
			m_pszPacket[m_iPacketLen+iPos] = iIndex;
			++iPos;
			break;
		}

		m_pszPacket[m_iPacketLen+iPos] = 0xF0 + iIndex % 128;
		iIndex /= 128;
	}

	return iPos;
}
