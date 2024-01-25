
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
 *	@author	������(ijchoi@hcilab.co.kr) (��)HCILAB(http://www.hcilab.co.kr)
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

// �־��� �ؽ�Ʈ�� UTF-8 �ڵ����� �Ǻ�
HCILAB_PUBLIC HCI_BASE_API hci_int32
PowerASR_Base_is_UTF8(unsigned char* str);

//�����ڵ忡�� ANSI ���ڿ��� ��ȯ
HCILAB_PUBLIC HCI_BASE_API char*
PowerASR_Base_UnicodeToAnsi(wchar16_t* szUniStr,
							int cbSize);

// Big-endian �����ڵ忡�� Little-endian �����ڵ� ���ڿ��� ��ȯ
HCILAB_PUBLIC HCI_BASE_API void
PowerASR_Base_UnicodeBigEndianToUnicode(wchar16_t* scrString,
										long cbsize);

#ifdef __cplusplus
}
#endif

#endif //_UTF8__H_
