
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
 *	@file	hci_macro.h
 *	@ingroup common_base_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	definitions for standard C macros
 */


#ifndef __HCILAB_MACRO_H__
#define __HCILAB_MACRO_H__

#define HCI_ABS(x) ((x)>=0?(x):-(x))
#define	HCI_MIN(a,b) (((a)<(b))?(a):(b))
#define	HCI_MAX(a,b) (((a)>(b))?(a):(b))
#define HCI_MIN3(a,b,c) (((a)<(b))?(((a)<(c))?(a):(c)):(((b)<(c))?(b):(c)))
#define HCI_MAX3(a,b,c)	(((a)>(b))?(((a)>(c))?(a):(c)):(((b)>(c))?(b):(c)))
#define HCI_SQ(x)  ((x)*(x))

#ifndef	FALSE
#define FALSE	0
#endif /* FALSE */
#ifndef	TRUE
#define TRUE	1
#endif /* TRUE */

#define	CERROR		(-1)

//#ifndef	bool
//typedef enum	{ false = 0, true = 1 } bool;
//#endif /* bool */

#define	sizeofS(string)	(sizeof(string) - 1)
#define sizeofA(array)	(sizeof(array)/sizeof(array[0]))

#define caseE(enum_type)	case (hci_int32)(enum_type)

#endif  // #ifndef __HCILAB_MACRO_H__
