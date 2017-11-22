#include "stdafx.h"
#include "StringManipulation.h"

#include <algorithm>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>

using namespace std;

bool WriteWideString( ostream& stream, const wstring& str )
{
	stream.write( (char*)str.c_str(), str.length() * sizeof( wchar_t ) );

	return !stream.fail();
}

bool ExtractString( wistream &stream, wstring &str)
{
	wchar_t start;
	stream >> start;
	if( start != L'\"' )
		return false;

	getline( stream, str, L'\"' );

	if(stream.fail() || stream.eof() )
		return false;

	return true;
}

bool ExtractString( wistream& stream, string &output )
{
	wstring wide_version;
	const bool result = ExtractString( stream, wide_version );
	StringFromWstring( wide_version, output );
	return result;
}

bool CheckString( wistream &stream, const wstring &to_check )
{
	wstring token;
	stream >> token;
	return token == to_check;
}


bool StringFormat(const wstring& format_string, void *const *begin, const void *const *const end, wstring& out)
{
	wstringstream converter;
	for(size_t i = 0; i < format_string.length(); ++i)
	{
		if(format_string[i] == L'%')
		{
			++i;
			if(i == format_string.length()) return false;
			if( format_string[i] == L'%' )
			{
				converter << L'%';
			}
			else
			{
				if( begin == end )
					return false;

				switch( format_string[i] )
				{
				case L's':
					converter << *((wstring*)*begin);
					break;
				case L'd':
					converter << *((int*)*begin);
					break;
				default:
					return false;
				}

				++begin;
			}
		}
		else
		{
			converter << format_string[i];
		}
	}
	out = converter.str();
	return true;
}


/// Checks to see if there are any tokens left in the given stream.
/// NOTE: The given stream's fail() will return true if the end of the stream is reached while checking for more tokens.
/// @param [in] stream The stream to check for tokens.
/// @return true if there are tokens left in the stream, false otherwise.
bool HasTokens( wistream& stream )
{
	if( stream.fail() )
		return false;

	const streamoff pos = stream.tellg();  // Record initial stream position
	wstring token;

	stream >> token;  // Attempt to read something

	const bool success = !stream.fail();

	if( !success )
		stream.clear( stream.rdstate() & ~ios::failbit );

	stream.seekg( pos );

	return success;
}

/// Reads the next token from the given stream and compares it to the given expected string.
/// @param [in] stream   The stream to read from.
/// @param [in] expected Expected string.
/// @return true if the next token in the stream and the given expected string match, false otherwise.
bool ReadExpected( wistream& stream, const wstring& expected )
{
	wstring token;

	stream >> token;

	return !(stream.fail() || (token != expected));
}


/// Reads a quoted string from the given stream.
/// @param [in] stream The stream to read from.
/// @return The quoted string that was read from the stream (minus the quotation marks.)
wstring ReadQuotedString( wistream& stream )
{
	wstring ret;

	stream >> ret;

	if( ret[0] != L'\"' )
		return L"";

	if( (ret.size() == 1) || ret[ret.size() - 1] != L'\"' )
	{
		while( !stream.eof() )
		{
			wchar_t c = stream.get();

			ret += c;

			if (c == L'\"')
				break;
		}
	}

	if( (ret[ret.size() - 1] != '\"') || (ret.size() == 1) )
	{
		wcout << L"Quoted string doesn't have a closing quotation mark." << endl;

		return L"";
	}

	return ret.substr( 1, ret.size() - 2 );
}


// FNV-1 hash algorithm
unsigned int FNVHash32(const char* data, size_t length)
{
	unsigned int hval = 0;
	unsigned char *bp = (unsigned char *)data;	// start of buffer
	unsigned char *be = bp + (length == UINT_MAX ? strlen(data) : length); // beyond end of buffer
    
	// FNV-1 hash each octet in the buffer
	while (bp < be) 
	{
		// multiply by the 32 bit FNV magic prime mod 2^32 
		hval += (hval<<1) + (hval<<4) + (hval<<7) + (hval<<8) + (hval<<24);

		// xor the bottom with the current octet
		hval ^= *bp++;
	}

	return hval;
}

unsigned short FNVHash16(const char* data, size_t length)
{
	unsigned int hval = FNVHash32(data, length);
	return (hval >> 16) ^ (hval & 0xFFFF);
}

unsigned short FNVHash16WideString(const wstring& str)
{
	//linux and windows have different sizes for wchar_t.
	//we'll hash the non-wide version for now
	string non_wide_str;
	StringFromWstring(str, non_wide_str);

	return FNVHash16((const char*)non_wide_str.c_str(), non_wide_str.length());
}

wchar_t widen(const char c)
{
	return wcout.widen(c);
}

void WstringFromString(const string &from, wstring &to)
{
	to.resize(from.size());
	transform(from.begin(), from.end(), to.begin(), widen);
}

