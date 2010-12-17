/***
*ompassem.h - Libraries Assembly information
*
*       Copyright (c) Microsoft Corporation. All rights reserved.
*
*Purpose:
*       This file has information about Libraries Assembly version.
*
*
****/

#pragma once

#ifndef _VC_ASSEMBLY_PUBLICKEYTOKEN
#define _VC_ASSEMBLY_PUBLICKEYTOKEN "1fc8b3b9a1e18e3b"
#endif

#if !defined(_BIND_TO_CURRENT_VCLIBS_VERSION)
  #define _BIND_TO_CURRENT_VCLIBS_VERSION 0
#endif

#if !defined(_BIND_TO_CURRENT_OPENMP_VERSION)
  #if _BIND_TO_CURRENT_VCLIBS_VERSION
    #define _BIND_TO_CURRENT_OPENMP_VERSION 1
  #else
    #define _BIND_TO_CURRENT_OPENMP_VERSION 0
  #endif
#endif

#ifndef __OPENMP_ASSEMBLY_VERSION
#if _BIND_TO_CURRENT_OPENMP_VERSION
#define __OPENMP_ASSEMBLY_VERSION "9.0.30729.4148"
#else
#define __OPENMP_ASSEMBLY_VERSION "9.0.21022.8"
#endif
#endif

#ifndef __OPENMP_ASSEMBLY_NAME_PREFIX
#define __OPENMP_ASSEMBLY_NAME_PREFIX "Microsoft.VC90"
#endif
