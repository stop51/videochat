

#include "SipPlatformDefine.h"
#include "JsonObject.h"
#include "Log.h"
#include "MemoryDebug.h"

CJsonObject::CJsonObject()
{
	m_cType = JSON_TYPE_OBJECT;
}

CJsonObject::~CJsonObject()
{
	Clear();
}


int CJsonObject::Parse( const char * pszText, int iTextLen )
{
	int iPos = -1, iParseLen;
	uint8_t	cType = 0;
	std::string	strName;

	Clear();

	for( int i = 0; i < iTextLen; ++i )
	{
		if( pszText[i] == '{' )
		{
			iPos = i + 1;
			break;
		}
	}

	if( iPos == -1 ) 
	{
		CLog::Print( LOG_ERROR, "%s { is not found", __FUNCTION__ );
		return -1;
	}

	for( int i = iPos; i < iTextLen; ++i )
	{
		if( cType == 3 )
		{
			CJsonType * pclsType = GetJsonType( pszText, iTextLen, i );

			if( pclsType )
			{
				iParseLen = pclsType->Parse( pszText + i, iTextLen - i );
				if( iParseLen == -1 )
				{
					CLog::Print( LOG_ERROR, "%s json parse error", __FUNCTION__ );
					delete pclsType;
					return -1;
				}

				m_clsMap.insert( JSON_OBJECT_MAP::value_type( strName, pclsType ) );
				cType = 0;
				i += iParseLen - 1;
			}
		}
		else if( pszText[i] == '"' )
		{
			if( cType == 0 )
			{
				iPos = i + 1;
				++cType;
			}
			else if( cType == 1 )
			{
				strName.clear();
				strName.append( pszText + iPos, i - iPos );
				++cType;
			}
		}
		else if( pszText[i] == ':' )
		{
			if( cType == 2 )
			{
				++cType;
			}
		}
		else if( pszText[i] == '}' )
		{
			if( cType == 0 )
			{
				iPos = i + 1;
				break;
			}
		}
	}

	return iPos;
}


int CJsonObject::ToString( std::string & strText, EJsonNewLine eNewLine, int iDepth )
{
	JSON_OBJECT_MAP::iterator itMap;
	std::string strBuf;

	if( m_clsMap.empty() )
	{
		strBuf.append( "{ }" );
	}
	else
	{
		if( eNewLine != E_JNL_NULL )
		{
			strBuf.append( "{\n" );
		}
		else
		{
			strBuf.append( "{" );
		}

		for( itMap = m_clsMap.begin(); itMap != m_clsMap.end(); ++itMap )
		{
			if( itMap != m_clsMap.begin() )
			{
				strBuf.append( "," );
				if( eNewLine == E_JNL_ALL ) strBuf.append( m_strNewLine );
			}

			if( eNewLine == E_JNL_ALL ) 
			{
				AddTab( strBuf, iDepth + 1 );
				strBuf.append( "\"" );
			}
			else
			{
				strBuf.append( " \"" );
			}
			
			strBuf.append( itMap->first );
			strBuf.append( "\" : " );

			JsonToString( itMap->second, strBuf, eNewLine == E_JNL_TOP ? E_JNL_NULL : eNewLine, iDepth + 1 );
		}

		if( eNewLine != E_JNL_NULL ) 
		{
			strBuf.append( m_strNewLine );
			AddTab( strBuf, iDepth );
			strBuf.append( "}" );
		}
		else
		{
			strBuf.append( " }" );
		}
	}

	strText.append( strBuf );

	return (int)strBuf.length();
}


int CJsonObject::GetStringLen( )
{
	JSON_OBJECT_MAP::iterator itMap;
	int iLen = 3;

	for( itMap = m_clsMap.begin(); itMap != m_clsMap.end(); ++itMap )
	{
		if( itMap != m_clsMap.begin() )
		{
			iLen += 1;
		}

		iLen += 6;
		iLen += (int)itMap->first.length();
		iLen += itMap->second->GetStringLen();
	}

	return iLen;
}


CJsonType * CJsonObject::Copy( )
{
	JSON_OBJECT_MAP::iterator itMap;
	CJsonObject * pclsObject = new CJsonObject();
	if( pclsObject == NULL ) return NULL;

	for( itMap = m_clsMap.begin(); itMap != m_clsMap.end(); ++itMap )
	{
		CJsonType * pclsType = itMap->second->Copy();
		if( pclsType == NULL )
		{
			delete pclsObject;
			return NULL;
		}

		pclsObject->m_clsMap.insert( JSON_OBJECT_MAP::value_type( itMap->first, pclsType ) );
	}

	return pclsObject;
}


