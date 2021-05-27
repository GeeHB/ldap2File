//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//--
//--	FILE	:	commonTypes.h
//--
//--	AUTHOR	:	Jérôme Henry-Barnaudière (JHB)
//--
//--	DATE	:	05/08/2004
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTIONS:
//--
//--	Types and structures for platform portability
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	11/10/2000 - JHB - Creation
//--
//--	05/08/2004 - JHB - Added color macros from JHBCtrl.h
//--
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#ifndef __JHB_COMMON_TYPES_h__
#define __JHB_COMMON_TYPES_h__  1

//
//	Global includes
//
#ifdef __WINDOWS_API__
#pragma warning(disable:4996)	// stricmp
#define NOMINMAX
#include <windows.h>
#include <tchar.h>
#else
#define __STDC_WANT_LIB_EXT1__ 1
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>  // needed for errno
#include <string.h> // needed for strerror
#define Sleep sleep
typedef char* LPTSTR;
typedef const char* LPCTSTR;
typedef int   BOOL;
typedef unsigned int UINT;
typedef unsigned int* PUINT;
typedef char TCHAR;
typedef const char* LPCSTR;
typedef char* LPSTR;
typedef unsigned int DWORD;
typedef unsigned short USHORT;
typedef unsigned int ULONG;
typedef unsigned char UCHAR;
#endif	//	#ifdef ___WINDOWS_API__

//
//	Global defines ...
//

#define __EXTENDED_WND_DEFS__

//
//	Numeric types
//

#ifndef PBYTE
#define PBYTE	byte*
#define LPBYTE	PBYTE
#endif	// #ifndef PBYTE

#ifndef LPVOID
#define LPVOID	void*
#endif	// #ifndef LPVOID

#ifndef __WINDOWS_API__
#define _T(sz)	sz

//
//	Strings
//
/*
#ifndef LPCTSTR
#define CHAR	char
#define LPSTR 	char*
#define LPTSTR 	LPSTR			// no UNICODE on UNIX
#define LPCTSTR	const LPSTR
#endif	// #ifndef LPCTSTR
*/

#endif	// #ifndef __WINDOWS_API__

// End Of String character
#ifndef EOS
#define EOS				_T('\0')
#endif	//	#ifndef EOS

// End of line in the windows world !!!!
#ifdef __WINDOWS_API__
#ifndef EOL
#define EOL_LEN			2
#define EOL(var)		CHAR var[EOL_LEN+1];var[0] = 13; var[1] = 10; var[2] = 0;
#endif	//	#ifndef EOL

#ifndef EOL_HTTP
#define EOL_HTTP(var)		CHAR var[EOL_LEN+1];var[0] = 10; var[1] = 10; var[2] = 0;
#endif	//	#ifndef EOL_HTTP

#endif	// #ifdef __WINDOWS_API__

#ifndef IS_EMPTY
#define IS_EMPTY(sz)	(!sz || EOS == sz[0])
#define EMPTY(sz)		{if (!IS_EMPTY(sz)) sz[0] = EOS;}
#endif // #ifndef IS_EMPTY

#ifndef CLOSE_HANDLE
#define CLOSE_HANDLE(handle)	if (handle && INVALID_HANDLE_VALUE != handle) { try { CloseHandle(handle); } catch(...) {}	handle = NULL; }
#endif // #ifndef CLOSE_HANDLE

#ifdef __WINDOWS_API__

#ifndef EQUAL_STRING
#define EQUAL_STRING(sz1,sz2,caseSens)	((NULL == sz1 && NULL == sz2) || (sz1!=NULL && sz2!=NULL && 0 == (caseSens?_tcscmp(sz1,sz2):_tcsicmp(sz1,sz2))))
#endif // #ifndef EQUAL_STRING

#ifndef EQUAL_STRING_NO_CASE
#define EQUAL_STRING_NO_CASE(sz1,sz2)	EQUAL_STRING(sz1,sz2,FALSE)
#endif // #ifndef EQUAL_STRING_NO_CASE

#ifndef TO_UNICODE
#define TO_UNICODE(szString, wString, len)	(0 != MultiByteToWideChar(CP_ACP, 0, szString, -1, wString, len))
#endif // #ifndef TO_UNICODE

#ifndef FROM_UNICODE
#define FROM_UNICODE(wString, szString, len) (0 != WideCharToMultiByte(CP_ACP, 0, wString,-1, szString, len, NULL, NULL))
#endif // FROM_UNICODE

#endif // #ifdef __WINDOWS_API__

#ifndef SET_IN_RANGE
#define SET_IN_RANGE(x,Min,Max)	(x=(x<Min)?Min:(x>Max?Max:x))
#endif	//	#ifndef SET_IN_RANGE

#ifndef MIN
#define MIN(x,y)		(x>y?y:x)
#endif // #ifndef MIN

#ifndef MAX
#define MAX(x,y)		(x>y?x:y)
#endif // #ifndef MIN

#ifndef MIN_MAX
#define MIN_MAX(val,min,max)		(val>max?max:val<min?min:val)
#endif	// #ifndef MIN_MAX

#ifndef LOCALHOST
#define LOCALHOST		_T("127.0.0.1")
#endif // #ifndef LOCAL_HOST

#ifndef _NO_COLORS_MACROS_
#ifndef GET_LIGHTER_COLOR
#define GET_LIGHTER_COLOR(col, percent)		RGB(GetRValue(col)+ (255 - GetRValue(col)) * percent, GetGValue(col)+ (255 - GetGValue(col)) * percent, GetBValue(col)+ (255 - GetBValue(col)) * percent)
#define GET_DARKER_COLOR(col, percent)		RGB(GetRValue(col) * ( 1 - percent), GetGValue(col) * ( 1 - percent), GetBValue(col) * (1 -  percent))

#define COLOR_LITE_PERCENT					0.50		// Lighter color
#define COLOR_BLEND_12						0.125
#define COLOR_BLEND_25						0.25
#define COLOR_BLEND_50						0.50
#define COLOR_DARK_PERCENT					COLOR_LITE_PERCENT
#endif // GET_LIGHTER_COLOR

#endif // #ifndef _NO_COLORS_MACROS_

#endif	// #ifndef __JHB_COMMON_TYPES_h__

// EOF
