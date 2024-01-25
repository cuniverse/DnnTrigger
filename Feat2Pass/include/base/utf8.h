
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
 *	@file	utf8.h
 *	@ingroup common_base_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	Text Encoding library
 */

#ifndef _UTF8__H_
#define _UTF8__H_

#include "base/hci_type.h"
#include "base/base_lib.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	eANSI,
	eUTF8,
	eUnicode,
	eUnicodeBigEndian,
	eUNK
} FTYPE;

typedef unsigned short wchar16_t; 

// ISO 5601 -> UTF
HCILAB_PUBLIC HCI_BASE_API hci_int32
PowerASR_Base_encode_UTF8(const unsigned char *instr,
						  int szinstr,
						  unsigned char **outstr,
						  int *outlen);

// UTF -> ISO 5601
HCILAB_PUBLIC HCI_BASE_API hci_int32
PowerASR_Base_decode_UTF8(const unsigned char *instr,
						  int inlen,
						  char *outstr,
						  int szoutstr);

// 주어진 텍스트가 UTF-8 코드인지 판별
HCILAB_PUBLIC HCI_BASE_API hci_int32
PowerASR_Base_is_UTF8(unsigned char* str);

//유니코드에서 ANSI 문자열로 변환
HCILAB_PUBLIC HCI_BASE_API char*
PowerASR_Base_UnicodeToAnsi(wchar16_t* szUniStr,
							int cbSize);

// Big-endian 유니코드에서 Little-endian 유니코드 문자열로 변환
HCILAB_PUBLIC HCI_BASE_API void
PowerASR_Base_UnicodeBigEndianToUnicode(wchar16_t* scrString,
										long cbsize);

#ifdef __cplusplus
}
#endif

#endif //_UTF8__H_
