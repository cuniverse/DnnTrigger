
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

/*
 * powerasr_const.h -- constant definitions for PowerASR
 */


#ifndef __POWERASR_CONST_H__
#define __POWERASR_CONST_H__

#define	FRAME_MSEC	10			// frame duration in msec 

#define MAX_DEC_FRAME	50 //30000		// maximum length of decoding frames (specific to application & frame compression type) // fixed SIZE for LVCSR
#define MAX_LEN_FEAT_FRAME	50 //30000	// maximum length of feature frames for ASR (specific to application) // fixed SIZE for LVCSR

#endif  // #ifndef __POWERASR_CONST_H__
