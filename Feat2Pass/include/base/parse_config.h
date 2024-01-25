
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
 *	@file	parse_config.h
 *	@ingroup common_base_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	Command/Configuration line argument parsing and handling
 *
 *	- A command-line parsing routine that handle command-line input and file input of arguments.
 *  - [Format] name = values
 *	- arguments with '#' or ';' are regarded as comments.
 */


#ifndef __BASELIB_PARSE_CONFIG_H__
#define __BASELIB_PARSE_CONFIG_H__

#include "base/hci_type.h"
#include "base/base_lib.h"

#define ARG_MAX_LENGTH 256

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Parse each tokens of an arguments file [format: name = value]
 *  ; arguments with '#' or ';' are regarded as comments.
 */
HCILAB_PUBLIC HCI_BASE_API hci_int32
PowerASR_Base_parseConfigFile(const char *filename	/**< In: A file that contains all the arguments */ 
);

/**
 * free memory for configurations
 */
HCILAB_PUBLIC HCI_BASE_API void
PowerASR_Base_closeConfigurations();

/**
 * get argument value from hash table & arg-value table
 */
HCILAB_PUBLIC HCI_BASE_API char *
PowerASR_Base_getArgumentValue(const char *strArg);

#ifdef __cplusplus
}
#endif

#endif // #ifndef __BASELIB_PARSE_CONFIG_H__


