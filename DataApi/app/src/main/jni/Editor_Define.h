#ifndef _EDITOR_H_DEFINE_
#define _EDITOR_H_DEFINE_

#ifdef WIN32
#ifdef FM_SDK_EDITOR_EXPORTS
	#define EDITOR_API __declspec(dllexport)
#else
	#define EDITOR_API __declspec(dllimport)
#endif
#else
#define EDITOR_API
#endif

#endif