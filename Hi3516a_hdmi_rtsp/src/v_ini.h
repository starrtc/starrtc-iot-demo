#ifndef _ini__
#define _ini__

#include <string>
#include <fstream>
#include <map>
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>

using std::string;
using std::fstream;
using std::map;

class ini_reader
{
public:
	ini_reader( void )
	{
	}
	~ini_reader( )
	{
	}
public:
	int load_ini( const char* name )
	{
		int     pos;
		string  line;
		fstream fs( name );
		m_key_val.clear( );

		if( !fs )
		{
			return -1;
		}

		while( getline( fs, line ) )
		{
			pos = line.find( "=" );
			if( 0 < pos )
			{
				string key; 
				string val;
				int    i = 0;

				if( string::npos != line.find( "\r" ) )
				{
					i = 1;
				}
				else
				{
					i = 0;
				}

				key = line.substr( 0, pos );
				val = line.substr( pos + 1, line.length() - pos - 1 - i );

				m_key_val.insert( std::make_pair( key, val ) );
			}
		}
		return m_key_val.size() ? 1 :-1;
	}
	int save_ini( const char* name )
	{
		FILE* f = fopen( name, "w+" );
		if( !f )
		{
			char info[256] = { 0 };
			sprintf( info, "\nfopen:%s failed", name );
			return -1;
		}
		map<string,string>::iterator iter = m_key_val.begin( );
		while( iter != m_key_val.end( ) )
		{
			char buf[128] = { 0 };
			sprintf( buf, "%s=%s\r\n", iter->first.c_str(), iter->second.c_str() );
			fwrite( buf, strlen((const char*)(buf)), 1, f );
			++iter;
		}
		fclose( f );
	}
	int set_value( const char* key, string& val )
	{
		map<string,string>::iterator iter = m_key_val.find( string( key ) );
		if( iter == m_key_val.end() )
		{
			m_key_val.insert( std::make_pair<string,string>( (string)(key), val ) );
		}
		else
		{
			iter->second = val;
		}
		return 0;
	}
	int get_value( const char* key, string& val )
	{
		map<string,string>::iterator iter = m_key_val.find( string( key ) );
		if( iter == m_key_val.end() )
		{
			return -1;
		}
		else
		{
			val = iter->second;
			return 0;
		}
	}
private:
	map<string,string> m_key_val;
};
#endif
