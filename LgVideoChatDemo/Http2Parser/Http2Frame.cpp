

#include "Http2Define.h"
#include "Http2Frame.h"
#include "Http2Settings.h"
#include "SipTcp.h"
#include "MemoryDebug.h"

CHttp2Frame::CHttp2Frame() : m_pszPacket(NULL), m_iPacketLen(0), m_iPacketSize(0)
{
}

CHttp2Frame::~CHttp2Frame()
{
	Delete();
}


bool CHttp2Frame::Set( uint8_t cType, uint8_t cFlag, uint32_t iStreamIdentifier, uint8_t * pszBody, int iBodyLen )
{
	if( ( 9 + iBodyLen ) > m_iPacketSize )
	{
		Delete();
	}

	if( m_pszPacket == NULL )
	{
		m_iPacketSize = 9 + iBodyLen;

		m_pszPacket = (uint8_t *)malloc( m_iPacketSize );
		if( m_pszPacket == NULL )
		{
			CLog::Print( LOG_ERROR, "%s malloc(%d) error(%d)", __FUNCTION__, m_iPacketSize, GetError() );
			Delete();
			return false;
		}
	}

	int iTemp = htonl( iBodyLen );
	memcpy( m_pszPacket, (char *)&iTemp + 1, 3 );
	m_pszPacket[3] = cType;
	m_pszPacket[4] = cFlag;

	iTemp = htonl( iStreamIdentifier );
	memcpy( m_pszPacket + 5, &iTemp, 4 );

	if( iBodyLen > 0 )
	{
		memcpy( m_pszPacket + 9, pszBody, iBodyLen );
	}

	m_iPacketLen = 9 + iBodyLen;

	return true;
}


bool CHttp2Frame::Set( uint8_t * pszPacket, int iPacketLen )
{
	if( iPacketLen > m_iPacketSize )
	{
		Delete();
	}

	if( m_pszPacket == NULL )
	{
		m_iPacketSize = iPacketLen;

		m_pszPacket = (uint8_t *)malloc( m_iPacketSize );
		if( m_pszPacket == NULL )
		{
			CLog::Print( LOG_ERROR, "%s malloc(%d) error(%d)", __FUNCTION__, m_iPacketSize, GetError() );
			Delete();
			return false;
		}
	}

	memcpy( m_pszPacket, pszPacket, iPacketLen );
	m_iPacketLen = iPacketLen;

	return true;
}


bool CHttp2Frame::SetFlags( uint8_t cFlag )
{
	if( m_iPacketLen >= 9 )
	{
		m_pszPacket[4] = cFlag;
		return true;
	}

	return false;
}


uint8_t CHttp2Frame::GetType()
{
	if( m_iPacketLen >= 9 ) return m_pszPacket[3];

	return 0xFF;
}


uint8_t CHttp2Frame::GetFlags()
{
	if( m_iPacketLen >= 9 ) return m_pszPacket[4];

	return 0x00;
}


uint32_t CHttp2Frame::GetStreamIdentifier()
{
	uint32_t iId;

	memcpy( &iId, m_pszPacket + 5, 4 );

	return ntohl( iId );
}


uint8_t * CHttp2Frame::GetBody()
{
	if( m_iPacketLen > 9 ) 
	{
		if( m_pszPacket[4] & HTTP2_FLAG_PRIORITY )
		{
			if( m_iPacketLen > 14 )
			{
				return m_pszPacket + 14;
			}

			return NULL;
		}

		return m_pszPacket + 9;
	}

	return NULL;
}


int CHttp2Frame::GetBodyLen()
{
	if( m_iPacketLen > 9 )
	{
		if( m_pszPacket[4] & HTTP2_FLAG_PRIORITY )
		{
			if( m_iPacketLen > 14 )
			{
				return m_iPacketLen - 14;
			}

			return 0;
		}

		return m_iPacketLen - 9;
	}

	return 0;
}


void CHttp2Frame::Clear()
{
	m_iPacketLen = 0;
}


void CHttp2Frame::Delete()
{
	if( m_pszPacket )
	{
		free( m_pszPacket );
		m_pszPacket = NULL;
	}

	m_iPacketSize = 0;
	m_iPacketLen = 0;
}


void CHttp2Frame::PrintLog( EnumLogLevel eLogLevel, const char * pszIp, int iPort, bool bSend )
{
	if( m_iPacketLen >= 9 && CLog::IsPrintLogLevel( eLogLevel ) )
	{
		int iFrameLen = 0, iFrameId;

		memcpy( (char *)&iFrameLen + 1, m_pszPacket, 3 );
		iFrameLen = ntohl( iFrameLen );

		memcpy( &iFrameId, m_pszPacket + 5, 4 );
		iFrameId = ntohl( iFrameId );

		CLog::Print( eLogLevel, "%s(%s:%d) frame( len=%d, type=%0x, flag=%0x, id=%d )", bSend ? "Send" : "Recv", pszIp, iPort, iFrameLen, m_pszPacket[3], m_pszPacket[4], iFrameId );

		switch( m_pszPacket[3] )
		{
		case HTTP2_FRAME_TYPE_SETTINGS:
			CHttp2Settings::PrintLog( eLogLevel, m_pszPacket + 9, iFrameLen );
			break;
		}
	}
}
