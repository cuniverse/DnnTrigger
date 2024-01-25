
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
 *	@file	hci_logadd.h
 *	@ingroup basic_op_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	Log-addition table
 */

#ifndef __HCI_LOGADD_H__
#define __HCI_LOGADD_H__

#include "base/hci_type.h"
#include "base/hci_macro.h"
#include "basic_op/fixpoint.h"

#define	MAX_LOG_TABLE	512			///< size of log-addition table

/** natural log addition table */
#ifdef FIXED_POINT_FE
#define LOG_DIFF	(1<<18)			///< maximum difference in fixed-point log-addition
#define LOG_IDX(x)	( (x)>>9 )		///< table index in fixed-point log-addition
#else	// !FIXED_POINT_FE
#define	LOG_DIFF	(8.0f)			///< maximum difference in floating-point log-addition
#define	LOG_PROD	(MAX_LOG_TABLE/LOG_DIFF)		///< constant term to get table index
#define	LOG_IDX(x)	((hci_int32)( (x)*LOG_PROD ))	///< table index in floating-point log-addition
#endif	// #ifdef FIXED_POINT_FE

/** 100 x natural log addition table */
#define FIXED_LOG_DIFF	512
#define FIXED_LOG_PROD	(MAX_LOG_TABLE/FIXED_LOG_DIFF)
#define FIXED_LOG_IDX(x)	(x)

#ifndef hci_logadd_defined
#ifdef FIXED_POINT_FE
typedef hci_fixed32 hci_logadd_t;
#else	// !FIXED_POINT_FE
typedef	hci_float32	hci_logadd_t;
#endif	// #ifdef FIXED_POINT_FE
#define hci_logadd_defined
#endif	// !hci_logadd_t

/** Add two numbers in LOG domain. */
#define HCI_LOG_ADD(x,y,z,_log_tbl) \
{	\
	hci_logadd_t _d = HCI_ABS((y)-(z));	\
	hci_logadd_t _max = HCI_MAX((y),(z));	\
	x = ((_d) < LOG_DIFF ? _max+_log_tbl[LOG_IDX(_d)] : _max);	\
}	\

#endif	// #ifndef __HCI_LOGADD_H__