int CJsonObject::Parse( std::string & strText )
{
	return Parse( strText.c_str(), (int)strText.length() );
}


int CJsonObject::MakeString( std::string & strText, EJsonNewLine eNewLine )
{
	strText.clear();

	return ToString( strText, eNewLine );
}


void CJsonObject::Clear()
{
	JSON_OBJECT_MAP::iterator itMap;

	for( itMap = m_clsMap.begin(); itMap != m_clsMap.end(); ++itMap )
	{
		delete itMap->second;
	}

	m_clsMap.clear();
}


bool CJsonObject::SelectStringData( const char * pszName, std::string & strValue )
{
	CJsonType * pclsType;

	strValue.clear();

	if( SelectData( pszName, &pclsType ) == false ) return false;
	if( pclsType->m_cType == JSON_TYPE_INT )
	{
		int64_t iValue = ((CJsonInt *)pclsType)->m_iValue;
		char szValue[22];

		snprintf( szValue, sizeof(szValue), LONG_LONG_FORMAT, iValue );
		strValue = szValue;
	}
	else if( pclsType->m_cType == JSON_TYPE_NULL )
	{
		// null 인 경우에는 아무 것도 입력하지 않는다.
	}
	else
	{
		if( pclsType->m_cType != JSON_TYPE_STRING )
		{
			CLog::Print( LOG_ERROR, "%s name(%s)'s type is not string (%s)", __FUNCTION__, pszName, pclsType->GetTypeString() );
			return false;
		}

		strValue = ((CJsonString *)pclsType)->m_strValue;
	}

	return true;
}


bool CJsonObject::SelectData( const char * pszName, std::string & strValue )
{
	CJsonType * pclsType;

	if( SelectData( pszName, &pclsType ) == false ) return false;
	if( pclsType->m_cType != JSON_TYPE_STRING )
	{
		if( pclsType->m_cType != JSON_TYPE_NULL && pclsType->m_cType != JSON_TYPE_NUMBER )
		{
			CLog::Print( LOG_ERROR, "%s name(%s)'s type is not string (%s)", __FUNCTION__, pszName, pclsType->GetTypeString() );
		}

		return false;
	}

	strValue = ((CJsonString *)pclsType)->m_strValue;

	return true;
}


bool CJsonObject::SelectData( const char * pszName, int32_t & iValue )
{
	CJsonType * pclsType;

	if( SelectData( pszName, &pclsType ) == false ) return false;
	if( pclsType->m_cType != JSON_TYPE_INT )
	{
		if( pclsType->m_cType != JSON_TYPE_NULL )
		{
			CLog::Print( LOG_ERROR, "%s name(%s)'s type is not int (%s)", __FUNCTION__, pszName, pclsType->GetTypeString() );
		}

		return false;
	}

	iValue = (int32_t)(((CJsonInt *)pclsType)->m_iValue);

	return true;
}


bool CJsonObject::SelectData( const char * pszName, int64_t & iValue )
{
	CJsonType * pclsType;

	if( SelectData( pszName, &pclsType ) == false ) return false;
	if( pclsType->m_cType != JSON_TYPE_INT )
	{
		if( pclsType->m_cType != JSON_TYPE_NULL )
		{
			CLog::Print( LOG_ERROR, "%s name(%s)'s type is not int (%s)", __FUNCTION__, pszName, pclsType->GetTypeString() );
		}

		return false;
	}

	iValue = ((CJsonInt *)pclsType)->m_iValue;

	return true;
}


bool CJsonObject::SelectData( const char * pszName, bool & bValue )
{
	CJsonType * pclsType;

	if( SelectData( pszName, &pclsType ) == false ) return false;
	if( pclsType->m_cType != JSON_TYPE_BOOL )
	{
		if( pclsType->m_cType != JSON_TYPE_NULL )
		{
			CLog::Print( LOG_ERROR, "%s name(%s)'s type is not bool (%s)", __FUNCTION__, pszName, pclsType->GetTypeString() );
		}

		return false;
	}

	bValue = ((CJsonBool *)pclsType)->m_bValue;

	return true;
}


bool CJsonObject::SelectData( const char * pszName, CJsonObject ** ppclsObject )
{
	CJsonType * pclsType;

	if( SelectData( pszName, &pclsType ) == false ) return false;
	if( pclsType->m_cType != JSON_TYPE_OBJECT )
	{
		if( pclsType->m_cType != JSON_TYPE_NULL )
		{
			CLog::Print( LOG_ERROR, "%s name(%s)'s type is not object (%s)", __FUNCTION__, pszName, pclsType->GetTypeString() );
		}

		return false;
	}

	*ppclsObject = (CJsonObject *)pclsType;

	return true;
}


