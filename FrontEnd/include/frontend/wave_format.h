
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
 *	@file	wave_format.h
 *	@ingroup interface_src
 *	@date	2008/07/30
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	wave format handling library
 */

#ifndef __WAVE_FORMAT_H__
#define __WAVE_FORMAT_H__

#include "base/hci_type.h"
#include "base/hci_macro.h"

//////////////////////////////////////////////////////////////////////////

/* constants definitions */

//////////////////////////////////////////////////////////////////////////

/* structures */

typedef struct tagWAVEFILEHEADER1 {
	char szRiff[4];
	hci_int32 lFileSize;
	char szWave[8];
	hci_int32 lWaveFormatSize;
	hci_int16 nFormat;
	hci_int16 nChannels;
	hci_int32 lSamplesPerSec;
	hci_int32 lAvgBytesPerSec;
	hci_int16 nBlockAlign;
	hci_int16 nBits;
} WAVEFILEHEADER1;

typedef struct tagWAVEFILEHEADER2 {
	char szData[4];
	hci_int32 lDataSize;
} WAVEFILEHEADER2;

//////////////////////////////////////////////////////////////////////////

/* functions */

#endif	// #ifndef __WAVE_FORMAT_H__
