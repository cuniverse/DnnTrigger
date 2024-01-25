
/* ====================================================================
 * Copyright (c) 2007 HCI LAB. 
 * ALL RIGHTS RESERVED.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are prohibited provided that permissions by HCI LAB
 * are not given.
 *
 * ====================================================================
 *
 */

/**
 *	@file	hci_type.h
 *	@ingroup common_base_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	HCI Lab primitive types; more machine-independent.
 *
 *	Type signatures used in HCILAB PowerASR
 */


#ifndef __HCILAB_PRIM_TYPE_H__
#define __HCILAB_PRIM_TYPE_H__

#include <float.h>
#include <limits.h>

#define HCILAB_PUBLIC
#define HCILAB_PRIVATE	static

/**
 * Check current platform
 */

#if defined(__MSDOS__)
#define PC
#define PLATFORM "PC"
#define LSBFIRST
#elif defined(__osf__)
#define OSF
#define PLATFORM "OSF"
#define LSBFIRST
#elif defined(__sun__) || defined(__sun)
#define SUN
#define PLATFORM "SUN"
#undef LSBFIRST
#elif defined(__i386) || defined(_M_IX86) || defined(__x86_64__) || defined(_M_X64) || defined(__arm__) || defined(__mips__)
#define PC
#define PLATFORM "PC"
#define LSBFIRST
#elif defined(_AIX)
#define AIX
#define PLATFORM "AIX"
#define LSBFIRST
#elif defined (__hpux)
#define HPUX
#define PLATFORM "HPUX"
#undef LSBFIRST
#else
#define PC
#define PLATFORM "PC"
#define LSBFIRST
//#error "can't determine architecture; adapt hci_type.h to your platform"
#endif

/**
 * define symbols for compiler
 */

#if defined(__BORLANDC__)
#define HCI_BORL
#if defined(__WIN32__)
#define HCI_BORL_32
#elif defined(__DPMI32__)
#define HCI_BORL_DPMI32
#elif defined(__DPMI16__)
#define HCI_BORL_DPMI16
#else
#define HCI_BORL_16
#endif
#endif      /* Borland */

#if defined(_MSC_VER)
#if defined(MSDOS)
#define HCI_MSC_16
#elif defined(_WIN32)
#define HCI_MSC_32
#endif
#endif      /* Microsoft */

#if defined(__ZTC__)
#define HCI_ZTC_32
#endif      /* Zortech */

#if defined(__WATCOMC__)
#define HCI_WATC_32
#endif      /* Watcom */

#if defined(UNIX)
#if defined(__osf__)
#define HCI_UNIX_64
#else
#define HCI_UNIX_32
#endif
#endif      /* Unix */

#if defined(_TMS320C30)
#define HCI_TMS_C30
#endif      /* TMS C30 */
#if defined(__HOS_OS2__)
#define HCI_OS2_32
#endif 
/*
#if defined(HCI_BORL) || defined(HCI_MSC_16) || defined(HCI_MSC_32) \
|| defined(HCI_ZTC_32) || defined(HCI_WATC_32)
#define HCI_PCDOS
#define HCI_SYSTEM_STRING     "PCDOS"
#if defined(HCI_ZTC_32)
#define _dos_getdrive       dos_getdrive
#define _dos_setdrive       dos_setdrive
#define _dos_setfileattr    dos_setfileattr
#endif
#elif defined(HCI_UNIX_32) || defined(HCI_UNIX_64)
#define HCI_UNIX
#define HCI_SYSTEM_STRING     "UNIX"
#elif defined(HCI_TMS_C30)
#define HCI_TMSC30
#define HCI_SYSTEM_STRING     "TMSC30"
#elif defined(HCI_OS2_32)
#define HCI_OS2
#define HCI_SYSTEM_STRING      "OS/2"
#else
#error "can't define macros to your system" 
#endif
*/
//////////////////////////////////////////////////////////////////////////
#if defined(HCI_MSC_16)
#define HCI_USER  cdecl
#elif defined(HCI_MSC_32)
#define HCI_USER  __cdecl
#elif defined(HCI_BORL_16)
#define HCI_USER  _USERENTRY
#elif defined(HCI_BORL_32) || defined(HCI_BORL_DPMI32) || defined(HCI_BORL_DPMI16)
#define HCI_USER  __cdecl
#else
#define HCI_USER
#endif

#ifdef _WIN32_WCE
#define pack_t
#elif defined(_WIN32)
#define pack_t 
#else
#define pack_t __attribute__ ((packed))
#endif



/*
********************************************************************************
*                         DEFINITION OF CONSTANTS 
********************************************************************************
*/
/**
 * define char type
 */
typedef char hci_char;

/**
 * define 8 bit signed/unsigned types & constants
 */
