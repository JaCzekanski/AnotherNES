#pragma once
#include <cstdio>
#include <cstdarg>
#include <string>
#include <ctime>
#include <windows.h>

/*
Logger::Open( const char* file );
Logger::Info( const char* format, ... );
Logger::Success( const char* format, ... );
Logger::Fail( const char* format, ... );
Logger::Error( const char* format, ... );
Logger::Debug( const char* format, ... );
*/

enum LogType
{
	LOG_INFO = 0,
	LOG_SUCCESS = 1,
	LOG_ERROR = 2,
	LOG_FATAL = 3,
	LOG_DEBUG = 4
};

class Logger
{
private:
	std::string programName;
	FILE* file;
	bool initialized;

	void Log( int type, const char* format, va_list list )
	{
		char buffer[2048];

		vsprintf(buffer, format, list );


		char _logtype[64];

		switch( type )
		{
		case LOG_INFO:
			strcpy( _logtype, "INFO" );
			break;

		case LOG_SUCCESS:
			strcpy( _logtype, "SUCCESS" );
			break;

		case LOG_ERROR:
			strcpy( _logtype, "ERROR" );
			break;

		case LOG_FATAL:
			strcpy( _logtype, "FATAL" );
			break;

		case LOG_DEBUG:
			strcpy( _logtype, "DEBUG" );
			break;

		default:
			strcpy( _logtype, "UNKNOWN" );
			break;
		}

		char _time[64];
		time_t secs = time(0);
		strftime( _time, 64, "%X", localtime( &secs ) );

		/*if (type != LOG_DEBUG) */fprintf( file, "[%s][%s]\t%s\n", _time, _logtype, buffer );
		printf( "[%s][%s]\t%s\n", _time, _logtype, buffer );
		if ( type == LOG_FATAL )
		{
			char message_buffer[4096];
			sprintf( message_buffer, "FATAL ERROR: %s", buffer );
			MessageBox(NULL, message_buffer, programName.c_str(), MB_OK);
		}

		fflush( file );

		char bigbuffer[4096];
		sprintf(bigbuffer,  "[%s][%s]\t%s\n", _time, _logtype, buffer );
		OutputDebugString( bigbuffer );
	}
	void Open( const char* filename )
	{
		file = fopen(filename, "w");
		initialized = true;
		Info("=== Log opended ===");
	}
	void Close()
	{
		Info("=== Log closed ===");
		fclose(file);
		initialized = false;
	}
public:
	Logger()
	{
		Open("log.txt");
	}
	Logger( const char* filename )
	{
		Open(filename);
	}
	~Logger()
	{
		Close();
	}
	void setProgramName(std::string name) { programName = name;  }
	void Info( const char* format, ... )
	{
		va_list list;
		va_start( list, format );
		Log( LOG_INFO, format, list );
		va_end( list );
	}
	void Success( const char* format, ... )
	{
		va_list list;
		va_start( list, format );
		Log( LOG_SUCCESS, format, list );
		va_end( list );
	}
	void Error( const char* format, ... )
	{
		va_list list;
		va_start( list, format );
		Log( LOG_ERROR, format, list );
		va_end( list );
	}
	void Fatal( const char* format, ... )
	{
		va_list list;
		va_start( list, format );
		Log( LOG_FATAL, format, list );
		va_end( list );
	}
	void Debug( const char* format, ... )
	{
//#ifdef _DEBUG
		va_list list;
		va_start( list, format );
		Log( LOG_DEBUG, format, list );
		va_end( list );
//#endif
	}
};