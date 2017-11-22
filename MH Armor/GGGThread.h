#pragma once
#include <stdexcept>
#include <windows.h>
#include "Solution.h"

template< typename Return, typename Param >
class Thread
{
public:
	typedef Return (*ThreadFunc)( Param data );

	Thread( ThreadFunc, Param parameter );
	~Thread( );

	Return Join( );

private:
	Thread( const Thread< typename Return, typename Param >& other ) {} //remove copy
	void operator = ( const Thread< typename Return, typename Param >& other ) {} //remove assign


#	ifdef WIN32
	static DWORD __stdcall ThreadEntry( void* thread_obj );
	typedef HANDLE ThreadHandle;
#	else
	static void* ThreadEntry( void* thread_obj );
	typedef pthread_t ThreadHandle;
#	endif
	ThreadHandle thread_handle;

	ThreadFunc func;
	Param parameter;
	Return return_value;
};


///////////// Template Function Implementation //////////////////////

template< typename Return, typename Param >
Thread<Return, Param>::Thread( ThreadFunc func, Param parameter ) : func( func ), parameter( parameter ), thread_handle( 0 )
{
	//Create thread
#	ifdef WIN32
	thread_handle = CreateThread( NULL, 0, ThreadEntry, this, 0, NULL );
	if( thread_handle == NULL )
		throw std::runtime_error( "Can not create thread" );
#	else
	if( pthread_create( &thread_handle, NULL, ThreadEntry, this ) )
		throw std::runtime_error( "Can not create thread" );
#	endif
}

#ifdef WIN32
template< typename Return, typename Param >
DWORD __stdcall Thread<Return, Param>::ThreadEntry( void* thread_obj )
#else
template< typename Return, typename Param >
void* Thread<Return, Param>::ThreadEntry( void* thread_obj )
#endif
{
	Thread< Return, Param >* thread_object = (Thread<Return,Param>*)thread_obj;
	thread_object->return_value = thread_object->func( thread_object->parameter );
	return 0;
}

template< typename Return, typename Param >
Thread<Return, Param>::~Thread()
{
	if( !thread_handle )
		return;

	//Detach thread
#	ifdef WIN32
	CloseHandle( thread_handle );
#	else
	pthread_detach( thread_handle );
#	endif
}


template< typename Return, typename Param >
Return Thread<Return, Param>::Join()
{
	if( thread_handle == 0 )
		throw std::runtime_error( "Waited for a thread twice" );

	//Join thread
#	ifdef WIN32
	WaitForSingleObject( thread_handle, INFINITE );
	CloseHandle( thread_handle );
	thread_handle = 0;
#	else
	pthread_join( thread_handle, NULL );
	thread_handle = 0;
#	endif
	return return_value;
}
