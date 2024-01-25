
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
 *	@file	hci_msg.h
 *	@ingroup common_base_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	Header file to define API functions for checking/reporting errors
 *
 *  Logging, warning and error message output funtionality is provided in this file.
 */


#ifndef __HCILAB_MSG_H__
#define __HCILAB_MSG_H__

#include <stdarg.h>
#ifndef _WIN32_WCE
#include <errno.h>
#endif
#include "base/hci_type.h"
#include "base/base_lib.h"

#ifdef __cplusplus
extern "C" {
#endif

HCILAB_PUBLIC HCI_BASE_API void _E__pr_header( char const *file, long line, char const *msg );
HCILAB_PUBLIC HCI_BASE_API void _E__pr_info_header( char const *file, long line, char const *tag );
HCILAB_PUBLIC HCI_BASE_API void _E__pr_info_header_wofn(char const *msg);
HCILAB_PUBLIC HCI_BASE_API void _E__pr_warn( char const *fmt, ... );
HCILAB_PUBLIC HCI_BASE_API void _E__pr_info( char const *fmt, ... );
HCILAB_PUBLIC HCI_BASE_API void _E__die_error( char const *fmt, ... );
HCILAB_PUBLIC HCI_BASE_API void _E__abort_error( char const *fmt, ... );
HCILAB_PUBLIC HCI_BASE_API void _E__sys_error( char const *fmt, ... );
HCILAB_PUBLIC HCI_BASE_API void _E__fatal_sys_error( char const *fmt, ... );

/**
 * set logging file & open log-file
 */
HCILAB_PUBLIC HCI_BASE_API hci_int32
PowerASR_Base_setLogFile(char const *file);

/**
 * close log-file
 */
HCILAB_PUBLIC HCI_BASE_API void
PowerASR_Base_closeLogFile();

/**
 * exit with non-zero status after error message 
 */
#define HCIMSG_FATAL  _E__pr_header(__FILE__, __LINE__, "FATAL_ERROR"),_E__die_error

/**
 * Print error text; Call perror(""); exit(errno); 
 */
#define HCIMSG_FATAL_SYSTEM	_E__pr_header(__FILE__, __LINE__, "SYSTEM_ERROR"),_E__fatal_sys_error

/** Print error text; Call perror(""); 
 *
 */
#define HCIMSG_WARN_SYSTEM	_E__pr_header(__FILE__, __LINE__, "SYSTEM_ERROR"),_E__sys_error

/** Print error text; Call perror(""); 
 *
 */
#define HCIMSG_ERROR_SYSTEM	_E__pr_header(__FILE__, __LINE__, "SYSTEM_ERROR"),_E__sys_error


/**
 *Print logging information to standard error stream
 */
#define HCIMSG_INFO	  _E__pr_info_header(__FILE__, __LINE__, "INFO"),_E__pr_info

/**
 *Print logging information without header, to standard error stream
 */

#define HCIMSG_INFOCONT	  _E__pr_info

/**
 *
 */

#define HCIMSG_INFO_NOFN _E__pr_info_header_wofn("INFO"),_E__pr_info


/**
 *Print warning information to standard error stream
 */

#define HCIMSG_WARN	  _E__pr_header(__FILE__, __LINE__, "WARNING"),_E__pr_warn

/**
 *Print error message to standard error stream
 */

#define HCIMSG_ERROR	  _E__pr_header(__FILE__, __LINE__, "ERROR"),_E__pr_warn


#ifdef __cplusplus
}
#endif


#endif // #ifndef __HCILAB_MSG_H__