void StringFromWstring(const wstring &from, string &to)
{
	to.clear();
	char buffer[8];
	for(unsigned i = 0; i < from.size(); ++i)
	{
#		ifdef WIN32
			int n;
			wctomb_s( &n, buffer, 8, from[ i ] );
#		else
			const int n = wctomb(buffer, from[i]);
#		endif

		if(n > 0)
		{
			buffer[n] = 0;
			to += buffer;
		}
	}
}

string WstringToString( const wstring &from )
{
	string to;
	StringFromWstring( from, to );
	return to;
}

wstring StringToWstring( const string &from )
{
	wstring to;
	WstringFromString( from, to );
	return to;
}

wstring LowercaseString( const wstring& in )
{
	/*
	wstring toReturn = in;
	for(unsigned int i=0;i<toReturn.size();++i)
	{
		if( toReturn[ i ] >= 'A' && toReturn[i]<='Z') toReturn[i]+=32;
	}
	return toReturn;
	*/
	
	// TODO: Decide whether to use the "tolower" version below instead.
	
	const unsigned int in_length = (unsigned int)in.size();
	
	wstring to_return;
	
	to_return.resize( in_length );
	
	for( unsigned int i = 0; i < in_length; ++i )
	{
		to_return[i] = tolower( (int)in[i] );
	}
	
	return to_return;
}

wstring UppercaseString( const wstring& in )
{
	/*
	wstring toReturn = in;
	for(unsigned int i=0;i<toReturn.size();++i)
	{
		if(toReturn[i]>='a' && toReturn[i]<='z') toReturn[i]-=32;
	}
	return toReturn;
	*/
	//
	// TODO: Decide whether to use the "toupper" version below instead.
	//
	const unsigned int in_length = (unsigned int)in.size();
	
	wstring to_return;
	
	to_return.resize( in_length );
	
	for( unsigned int i = 0; i < in_length; ++i )
	{
		to_return[ i ] = toupper( (int)in[ i ] );
	}
	
	return to_return;
}

/// Given a string, returns a version that contains no white space at the beginning and end of the string.
/// @param [in] s The string to trim.
/// @return A trimmed version of the input string.
wstring TrimString( const wstring& in )
{
	if( in.length() == 0 )
		return L"";

	wstring ret;

	const wchar_t* start = &in[0];
	const wchar_t* end = (start + (in.length() - 1));
	
	while( (start < end) && ((*start == L' ') || (*start == L'\t')) )
	{ ++start; }
	
	while( (end >= start) && ((*end == L' ') || (*end == L'\t')) )
	{ --end; }
	
	const unsigned int len = (end - start + 1);
	const unsigned int byteLen = (((char*)end) - ((char*)start) + sizeof('\0'));
	
	if( !len )
		return L"";

	ret.resize( len );
	
	memcpy( &ret[0], start, byteLen );

	return ret;
}

bool FilenameHasExtension( const std::wstring& filename, const std::wstring& extension )
{
	if( filename.empty() ) return false;
	if( extension.empty() ) return true;

	const size_t ext_dot_pos = extension.rfind( L'.' );
	const wstring clean_ext = ext_dot_pos == wstring::npos ? extension : extension.substr( ext_dot_pos + 1 );
	
	if( filename.length() < clean_ext.length() + 1 ) return false;
	const size_t filename_dot_pos = filename.rfind( L'.' );
	if( filename_dot_pos == wstring::npos || filename_dot_pos + clean_ext.length() >= filename.length() ) return false;
	
	const wstring filename_ext = filename.substr( filename_dot_pos + 1 );
	return filename_ext == clean_ext;
}

/// Gets a line of text from a file.
/// Lines can be terminated by either \n or \r\n.
/// @param [in]  stream The stream to get the line from.
/// @param [out] out Reference to the string to output the line data into.
/// @return true if the function succeeded, false otherwise.
bool GetLine( wistream& stream, wstring& out )
{
	out.resize( 0 );
	wchar_t buff[256];

	stream.getline( buff, 256, L'\n' );

	if( stream.fail() )
		return false;

	out = buff;

	// If the line ends with a carriage return, remove it
	if( out[out.size() - 1] == L'\r' )
		out.resize( out.size() - 1 );

	return true;
}

std::wstring AddNumericalCommas( const std::wstring& str )
{
	const unsigned length = str.size(), number_leading_chars = length % 3;

	wstringstream end_result;

	end_result << str.substr( 0, number_leading_chars );

	for( unsigned i = number_leading_chars; i < length; i += 3 )
	{
		end_result << L"," << str.substr( i, 3 );
	}

	return end_result.str();
}

std::wstring ConvertToPrintableTime( long time_in_secs )
{
	std::wstringstream converter; 

	converter << time_in_secs / 60 << ":" << std::setw( 2 ) << std::setfill(L'0') << time_in_secs % 60;

	return converter.str();
}
