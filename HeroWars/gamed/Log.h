#ifndef _LOG_H
#define _LOG_H
#include <iostream>
#include <fstream>
#include <string>
#include <Windows.h>

#define LOG_BUFFER_SIZE 1024
#define DO_LOG

#ifdef DO_LOG
#define PDEBUG_LOG_LINE(pLogHandler,buffer,...) pLogHandler->debugLine(buffer,__VA_ARGS__)
#define VDEBUG_LOG_LINE(LogHandler,buffer,...) LogHandler.debugLine(buffer,__VA_ARGS__)
#define PDEBUG_LOG(pLogHandler,buffer,...) pLogHandler->debug(buffer,__VA_ARGS__)
#define VDEBUG_LOG(LogHandler,buffer,...) LogHandler.debug(buffer,__VA_ARGS__)
#else
#define PDEBUG_LOG_LINE(pLogHandler,buffer,...) 
#define VDEBUG_LOG_LINE(LogHandler,buffer,...)
#define PDEBUG_LOG(pLogHandler,buffer,...)
#define VDEBUG_LOG(LogHandler,buffer,...)
#endif

using namespace std;

/*
* This class was made to simplify Logs
* As you can see we can make multiple instance of logs (if one day we make multi-party server it can be helpfull)
* but anyway we have a also a main instance of logs that can be access by getMainInstance.
* I also made PDEBUG_LOG and VDEBUG_LOG macros to remove some useless message when we'll be in release mode. 
* SYSTEMTIME is from window's API, , that broke the portability.
*/

class Log
{
public:
	enum LogType 
	{
		LT_NORMAL,
		LT_DEBUG,
		LT_ERROR
	};
	
public:
	Log();
	~Log();
	void setStream(ostream* stream);
	void setOutputFile(const char* path);
	void writeDate();
	void writeLine(const char* buffer,...);
	void write(const char* buffer,...);
	void errorLine(const char* buffer,...);
	void error(const char* buffer,...);
	void debugLine(const char* buffer,...);
	void debug(const char* buffer,...);
	void clear();
	void setLogType(LogType type);
	static Log* getMainInstance();

private:
	void vaWriteLine(const char* buffer,va_list l);
	void vaWrite(const char* buffer,va_list al);
private:
	static Log* m_instance;
	LogType m_type;
	ostream* m_stream;
	SYSTEMTIME m_time;
	char m_buffer[LOG_BUFFER_SIZE]; 
};
#endif