bool CJsonObject::SelectData( const char * pszName, CJsonArray ** ppclsArray )
{
	CJsonType * pclsType;

	if( SelectData( pszName, &pclsType ) == false ) return false;
	if( pclsType->m_cType != JSON_TYPE_ARRAY )
	{
		if( pclsType->m_cType != JSON_TYPE_NULL )
		{
			CLog::Print( LOG_ERROR, "%s name(%s)'s type is not array (%s)", __FUNCTION__, pszName, pclsType->GetTypeString() );
		}

		return false;
	}

	*ppclsArray = (CJsonArray *)pclsType;

	return true;
}


bool CJsonObject::SelectData( const char * pszName, CJsonType ** ppclsType )
{
	JSON_OBJECT_MAP::iterator itMap;

	itMap = m_clsMap.find( pszName );
	if( itMap == m_clsMap.end() )
	{
		CLog::Print( LOG_DEBUG, "%s name(%s) is not found", __FUNCTION__, pszName );
		return false;
	}

	*ppclsType = itMap->second;

	return true;
}


bool CJsonObject::InsertData( const char * pszName, const std::string & strValue )
{
	return InsertData( pszName, strValue.c_str() );
}


bool CJsonObject::InsertData( const char * pszName, const char * pszValue )
{
	if( Exist( pszName ) )
	{
		CLog::Print( LOG_ERROR, "%s name(%s) is found", __FUNCTION__, pszName );
		return false;
	}

	CJsonString * pclsString = new CJsonString();
	if( pclsString == NULL )
	{
		CLog::Print( LOG_ERROR, "%s new error", __FUNCTION__ );
		return false;
	}

	pclsString->m_strValue = pszValue;
	m_clsMap.insert( JSON_OBJECT_MAP::value_type( pszName, pclsString ) );

	return true;
}


bool CJsonObject::InsertData( const char * pszName, int32_t iValue )
{
	return InsertData( pszName, (int64_t)iValue );
}


bool CJsonObject::InsertData( const char * pszName, int64_t iValue )
{
	if( Exist( pszName ) )
	{
		CLog::Print( LOG_ERROR, "%s name(%s) is found", __FUNCTION__, pszName );
		return false;
	}

	CJsonInt * pclsInt = new CJsonInt();
	if( pclsInt == NULL )
	{
		CLog::Print( LOG_ERROR, "%s new error", __FUNCTION__ );
		return false;
	}

	pclsInt->m_iValue = iValue;
	m_clsMap.insert( JSON_OBJECT_MAP::value_type( pszName, pclsInt ) );

	return true;
}


bool CJsonObject::InsertData( const char * pszName, bool bValue )
{
	if( Exist( pszName ) )
	{
		CLog::Print( LOG_ERROR, "%s name(%s) is found", __FUNCTION__, pszName );
		return false;
	}

	CJsonBool * pclsBool = new CJsonBool();
	if( pclsBool == NULL )
	{
		CLog::Print( LOG_ERROR, "%s new error", __FUNCTION__ );
		return false;
	}

	pclsBool->m_bValue = bValue;
	m_clsMap.insert( JSON_OBJECT_MAP::value_type( pszName, pclsBool ) );

	return true;
}


bool CJsonObject::InsertData( const char * pszName, CJsonType * pclsType )
{
	if( Exist( pszName ) )
	{
		CLog::Print( LOG_ERROR, "%s name(%s) is found", __FUNCTION__, pszName );
		return false;
	}

	CJsonType * pclsNew = pclsType->Copy();
	if( pclsNew == NULL )
	{
		CLog::Print( LOG_ERROR, "%s Copy error", __FUNCTION__ );
		return false;
	}

	m_clsMap.insert( JSON_OBJECT_MAP::value_type( pszName, pclsNew ) );

	return true;
}


bool CJsonObject::InsertData( const char * pszName )
{
	if( Exist( pszName ) )
	{
		CLog::Print( LOG_ERROR, "%s name(%s) is found", __FUNCTION__, pszName );
		return false;
	}

	CJsonNull * pclsNew = new CJsonNull();
	if( pclsNew == NULL )
	{
		CLog::Print( LOG_ERROR, "%s new error", __FUNCTION__ );
		return false;
	}

	m_clsMap.insert( JSON_OBJECT_MAP::value_type( pszName, pclsNew ) );

	return true;
}


