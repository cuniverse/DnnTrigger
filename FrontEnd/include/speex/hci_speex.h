
/* ====================================================================
 * Copyright (c) 2007-2010 HCI LAB. 
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
 *	@file	hci_speex.h
 *	@ingroup speex_src
 *	@date	2010/06/16
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	API functions for PowerASR Speex Speech codec library
 */

#ifndef __HCILAB_SPEEX_H__
#define __HCILAB_SPEEX_H__

#ifndef WIN32
#include <pthread.h>
#endif

#include "base/hci_type.h"
#include "speex/speex_types.h"
#include "speex/os_types.h"

#if defined(HCI_MSC_32)
#ifdef HCI_SPEEX_EXPORTS
#define HCI_SPEEX_API __declspec(dllexport)
#elif defined(HCI_SPEEX_IMPORTS)
#define HCI_SPEEX_API __declspec(dllimport)
#else	// in case of static library
#define HCI_SPEEX_API
#endif // #ifdef HCI_EPD_EXPORTS
#elif defined(HCI_OS2)
#define HCI_SPEEX_API
#else
#define HCI_SPEEX_API
#endif

#define MAX_FRAME_SIZE	2000*2 // fixed SIZE for LVCSR
#define MAX_FRAME_BYTES 2000
#define ENC_FRAME_SIZE	320
#define DEC_FRAME_SIZE	200

#ifndef SpeexBits_defined

/** Bit-packing data structure representing (part of) a bit-stream. */
typedef struct _SpeexBits {
	char *chars;		/**< "raw" data */
	int   nbBits;		/**< Total number of bits stored in the stream*/
	int   charPtr;		/**< Position of the byte "cursor" */
	int   bitPtr;		/**< Position of the bit "cursor" within the current char */
	int   owner;		/**< Does the struct "own" the "raw" buffer (member "chars") */
	int   overflow;		/**< Set to one if we try to read past the valid data */
	int   buf_size;		/**< Allocated size for buffer */
	int   reserved1;	/**< Reserved for future use */
	void *reserved2;	/**< Reserved for future use */
} SpeexBits;

#define SpeexBits_defined
#endif	// #ifndef SpeexBits_defined

#ifndef ogg_page_defined

/* ogg_page is used to encapsulate the data in one Ogg bitstream page *****/
typedef struct {
  unsigned char *header;
  long header_len;
  unsigned char *body;
  long body_len;
} ogg_page;

#define ogg_page_defined
#endif	// #ifndef ogg_page_defined

#ifndef ogg_stream_state_defined

/* ogg_stream_state contains the current encode/decode state of a logical
   Ogg bitstream **********************************************************/
typedef struct {
  unsigned char   *body_data;    /* bytes from packet bodies */
  long    body_storage;          /* storage elements allocated */
  long    body_fill;             /* elements stored; fill mark */
  long    body_returned;         /* elements of fill returned */


  int     *lacing_vals;      /* The values that will go to the segment table */
  ogg_int64_t *granule_vals; /* granulepos values for headers. Not compact
                                this way, but it is simple coupled to the
                                lacing fifo */
  long    lacing_storage;
  long    lacing_fill;
  long    lacing_packet;
  long    lacing_returned;

  unsigned char    header[282];      /* working space for header encode */
  int              header_fill;

  int     e_o_s;          /* set when we have buffered the last packet in the
                             logical bitstream */
  int     b_o_s;          /* set after we've written the initial page
                             of a logical bitstream */
  long    serialno;
  long    pageno;
  ogg_int64_t  packetno;  /* sequence number for decode; the framing
                             knows where there's a hole in the data,
                             but we need coupling so that the codec
                             (which is in a seperate abstraction
                             layer) also knows about the gap */
  ogg_int64_t   granulepos;

} ogg_stream_state;

#define ogg_stream_state_defined
#endif	// #ifndef ogg_stream_state_defined

#ifndef ogg_sync_state_defined

typedef struct {
	unsigned char *data;
	int storage;
	int fill;
	int returned;
	
	int unsynced;
	int headerbytes;
	int bodybytes;
} ogg_sync_state;

#define ogg_sync_state_defined
#endif

#ifndef ogg_packet_defined

/* ogg_packet is used to encapsulate the data and metadata belonging
   to a single raw Ogg/Vorbis packet *************************************/
typedef struct {
  unsigned char *packet;
  long  bytes;
  long  b_o_s;
  long  e_o_s;

  ogg_int64_t  granulepos;

  ogg_int64_t  packetno;     /* sequence number for decode; the framing
                                knows where there's a hole in the data,
                                but we need coupling so that the codec
                                (which is in a seperate abstraction
                                layer) also knows about the gap */
} ogg_packet;

#define ogg_packet_defined
#endif	// #ifndef ogg_packet_defined

#define MAX_ENC_OUT_SIZE	4000
#define MAX_DEC_OUT_SIZE	3200

