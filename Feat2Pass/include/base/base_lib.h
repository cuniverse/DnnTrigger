
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
 *	@file	base_lib.h
 *	@ingroup common_base_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	Header file to define macros to import/export base-library APIs
 */

#ifndef __HCILAB_BASE_LIB_H__
#define __HCILAB_BASE_LIB_H__

#include "base/hci_type.h"

#if defined(HCI_MSC_32)
#ifdef HCI_BASELIB_EXPORTS
#define HCI_BASE_API __declspec(dllexport)
#elif defined(HCI_BASELIB_IMPORTS)
#define HCI_BASE_API __declspec(dllimport)
#else	// in case of static library
#define HCI_BASE_API
#endif // #ifdef HCI_BASELIB_EXPORTS
#elif defined(HCI_OS2)
#define HCI_BASE_API
#else
#define HCI_BASE_API HCI_USER
#endif

#endif  // #ifndef __HCILAB_BASE_LIB_H__
