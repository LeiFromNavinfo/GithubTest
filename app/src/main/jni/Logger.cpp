#include "Logger.h"
#if defined(__APPLE__)
#import <Foundation/Foundation.h>
#endif
namespace Logger
{

#if defined(__APPLE__) | defined(LINUX)

#include <stdio.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

    void    LogO(const char*  format, ...)
    {
        char msg[1024];

        va_list argptr;

        va_start(argptr, format);

        vsnprintf(msg, 1024 ,format, argptr);

        va_end(argptr);

//          printf("%s\n",msg);
        char buf[1024];
#ifdef LINUX
        int count = readlink("/proc/self/exe", buf, 1024);

        if (count < 0 || count >= 1024)
        {
            printf("readlink failed!\n");

            return;
        }

        for (int i=count; i>=0; i--)
        {
            if (buf[i] == '/')
            {
            	buf[i+1] = '\0';

            	break;
            }
        }
        
        strcpy(buf, "Kernel.log");
#else
        NSArray *paths = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES);
        NSString *path = paths[0];
        
        path = [path stringByAppendingPathComponent:@"Kernel.log"];
        
        strcpy(buf, [path UTF8String]);
#endif
        FILE* fp;

        struct stat fs;

       

        if (stat(buf, &fs) == 0)
        {
            if (fs.st_size > 10*1024*1024)
            	remove(buf);
        }

        fp = fopen(buf, "a");

        if (!fp)
        {
            return;
        }

        time_t now = time(0);

		struct tm* tm_now;

		tm_now = localtime(&now);

		char buf_time[32];

		strftime(buf_time, sizeof(buf), "%Y%m%d-%H:%M:%S:", tm_now);

		fprintf(fp, buf_time);

		fprintf(fp, msg);

		fprintf(fp, "\n");

		fclose(fp);
    }

#ifdef __APPLE__
    void    LogD(const char*  format, ...)
    {
#if DEBUG
        
        char msg[1024];
        
        va_list argptr;
        
        va_start(argptr, format);
        
        vsnprintf(msg, 1024 ,format, argptr);
        
        va_end(argptr);
        
        printf("%s\n",msg);
#endif
    }
#else
    void    LogD(const char*  format, ...)
    {
        char msg[1024];
        
        va_list argptr;
        
        va_start(argptr, format);
        
        vsnprintf(msg, 1024 ,format, argptr);
        
        va_end(argptr);
        
        printf("%s\n",msg);
    }
#endif

#endif

#ifdef ANDROID

#include <android/log.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <time.h>
#include <string.h>


    void    LogO(const char*  format, ...)
    {
        int msgLength = 1024000;

        char* msg = (char*)malloc(sizeof(char) * msgLength);

        va_list argptr;

        va_start(argptr, format);

        vsnprintf(msg, msgLength ,format, argptr);

        va_end(argptr);

        //__android_log_write(ANDROID_LOG_DEBUG, "lincy_kernel", msg);

        FILE* fp;

        struct stat fs;

        char buf[1024];

        strcpy(buf, "/storage/sdcard0/FastMap3/Kernel.log");

//            if (stat(buf, &fs) == 0)
//            {
//            	if (fs.st_size > 10*1024*1024 * 100)
//            		remove(buf);
//            }

        fp = fopen(buf, "a");

        if (!fp)
        {
            free(msg);

            return;
        }

        time_t now = time(0);

		struct tm* tm_now;

		tm_now = localtime(&now);

		char buf_time[32];

		strftime(buf_time, sizeof(buf), "%Y%m%d-%H:%M:%S:", tm_now);

		fprintf(fp, buf_time);

		fprintf(fp, msg);

		fprintf(fp, "\n");

		fclose(fp);

		free(msg);
    }

    void    LogD(const char*  format, ...)
    {
        char msg[1024];

        va_list argptr;

        va_start(argptr, format);

        vsnprintf(msg, 1024 ,format, argptr);

        va_end(argptr);

        __android_log_write(ANDROID_LOG_DEBUG, "lincy_kernel", msg);
    }
#endif

#ifdef WIN32

#include <windows.h>

	void    LogO(const char*  format, ...)
    {
		va_list     arg;

		FILE*		fp;

		SYSTEMTIME	sysTime;

		struct _stat fs; 

		char buf[1024];

		GetModuleFileNameA(NULL, buf, 1024);
		
#ifdef _DEBUG
		strcat(buf, ".DEBUG.log");
#else
		strcat(buf, ".RELEASE.log");
#endif

		// if log file size >10M, delete the file
		if (_stat(buf, &fs) == 0)
		{
			if ( fs.st_size > 10*1024*1024)
				remove(buf);
		}	

		fp = fopen(buf, "a");

		if (!fp)
		{
			return;
		}

		GetLocalTime( &sysTime ) ;

		if ( format ) 
		{
			fprintf(fp, "%04d/%02d/%02d-%02d:%02d:%02d.%03d:",
				sysTime.wYear, sysTime.wMonth, sysTime.wDay,
				sysTime.wHour, sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds);

			va_start( arg, format );

			vfprintf( fp, format, arg );

			fprintf ( fp, "\n" );

			va_end  ( arg );
		} 
		else 
		{
			fprintf ( fp, "\n" );
		}

		fclose(fp);
    }

    void    LogD(const char*  format, ...)
    {
        va_list argptr;

        va_start(argptr, format);

		int  nBuf;
		char szBuffer[2048];

		nBuf = _vsnprintf(szBuffer, _countof(szBuffer), (const char *)format, argptr);

        va_end(argptr);

        printf("%s\n", szBuffer);
    }
#endif

}