bool CJsonObject::InsertStringData( const char * pszName, int32_t iValue )
{
	char szValue[11];

	snprintf( szValue, sizeof(szValue), "%d", iValue );

	return InsertData( pszName, szValue );
}


bool CJsonObject::InsertStringOrNullData( const char * pszName, std::string & strValue )
{
	return InsertStringOrNullData( pszName, strValue.c_str() );
}


bool CJsonObject::InsertStringOrNullData( const char * pszName, const char * pszValue )
{
	if( pszValue == NULL || !strcmp( pszValue, "null" ) )
	{
		InsertData( pszName );
	}

	return InsertData( pszName, pszValue );
}


bool CJsonObject::UpdateData( const char * pszName, const std::string & strValue )
{
	return UpdateData( pszName, strValue.c_str() );
}


bool CJsonObject::UpdateData( const char * pszName, const char * pszValue )
{
	JSON_OBJECT_MAP::iterator itMap;
	bool bRes = false;

	itMap = m_clsMap.find( pszName );
	if( itMap != m_clsMap.end() )
	{
		if( itMap->second->m_cType == JSON_TYPE_STRING )
		{
			((CJsonString *)itMap->second)->m_strValue = pszValue;
			bRes = true;
		}
		else
		{
			CJsonString * pclsString = new CJsonString();
			if( pclsString == NULL )
			{
				CLog::Print( LOG_ERROR, "%s new error", __FUNCTION__ );
			}
			else
			{
				pclsString->m_strValue = pszValue;

				delete itMap->second;
				itMap->second = pclsString;

				bRes = true;
			}
		}
	}

	return bRes;
}


bool CJsonObject::UpdateData( const char * pszName, int32_t iValue )
{
	return UpdateData( pszName, (int64_t)iValue );
}


bool CJsonObject::UpdateData( const char * pszName, int64_t iValue )
{
	JSON_OBJECT_MAP::iterator itMap;
	bool bRes = false;

	itMap = m_clsMap.find( pszName );
	if( itMap != m_clsMap.end() )
	{
		if( itMap->second->m_cType == JSON_TYPE_INT )
		{
			((CJsonInt *)itMap->second)->m_iValue = iValue;
			bRes = true;
		}
		else
		{
			CJsonInt * pclsInt = new CJsonInt();
			if( pclsInt == NULL )
			{
				CLog::Print( LOG_ERROR, "%s new error", __FUNCTION__ );
			}
			else
			{
				pclsInt->m_iValue = iValue;

				delete itMap->second;
				itMap->second = pclsInt;

				bRes = true;
			}
		}
	}

	return bRes;
}


bool CJsonObject::UpdateData( const char * pszName, bool bValue )
{
	JSON_OBJECT_MAP::iterator itMap;
	bool bRes = false;

	itMap = m_clsMap.find( pszName );
	if( itMap != m_clsMap.end() )
	{
		if( itMap->second->m_cType == JSON_TYPE_BOOL )
		{
			((CJsonBool *)itMap->second)->m_bValue = bValue;
		}
		else
		{
			CJsonBool * pclsBool = new CJsonBool();
			if( pclsBool == NULL )
			{
				CLog::Print( LOG_ERROR, "%s new error", __FUNCTION__ );
			}
			else
			{
				pclsBool->m_bValue = bValue;

				delete itMap->second;
				itMap->second = pclsBool;

				bRes = true;
			}
		}
	}

	return bRes;
}


bool CJsonObject::UpdateData( const char * pszName, CJsonType * pclsType )
{
	JSON_OBJECT_MAP::iterator itMap;
	bool bRes = false;

	itMap = m_clsMap.find( pszName );
	if( itMap != m_clsMap.end() )
	{
		CJsonType * pclsNew = pclsType->Copy();
		if( pclsNew == NULL )
		{
			CLog::Print( LOG_ERROR, "%s Copy error", __FUNCTION__ );
		}
		else
		{
			delete itMap->second;
			itMap->second = pclsNew;

			bRes = true;
		}
	}

	return bRes;
}


bool CJsonObject::UpdateData( const char * pszName )
{
	JSON_OBJECT_MAP::iterator itMap;
	bool bRes = false;

	itMap = m_clsMap.find( pszName );
	if( itMap != m_clsMap.end() )
	{
		if( itMap->second->m_cType == JSON_TYPE_NULL )
		{
			bRes = true;
		}
		else
		{
			CJsonNull * pclsNew = new CJsonNull();
			if( pclsNew == NULL )
			{
				CLog::Print( LOG_ERROR, "%s new error", __FUNCTION__ );
			}
			else
			{
				delete itMap->second;
				itMap->second = pclsNew;

				bRes = true;
			}
		}
	}

	return bRes;
}


