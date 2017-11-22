#ifndef STRING_MANIPULATION_INCLUDED
#define STRING_MANIPULATION_INCLUDED

#include <sstream>
#include <string>
#include <vector>
#include <fstream>

void WstringFromString( const std::string& from, std::wstring& to);
void StringFromWstring( const std::wstring& from, std::string& to);

std::string WstringToString( const std::wstring& from );
std::wstring StringToWstring( const std::string& from );

std::wstring LowercaseString( const std::wstring& in );
std::wstring UppercaseString( const std::wstring& in );

std::wstring TrimString( const std::wstring& in );

/// Splits a given string up based on a given delimiter character.
/// @param [out] out       A forward iterator to a container to put the results
/// @param [in]  in        The input string.
/// @param [in]  delimiter The character which delimits each part of the string.
template< typename ForwardIterator >
void SplitString( ForwardIterator out, const std::wstring& in, wchar_t delimiter )
{
	if( in.size() < 2 )
		return;

	int start = 0;
	int end = 0;

	while( (end = in.find( delimiter, start )) != -1 )
	{
		const std::wstring part( in, start, end - start );

		if( !part.empty() )
		{
			*out++ = TrimString( part );
		}

		start = (end + 1);
	}

	const std::wstring left_over( in, start );

	if( !left_over.empty() )
		*out++ = TrimString( left_over );
}

inline void SplitString( std::vector<std::wstring>& out, const std::wstring& in, wchar_t delimiter )
{
	out.clear();
	SplitString( std::back_inserter( out), in, delimiter );
}

inline void SplitString( std::vector< int >& out, const std::wstring& in, wchar_t delimiter )
{
	std::vector< std::wstring > str_out;
	SplitString( str_out, in, delimiter );

	for( unsigned i = 0; i < str_out.size(); ++i )
	{
		std::wstringstream ss( str_out[ i ] );

		int amount;
		ss >> amount;

		out.push_back( amount );
	}
}


/// Checks if a given filename (possibly including path) ends with the given file extension.
/// @param [in] filename The string to check, treated as a filename
/// @param [in] extension The extension to check for. Doesn't matter if it contains a '.' character (faster if it doesn't, though)
bool FilenameHasExtension(const std::wstring& filename, const std::wstring& extension);

bool GetLine( std::wistream& stream, std::wstring& out );

//Extracts a string from a text file that is surrounded with ""
bool ExtractString( std::wistream& stream, std::wstring& output);
bool ExtractString( std::wistream& stream, std::string& output);

bool CheckString( std::wistream& stream, const std::wstring& to_check );


///Attempts to read a value from the stream and store it in the 'data' variable.
///Returns false if the operation fails, true otherwise.
template< typename T >
bool StreamData( std::wistream& stream, T& data )
{
	T temp;
	stream >> temp;
	if( stream.fail() ) return false;
	data = temp;
	return true;
}

/// Writes a wide character string to a narrow character stream.
/// @param [in] stream The stream to write the string to.
/// @param [in] str    The wide character string to write to the stream.
/// @return true if the write succeeded, false otherwise.
bool WriteWideString( std::ostream& stream, const std::wstring& str );

//Version of sprintf that takes an iterator pair and returns a wstring

bool StringFormat( const std::wstring& format_string, void **const begin, const void **const end, std::wstring& out );

bool HasTokens( std::wistream& file );
bool ReadExpected( std::wistream& file, const std::wstring& expected );
std::wstring ReadQuotedString( std::wistream& file );

/// Calculates the 32bit FNVHash of a block of data. 
/// @param[in] data The data to hash
/// @param[in] length The length to run the hash algorithm on, if no length is supplied, uses the string length
unsigned int FNVHash32( const char* data, size_t length = -1 );
/// Calculates the 16bit FNVHash of a block of data. 
/// @param[in] data The data to hash
/// @param[in] length The length to run the hash algorithm on, if no length is supplied, uses the string length
unsigned short FNVHash16( const char* data, size_t length = -1 );

unsigned short FNVHash16WideString( const std::wstring& str );

/// Adds numerical commas to a string, eg "10000000" becomes "10,000,000"
std::wstring AddNumericalCommas( const std::wstring& str );

/// Turns a int into a wstring that represents a time in minutes and seconds
std::wstring ConvertToPrintableTime( long time_in_secs );
#endif
