/*
 * Copyright (C) 2005-2008 Mangos <http://www.mangosproject.org/>
 *
 * Copyright (C) 2008 Trinity <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef NEO_DEFINE_H
#define NEO_DEFINE_H

#include <sys/types.h>

#include <ace/Basic_Types.h>
#include <ace/ACE_export.h>

#include "Platform/CompilerDefs.h"

#define NEO_LITTLEENDIAN 0
#define NEO_BIGENDIAN    1

#if !defined(NEO_ENDIAN)
#  if defined (ACE_BIG_ENDIAN)
#    define NEO_ENDIAN NEO_BIGENDIAN
#  else //ACE_BYTE_ORDER != ACE_BIG_ENDIAN
#    define NEO_ENDIAN NEO_LITTLEENDIAN
#  endif //ACE_BYTE_ORDER
#endif //NEO_ENDIAN

#if PLATFORM == PLATFORM_WINDOWS
#  define NEO_EXPORT __declspec(dllexport)
#  define NEO_LIBRARY_HANDLE HMODULE
#  define NEO_LOAD_LIBRARY(a) LoadLibrary(a)
#  define NEO_CLOSE_LIBRARY FreeLibrary
#  define NEO_GET_PROC_ADDR GetProcAddress
#  define NEO_IMPORT __cdecl
#  define NEO_SCRIPT_EXT ".dll"
#  define NEO_SCRIPT_NAME "NeoScript"
#  define NEO_PATH_MAX MAX_PATH
#else //PLATFORM != PLATFORM_WINDOWS
#  define NEO_LIBRARY_HANDLE void*
#  define NEO_EXPORT export
#  define NEO_LOAD_LIBRARY(a) dlopen(a,RTLD_NOW)
#  define NEO_CLOSE_LIBRARY dlclose
#  define NEO_GET_PROC_ADDR dlsym
#  if defined(__APPLE_CC__) && defined(BIG_ENDIAN)
#    define NEO_IMPORT __attribute__ ((longcall))
#  else
#    define NEO_IMPORT __attribute__ ((cdecl))
#  endif //__APPLE_CC__ && BIG_ENDIAN
#  if defined(__APPLE_CC__)
#    define NEO_SCRIPT_EXT ".dylib"
#    if defined(DO_SCRIPTS)
#      define NEO_SCRIPT_NAME "../lib/libneoscript"
#    else
#      define NEO_SCRIPT_NAME "../lib/libneointerface"
#    endif // DO_SCRIPTS
#  else
#    define NEO_SCRIPT_EXT ".so"
#    if defined(DO_SCRIPTS)
#      define NEO_SCRIPT_NAME "libneoscript"
#    else
#      define NEO_SCRIPT_NAME "libneointerface"
#    endif // DO_SCRIPTS
#  endif //__APPLE_CC__
#  define NEO_PATH_MAX PATH_MAX
#endif //PLATFORM

#if PLATFORM == PLATFORM_WINDOWS
#  ifdef NEO_WIN32_DLL_IMPORT
#    define NEO_DLL_DECL __declspec(dllimport)
#  else //!NEO_WIN32_DLL_IMPORT
#    ifdef NEO_WIND_DLL_EXPORT
#      define NEO_DLL_DECL __declspec(dllexport)
#    else //!NEO_WIND_DLL_EXPORT
#      define NEO_DLL_DECL
#    endif //NEO_WIND_DLL_EXPORT
#  endif //NEO_WIN32_DLL_IMPORT
#else //PLATFORM != PLATFORM_WINDOWS
#  define NEO_DLL_DECL
#endif //PLATFORM

#if PLATFORM == PLATFORM_WINDOWS
#  define NEO_DLL_SPEC __declspec(dllexport)
#  ifndef DECLSPEC_NORETURN
#    define DECLSPEC_NORETURN __declspec(noreturn)
#  endif //DECLSPEC_NORETURN
#else //PLATFORM != PLATFORM_WINDOWS
#  define NEO_DLL_SPEC
#  define DECLSPEC_NORETURN
#endif //PLATFORM

#if !defined(DEBUG)
#  define NEO_INLINE inline
#else //DEBUG
#  if !defined(NEO_DEBUG)
#    define NEO_DEBUG
#  endif //NEO_DEBUG
#  define NEO_INLINE
#endif //!DEBUG

#if COMPILER == COMPILER_GNU
#  define ATTR_NORETURN __attribute__((noreturn))
#  define ATTR_PRINTF(F,V) __attribute__ ((format (printf, F, V)))
#else //COMPILER != COMPILER_GNU
#  define ATTR_NORETURN
#  define ATTR_PRINTF(F,V)
#endif //COMPILER == COMPILER_GNU

typedef ACE_INT64 int64;
typedef ACE_INT32 int32;
typedef ACE_INT16 int16;
typedef ACE_INT8 int8;
typedef ACE_UINT64 uint64;
typedef ACE_UINT32 uint32;
typedef ACE_UINT16 uint16;
typedef ACE_UINT8 uint8;

#if COMPILER != COMPILER_MICROSOFT
typedef uint16      WORD;
typedef uint32      DWORD;
#endif //COMPILER

typedef uint64 OBJECT_HANDLE;

#endif //NEO_DEFINE_H

