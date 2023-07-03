

#include "SipPlatformDefine.h"
#include "SipUdp.h"
#include "WebSocketPacket.h"
#include "MemoryDebug.h"

CWebSocketPacket::CWebSocketPacket()
{
}

CWebSocketPacket::~CWebSocketPacket()
{
}


bool CWebSocketPacket::AddPacket( const char * pszPacket, int iPacketLen )
{
	m_strBuf.append( pszPacket, iPacketLen );

	return true;
}


bool CWebSocketPacket::GetData( CWebSocketPacketHeader & clsHeader, std::string & strData )
{
	strData.clear();

	int iPacketLen = (int)m_strBuf.length();
	if( iPacketLen < 2 )
	{
		return false;
	}

	const char * pszPacket = m_strBuf.c_str();
	bool bMask = false;
	
	if( (uint8_t)pszPacket[1] & 0x80 ) bMask = true;

	clsHeader.m_iOpCode = (uint8_t)pszPacket[0] & 0x0F;

	int iPayLoadLen = (uint8_t)pszPacket[1] & 0x7F;
	int iWantLen = 0;
	int iPayLoadPos = 0;

	if( iPayLoadLen == 126 )
	{
		if( iPacketLen < 4 )
		{
			return false;
		}

		uint16_t sPayLoadLen;

		memcpy( &sPayLoadLen, pszPacket + 2, 2 );
		iPayLoadLen = ntohs( sPayLoadLen );
		iWantLen = 2 + 2 + iPayLoadLen;
		iPayLoadPos = 2 + 2;
	}
	else if( iPayLoadLen == 127 )
	{
		if( iPacketLen < 10 )
		{
			return false;
		}

		uint64_t lPayLoadLen;

		memcpy( &lPayLoadLen, pszPacket + 2, 8 );
		iPayLoadLen = ntohll( lPayLoadLen );
		iWantLen = 2 + 8 + iPayLoadLen;
		iPayLoadPos = 2 + 8;
	}
	else
	{
		iWantLen = 2 + iPayLoadLen;
		iPayLoadPos = 2;
	}

	if( bMask )
	{
		iWantLen += 4;
	}

	if( iPacketLen < iWantLen )
	{
		return false;
	}

	if( bMask )
	{
		int iMaskPos = iPayLoadPos;
		iPayLoadPos += 4;

		char * pszData = (char *)malloc(iPayLoadLen);
		if( pszData == NULL )
		{
			return false;
		}

		for( int i = 0; i < iPayLoadLen; ++i )
		{
			pszData[i] = pszPacket[iPayLoadPos+i] ^ pszPacket[iMaskPos+i%4];
		}

		strData.append( pszData, iPayLoadLen );

		free( pszData );
	}
	else
	{
		strData.append( pszPacket + iPayLoadPos, iPayLoadLen );
	}

	m_strBuf.erase( 0, iWantLen );

	return true;
}
