#ifndef FM_SDK_Logger_h
#define FM_SDK_Logger_h

#include "Editor_Define.h"

#ifdef WIN32
#include <sys/stat.h>
#include <iostream>
#endif

namespace Logger
{
    EDITOR_API void    LogO(const char*  format, ...);
        
    EDITOR_API void    LogD(const char*  format, ...);
}

#endif