bool CJsonObject::DeleteData( const char * pszName )
{
	JSON_OBJECT_MAP::iterator itMap;
	bool bRes = false;

	itMap = m_clsMap.find( pszName );
	if( itMap != m_clsMap.end() )
	{
		delete itMap->second;
		m_clsMap.erase( itMap );
		bRes = true;
	}

	return bRes;
}


bool CJsonObject::Exist( const char * pszName )
{
	JSON_OBJECT_MAP::iterator itMap;

	itMap = m_clsMap.find( pszName );
	if( itMap == m_clsMap.end() )
	{
		return false;
	}

	return true;
}


CJsonType * CJsonObject::GetJsonType( const char * pszText, int iTextLen, int iPos )
{
	CJsonType * pclsType = NULL;

	if( pszText[iPos] == '"' || pszText[iPos] == '\'' )
	{
		pclsType = new CJsonString();
		if( pclsType == NULL )
		{
			CLog::Print( LOG_ERROR, "%s new error", __FUNCTION__ );
			return NULL;
		}
	}
	else if( pszText[iPos] == '{' )
	{
		pclsType = new CJsonObject();
		if( pclsType == NULL )
		{
			CLog::Print( LOG_ERROR, "%s new error", __FUNCTION__ );
			return NULL;
		}
	}
	else if( pszText[iPos] == '[' )
	{
		pclsType = new CJsonArray();
		if( pclsType == NULL )
		{
			CLog::Print( LOG_ERROR, "%s new error", __FUNCTION__ );
			return NULL;
		}
	}
	else if( isdigit((uint8_t)pszText[iPos]) || ( pszText[iPos] == '-' && isdigit(pszText[iPos+1]) ) )
	{
		CJsonNumber clsNumber;

		int iParseLen = clsNumber.Parse( pszText + iPos, iTextLen - iPos );
		if( iParseLen == -1 )
		{
			CLog::Print( LOG_ERROR, "%s json number parse error", __FUNCTION__ );
			return NULL;
		}

		if( clsNumber.IsDouble() )
		{
			pclsType = new CJsonNumber();
			if( pclsType == NULL )
			{
				CLog::Print( LOG_ERROR, "%s new error", __FUNCTION__ );
				return NULL;
			}
		}
		else
		{
			pclsType = new CJsonInt();
			if( pclsType == NULL )
			{
				CLog::Print( LOG_ERROR, "%s new error", __FUNCTION__ );
				return NULL;
			}
		}
	}
	else if( pszText[iPos] == 't' || pszText[iPos] == 'f' )
	{
		pclsType = new CJsonBool();
		if( pclsType == NULL )
		{
			CLog::Print( LOG_ERROR, "%s new error", __FUNCTION__ );
			return NULL;
		}
	}
	else if( pszText[iPos] == 'n' )
	{
		pclsType = new CJsonNull();
		if( pclsType == NULL )
		{
			CLog::Print( LOG_ERROR, "%s new error", __FUNCTION__ );
			return NULL;
		}
	}

	return pclsType;
}


void CJsonObject::JsonToString( CJsonType * pclsType, std::string & strText, EJsonNewLine eNewLine, int iDepth )
{
	switch( pclsType->m_cType )
	{
	case JSON_TYPE_STRING:
		((CJsonString *)pclsType)->ToString( strText );
		break;
	case JSON_TYPE_NUMBER:
		((CJsonNumber *)pclsType)->ToString( strText );
		break;
	case JSON_TYPE_INT:
		((CJsonInt *)pclsType)->ToString( strText );
		break;
	case JSON_TYPE_OBJECT:
		((CJsonObject *)pclsType)->ToString( strText, eNewLine, iDepth );
		break;
	case JSON_TYPE_ARRAY:
		((CJsonArray *)pclsType)->ToString( strText, eNewLine, iDepth );
		break;
	case JSON_TYPE_BOOL:
		((CJsonBool *)pclsType)->ToString( strText );
		break;
	case JSON_TYPE_NULL:
		((CJsonNull *)pclsType)->ToString( strText );
		break;
	}
}


void CJsonObject::AddTab( std::string & strText, int iDepth )
{
	for( int i = 0; i < iDepth; ++i )
	{
		strText.append( "\t" );
	}
}