// speex encoded data stream
typedef struct _SPEEX_ENC_OUTPUT
{
	long total_enc_size;
	long max_block_size;
	long size_enc_stream;
	char enc_stream[MAX_ENC_OUT_SIZE];
} SPEEX_ENC_OUTPUT;

// decoded speex speech stream
typedef struct _SPEEX_DEC_OUTPUT
{
	long total_dec_size;
	long block_size;
	long size_dec_stream;
	short dec_stream[MAX_DEC_OUT_SIZE];
} SPEEX_DEC_OUTPUT;

// speex encoding data structure
typedef struct _SPEEX_ENC_DATA
{
	void				*st;
	int					total_samples;
	int					nb_encoded;
	int					id;
	int					lookahead;
	long				nProcFrame;
	long				nEndFrame;
	long				size_enc_data;
	unsigned char		*enc_data;
	SpeexBits			bits;
	ogg_stream_state	os;
	ogg_page			og;
	ogg_packet			op;
	spx_int32_t			frame_size;
	long				len_unproc_data;
	short				unproc_data[ENC_FRAME_SIZE];
	SPEEX_ENC_OUTPUT	enc_output;
} SPEEX_ENC_DATA;


// speex decoding data structure
typedef struct _SPEEX_DEC_DATA
{
	void				*st;
	int					enh_enabled;
	int					forceMode;
	int					channels;
	int					rate;
	int					frame_bit_size;
	int					frame_size;
	int					granule_frame_size;
	int					nframes;
	int					packet_count;
	int					eos;
	int					speex_serialno;
	int					extra_headers;
	int					lookahead;
	int					stream_init;
	ogg_int64_t			last_granule;
	SpeexBits			bits;
	ogg_sync_state		oy;
	ogg_stream_state	os;
	ogg_page			og;
	ogg_packet			op;
	long				len_unproc_data;
	char				unproc_data[DEC_FRAME_SIZE];
	long				len_rec_wave;
	short				*rec_wave;
	SPEEX_DEC_OUTPUT	dec_output;
	
#ifdef WIN32
	HANDLE hDataMutex;					///< mutex handle to control generation/copy of wave data
#else	// LINUX or UNIX
	pthread_mutex_t hDataMutex;			///< mutex handle to control generation/copy of wave data
#endif

} SPEEX_DEC_DATA;


// API functions

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize speex wide-band speech encoder
 */
HCILAB_PUBLIC HCI_SPEEX_API int
PowerASR_SPEEX_initializeWBEncoding( SPEEX_ENC_DATA *spx_data,  			///< (i/o) speex encoding data
									 int nSampleRate
);

/**
 * Initialize speex wide-band speech decoder
 */
HCILAB_PUBLIC HCI_SPEEX_API int
PowerASR_SPEEX_initializeWBDecoding( SPEEX_DEC_DATA *spx_data, 			///< (i/o) speex decoding data
									 int nSampleRate);

/**
 * destroy speex wide-band speech encoder
 */
HCILAB_PUBLIC HCI_SPEEX_API int
PowerASR_SPEEX_releaseWBEncoding( SPEEX_ENC_DATA *spx_data				///< (i/o) speex encoding data
);

/**
 * destroy speex wide-band speech decoder
 */
HCILAB_PUBLIC HCI_SPEEX_API int
PowerASR_SPEEX_releaseWBDecoding( SPEEX_DEC_DATA *spx_data				///< (i/o) speex decoding data
);

/**
 * complete speex wide-band speech encoding process
 */
HCILAB_PUBLIC HCI_SPEEX_API int
PowerASR_SPEEX_completeWBEncoding( SPEEX_ENC_DATA *spx_data 			///< (i/o) speex encoding data
);

/**
 * complete speex wide-band speech decoding process
 */
HCILAB_PUBLIC HCI_SPEEX_API int
PowerASR_SPEEX_completeWBDecoding( SPEEX_DEC_DATA *spx_data 			///< (i/o) speex decoding data
);

/**
 * encode wide-band speech stream using speex encoder
 */
HCILAB_PUBLIC HCI_SPEEX_API int
PowerASR_SPEEX_doWBEncoding(SPEEX_ENC_DATA *spx_data,				///< (i/o) speex encoding data
							short *wavBuf,							///< (i) speech stream
							long nLenWavBuf							///< (i) length of speech stream in sample counts
);

/**
 * decode encoded speech stream using speex decoder
 */
HCILAB_PUBLIC HCI_SPEEX_API int
PowerASR_SPEEX_doWBDecoding(SPEEX_DEC_DATA *spx_data,			///< (i/o) speex decoding data
							char *encData,						///< (i) encoded data stream
							long nDataSize						///< (i) size of encoded data stream
);

#ifdef __cplusplus
}
#endif

#endif // #ifndef __HCILAB_SPEEX_H__