#if SCHAR_MAX == 127
typedef signed char hci_int8;
#define MIN_INT8  SCHAR_MIN
#define MAX_INT8  SCHAR_MAX

typedef unsigned char hci_uint8;
typedef unsigned char	hci_boolean;
#define MIN_UINT8 0
#define MAX_UINT8 UCHAR_MAX
#else
#error cannot find 8-bit type
#endif


#ifndef FALSE
#define	FALSE	0
#endif
#ifndef TRUE
#define	TRUE	1
#endif

/**
 * define 16 bit signed/unsigned types & constants
 */
#if INT_MAX == 32767
typedef int hci_int16;
#define MIN_INT16     INT_MIN
#define MAX_INT16     INT_MAX
typedef unsigned int hci_uint16;
#define MIN_UINT16    0
#define MAX_UINT16    UINT_MAX
#elif SHRT_MAX == 32767
typedef short hci_int16;
#define MIN_INT16     SHRT_MIN
#define MAX_INT16     SHRT_MAX
typedef unsigned short hci_uint16;
#define MIN_UINT16    0
#define MAX_UINT16    USHRT_MAX
#else
#error cannot find 16-bit type
#endif


/**
 * define 32 bit signed/unsigned types & constants
 */
#if INT_MAX == 2147483647
typedef int hci_int32;
typedef int hci_flag;
#define MIN_INT32     INT_MIN
#define MAX_INT32     INT_MAX
typedef unsigned int hci_uint32;
#define MIN_UINT32    0
#define MAX_UINT32    UINT_MAX
#elif LONG_MAX == 2147483647
typedef long hci_int32;
typedef long hci_flag;
#define MIN_INT32     LONG_MIN
#define MAX_INT32     LONG_MAX
typedef unsigned long hci_uint32;
#define MIN_UINT32    0
#define MAX_UINT32    ULONG_MAX
#else
#error cannot find 32-bit type
#endif

/**
 * define floating point type & constants
 */
typedef float hci_float32;
typedef double		hci_float64;

/**
 * define 64 bit signed/unsigned types & constants
 */
#define HAVE_LONG_LONG
#define SIZEOF_LONG_LONG	8

#ifndef _WIN32
typedef	char	CHAR;
typedef	short	SHORT;
typedef	int		INT;
//typedef	long	LONG;
typedef	int		LONG;
typedef	int		BOOL;
#ifndef VOID
#define	VOID	void
#endif

#endif


#if defined(HAVE_LONG_LONG) && (SIZEOF_LONG_LONG == 8)
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
typedef LONGLONG   hci_int64;
#else
typedef long long	hci_int64;
#endif
#else /* !HAVE_LONG_LONG && SIZEOF_LONG_LONG == 8 */
typedef double          hci_int64;
#endif /* !HAVE_LONG_LONG && SIZEOF_LONG_LONG == 8 */

/** The following are approximate; IEEE floating point standards might quibble! */
#define MAX_POS_FLOAT32		3.4e+38f
#define MIN_POS_FLOAT32		1.2e-38f	/* But not 0 */
#define MAX_POS_FLOAT64		1.8e+307
#define MIN_POS_FLOAT64		2.2e-308

#define MAX_IEEE_NORM_POS_FLOAT32        3.4e+38f
#define MIN_IEEE_NORM_POS_FLOAT32        1.2e-38f
#define MIN_IEEE_NORM_NEG_FLOAT32       -3.4e+38f
#define MAX_IEEE_NORM_POS_FLOAT64        1.8e+307
#define MIN_IEEE_NORM_POS_FLOAT64        2.2e-308
#define MIN_IEEE_NORM_NEG_FLOAT64       -1.8e+307

/** Will the following really work?? */
#define MAX_NEG_FLOAT32		((hci_float32) (-MAX_POS_FLOAT32))
#define MIN_NEG_FLOAT32		((hci_float32) (-MIN_POS_FLOAT32))
#define MAX_NEG_FLOAT64		((hci_float64) (-MAX_POS_FLOAT64))
#define MIN_NEG_FLOAT64		((hci_float64) (-MIN_POS_FLOAT64))

#ifdef __cplusplus
extern "C" {
#endif

typedef union any4byte_type_s {
	hci_int32 i_32;
	hci_uint32 ui_32;
} any4byte_type_t;

typedef union anytype_s {
	void *ptr;		///< User defined data types at this ptr
#ifdef _WIN32
	hci_int32 i_32;
	hci_uint32 ui_32;
	hci_float32 fl_32;
#else
	long i_32;
	size_t ui_32;
	double fl_32;
#endif
	hci_float64 fl_64;
} anytype_t;

#ifdef __cplusplus
}
#endif

#endif  // #ifndef __HCILAB_PRIM_TYPE_H__
