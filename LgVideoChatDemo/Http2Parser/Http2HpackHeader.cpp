

#include "Http2HpackHeader.h"
#include "Http2HuffmanCode.h"
#include <stdlib.h>
#include "MemoryDebug.h"

CHttp2HpackHeader::CHttp2HpackHeader() : m_iIndex(0), m_bIncrementalIndexing(false), m_pszBuf(NULL), m_iBufSize(0)
{
}

CHttp2HpackHeader::~CHttp2HpackHeader()
{
	if( m_pszBuf )
	{
		free( m_pszBuf );
		m_pszBuf = NULL;
	}
}


int CHttp2HpackHeader::Parse( const uint8_t * pszBody, int iBodyLen )
{
	int n, iBodyPos = 0;

	Clear();

	if( pszBody[0] & 0x80 )
	{
		n = ParseInt( pszBody, iBodyLen, 1, m_iIndex );
		if( n <= 0 ) return -1;

		++iBodyPos;
	}
	else if( pszBody[0] == 0x40 )
	{
		++iBodyPos;

		n = ParseString( pszBody + iBodyPos, iBodyLen - iBodyPos, m_strName );
		if( n <= 0 ) return -1;
		iBodyPos += n;

		n = ParseString( pszBody + iBodyPos, iBodyLen - iBodyPos, m_strValue );
		if( n <= 0 ) return -1;
		iBodyPos += n;

		m_bIncrementalIndexing = true;
	}
	else if( pszBody[0] & 0x40 )
	{
		n = ParseInt( pszBody, iBodyLen, 2, m_iIndex );
		if( n <= 0 ) return -1;
		iBodyPos += n;

		n = ParseString( pszBody + iBodyPos, iBodyLen - iBodyPos, m_strValue );
		if( n <= 0 ) return -1;
		iBodyPos += n;

		m_bIncrementalIndexing = true;
	}
	else if( pszBody[0] == 0x00 )
	{
		++iBodyPos;

		n = ParseString( pszBody + iBodyPos, iBodyLen - iBodyPos, m_strName );
		if( n <= 0 ) return -1;
		iBodyPos += n;

		n = ParseString( pszBody + iBodyPos, iBodyLen - iBodyPos, m_strValue );
		if( n <= 0 ) return -1;
		iBodyPos += n;
	}
	else if( pszBody[0] & 0x0F )
	{
		n = ParseInt( pszBody, iBodyLen, 4, m_iIndex );
		if( n <= 0 ) return -1;
		iBodyPos += n;

		n = ParseString( pszBody + iBodyPos, iBodyLen - iBodyPos, m_strValue );
		if( n <= 0 ) return -1;
		iBodyPos += n;
	}

	return iBodyPos;
}


int CHttp2HpackHeader::ParseString( const uint8_t * pszBody, int iBodyLen, std::string & strOutput )
{
	int n, iBodyPos = 0, iLen;
	bool bHuffmanCode = false;

	strOutput.clear();

	if( pszBody[0] & 0x80 ) bHuffmanCode = true;

	n = ParseInt( pszBody, iBodyLen, 1, iLen );
	if( n <= 0 ) return -1;
	iBodyPos += n;

	if( iBodyPos + iLen > iBodyLen ) return -1;

	if( bHuffmanCode )
	{
		if( iLen * 3 > m_iBufSize )
		{
			if( m_pszBuf )
			{
				free( m_pszBuf );
				m_pszBuf = NULL;
				m_iBufSize = 0;
			}
		}

		if( m_pszBuf == NULL )
		{
			m_pszBuf = (uint8_t *)malloc( iLen * 3 );
			if( m_pszBuf == NULL ) return -1;

			m_iBufSize = iLen * 3;
		}

		n = HuffmanCodeDecode( pszBody + iBodyPos, iLen, m_pszBuf, m_iBufSize );
		if( n == -1 ) return -1;

		strOutput.append( (char *)m_pszBuf, n );
	}
	else
	{
		strOutput.append( (char *)pszBody + iBodyPos, iLen );
	}
	iBodyPos += iLen;

	return iBodyPos;
}


int CHttp2HpackHeader::ParseInt( const uint8_t * pszBody, int iBodyLen, int iPrefixBit, int & iOutput )
{
	uint8_t cMax = 0xFF >> iPrefixBit;
	uint8_t cCur = 0;
	int iPos = 0, iMulti = 1;

	iOutput = 0;

	for( ; iPos < iBodyLen; ++iPos )
	{
		switch( iPrefixBit )
		{
		case 1: cCur = pszBody[iPos] & 0x7F; break;
		case 2: cCur = pszBody[iPos] & 0x3F; break;
		case 3: cCur = pszBody[iPos] & 0x1F; break;
		case 4: cCur = pszBody[iPos] & 0x0F; break;
		case 5: cCur = pszBody[iPos] & 0x07; break;
		case 6: cCur = pszBody[iPos] & 0x03; break;
		case 7: cCur = pszBody[iPos] & 0x01; break;
		}

		iOutput += cCur * iMulti;

		if( cMax == cCur )
		{
			if( iPos && ( ( pszBody[iPos] >> 7 ) & 0x00 ) ) break;

			iPrefixBit = 1;
			cMax = 0xFF >> iPrefixBit;

			if( iPos >= 1 ) iMulti *= 128;
		}
		else
		{
			++iPos;
			break;
		}
	}

	return iPos;
}


void CHttp2HpackHeader::Clear()
{
	m_iIndex = 0;
	m_strName.clear();
	m_strValue.clear();
	m_bIncrementalIndexing = false;
}
