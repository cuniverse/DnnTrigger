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
 *	@file	hci_speex.c
 *	@ingroup speex_src
 *	@date	2010/06/16
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	API functions for PowerASR Speex Speech codec library
 */

//#ifdef HAVE_CONFIG_H
#include "speex/config.h"
//#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include <speex/speex.h>
#include <speex/speex_callbacks.h>
#include <speex/speex_header.h>
#include <speex/ogg.h>
#include "speex/hci_speex.h"

#ifdef WIN32
#define snprintf _snprintf
#endif

// local functions
#ifdef __cplusplus
extern "C" {
#endif 

/**
 * encode a single frame speech samples by speex
 */
HCILAB_PRIVATE int
speex_encode_wideband_frame_sample(SPEEX_ENC_DATA *spx_data,		///< (i/o) speex encoding data
								   short *input);					///< (i) input wave buffer

/**
 * decode a single frame data sream by speex decoder
 */
HCILAB_PRIVATE int
speex_decode_wideband_frame_data(SPEEX_DEC_DATA *spx_data,		///< (i/o) speex decoding data
								 char *bit_stream,				///< (i) input frame data
								 int size_bit_stream);			///< (i) size of input frame data

/**
 * Write an Ogg page to a file pointer
 */
HCILAB_PRIVATE int
oe_write_page(SPEEX_ENC_DATA *spx_data);

/**
 * process speex header
 */
HCILAB_PRIVATE void *
process_header(ogg_packet *op, 
			   spx_int32_t enh_enabled, 
			   spx_int32_t *frame_size, 
			   int *granule_frame_size, 
			   spx_int32_t *rate, 
			   int *nframes, 
			   int forceMode, 
			   int *channels, 
			   int *extra_headers);

/**
 * Initialize OGG header
 */
HCILAB_PRIVATE void
comment_init(char **comments, int* length, char *vendor_string);

/**
 * Add OGG header
 */
HCILAB_PRIVATE void
comment_add(char **comments, int* length, char *tag, char *val);

#ifdef __cplusplus
}
#endif


/**
 * Initialize speex wide-band speech encoder
 */
HCILAB_PUBLIC HCI_SPEEX_API int
PowerASR_SPEEX_initializeWBEncoding( SPEEX_ENC_DATA *spx_data ,int nSampleRate)			///< (i/o) speex encoding data
{
	SpeexHeader header;
	void *st = 0;
	spx_int32_t rate = nSampleRate;
	spx_int32_t quality = 10;		// 1 ~ 10
	spx_int32_t lookahead = 0;
	spx_int32_t complexity = 3;
	spx_int32_t bVBR = 0;
	spx_int32_t bVAD = 0;			// 2010-08-13
	spx_int32_t abr_enabled = 40000;	// 2010-08-13
	spx_int32_t frame_size = 0;
	int result = 0, ret = 0;
	int bytes_written = 0;
	const char* speex_version = 0;
	char vendor_string[64];
	char *comments = 0;
	int comments_length = 0;

	if ( !spx_data ) return -1;

	if ( spx_data->enc_data ) 
	{
		free(spx_data->enc_data);
		spx_data->enc_data = 0;
	}
	memset( spx_data, 0, sizeof(SPEEX_ENC_DATA) );

	speex_lib_ctl(SPEEX_LIB_GET_VERSION_STRING, (void*)&speex_version);
	snprintf(vendor_string, sizeof(vendor_string), "Encoded with Speex %s", speex_version);
	
	comment_init(&comments, &comments_length, vendor_string);
	comment_add(&comments, &comments_length, "author=", "hcilab"); 

	// Initialize Ogg stream struct
	srand(time(NULL));
	if (ogg_stream_init(&spx_data->os, rand())==-1)
	{
		fprintf(stderr,"Error: stream init failed\n");
		return -1;
	}

	// speex header
	speex_init_header(&header, rate, 1, speex_lib_get_mode (nSampleRate==16000?SPEEX_MODEID_WB:SPEEX_MODEID_NB));


	header.frames_per_packet = 1;
	header.vbr               = bVBR;
	header.nb_channels       = 1;
	
	// Initialize Speex encoder
	st = speex_encoder_init(speex_lib_get_mode(nSampleRate==16000?SPEEX_MODEID_WB:SPEEX_MODEID_NB));
	
	speex_encoder_ctl(st, SPEEX_GET_FRAME_SIZE, &frame_size);
	speex_encoder_ctl(st, SPEEX_SET_VBR, &bVBR);
	if ( !bVBR && bVAD ) speex_encoder_ctl(st, SPEEX_SET_VAD, &bVAD);
	if ( bVBR && abr_enabled ) speex_encoder_ctl(st, SPEEX_SET_ABR, &abr_enabled);
	if ( !bVBR ) speex_encoder_ctl(st, SPEEX_SET_QUALITY, &quality);
	speex_encoder_ctl(st, SPEEX_SET_COMPLEXITY, &complexity);
	speex_encoder_ctl(st, SPEEX_SET_SAMPLING_RATE, &rate);
	speex_encoder_ctl(st, SPEEX_GET_LOOKAHEAD, &lookahead);
	
	spx_data->st         = st;
	spx_data->frame_size = frame_size;

	// Write header
	{
		int packet_size;
		spx_data->op.packet = (unsigned char *)speex_header_to_packet(&header, &packet_size);
		spx_data->op.bytes = packet_size;
		spx_data->op.b_o_s = 1;
		spx_data->op.e_o_s = 0;
		spx_data->op.granulepos = 0;
		spx_data->op.packetno = 0;
		ogg_stream_packetin(&spx_data->os, &spx_data->op);
		free(spx_data->op.packet); spx_data->op.packet = 0;

		while((result = ogg_stream_flush(&spx_data->os, &spx_data->og)))
		{
			if(!result) break;
			ret = oe_write_page( spx_data );
		}
		
		spx_data->op.packet = (unsigned char *)comments;
		spx_data->op.bytes = comments_length;
		spx_data->op.b_o_s = 0;
		spx_data->op.e_o_s = 0;
		spx_data->op.granulepos = 0;
		spx_data->op.packetno = 1;
		ogg_stream_packetin(&spx_data->os, &spx_data->op);
	}
	
	// writing the rest of the speex header packets
	while( ( result = ogg_stream_flush(&spx_data->os, &spx_data->og) ) )
	{
		if( !result ) break;
		ret = oe_write_page( spx_data );
	}

	if ( comments ) { free(comments); comments = 0; }
	
	speex_bits_init( &(spx_data->bits) );

	spx_data->total_samples   = 0;
	spx_data->nb_encoded      = -lookahead;
	spx_data->len_unproc_data = 0;
	spx_data->id              = -1;
	spx_data->lookahead       = lookahead;

	spx_data->enc_output.max_block_size = MAX_ENC_OUT_SIZE;

	return 0;
}


/**
 * Initialize speex wide-band speech decoder
 */
HCILAB_PUBLIC HCI_SPEEX_API int
PowerASR_SPEEX_initializeWBDecoding( SPEEX_DEC_DATA *spx_data, int nSampleRate )			///< (i/o) speex decoding data
{
	if ( 0 == spx_data ) return -1;

	if ( spx_data->rec_wave ) 
	{
		free(spx_data->rec_wave);
		spx_data->rec_wave = 0;
	}

	memset( spx_data, 0, sizeof(*spx_data) );

	spx_data->enh_enabled    = 1;		// enhancement
	spx_data->forceMode      = nSampleRate==16000?SPEEX_MODEID_WB:SPEEX_MODEID_NB;		// wide-band
	spx_data->channels       = 1;		// mono
	spx_data->rate           = nSampleRate;	// sampling frequency
	spx_data->frame_bit_size = DEC_FRAME_SIZE;	// processing size per frame
	spx_data->nframes        = 2;
	spx_data->speex_serialno = -1;

	// Init Ogg data struct
	ogg_sync_init(&spx_data->oy);
	
	speex_bits_init(&spx_data->bits);

	spx_data->dec_output.block_size = nSampleRate / 100;

	return 0;
}


/**
 * destroy speex wide-band speech encoder
 */
HCILAB_PUBLIC HCI_SPEEX_API int
PowerASR_SPEEX_releaseWBEncoding( SPEEX_ENC_DATA *spx_data )			///< (i/o) speex encoding data
{
	int ret = 0;

	if ( 0 == spx_data ) return -1;

	if ( spx_data->st )
	{
		speex_encoder_destroy(spx_data->st);
		speex_bits_destroy(&spx_data->bits);
		ogg_stream_clear(&spx_data->os);
	}

	spx_data->st = 0;
	if ( spx_data->enc_data ) {
		free(spx_data->enc_data);
		spx_data->enc_data = 0;
		spx_data->size_enc_data = 0;
	}
	
	memset( spx_data, 0, sizeof(*spx_data) );

	return 0;
}


/**
 * destroy speex wide-band speech decoder
 */
HCILAB_PUBLIC HCI_SPEEX_API int
PowerASR_SPEEX_releaseWBDecoding( SPEEX_DEC_DATA *spx_data )			///< (i/o) speex decoding data
{
	if ( 0 == spx_data ) return -1;

	if ( spx_data->st )
		speex_decoder_destroy( spx_data->st );

	speex_bits_destroy( &spx_data->bits );

	if ( spx_data->stream_init )
		ogg_stream_clear( &spx_data->os );

	ogg_sync_clear( &spx_data->oy );

	if ( spx_data->rec_wave ) free (spx_data->rec_wave);

	memset( spx_data, 0, sizeof(*spx_data) );

	return 0;
}


/**
 * complete speex wide-band speech encoding process
 */
HCILAB_PUBLIC HCI_SPEEX_API int
PowerASR_SPEEX_completeWBEncoding( SPEEX_ENC_DATA *spx_data )			///< (i/o) speex encoding data
{
	int ret = 0;

	if ( 0 == spx_data ) return -1;

	spx_data->op.b_o_s = 0;
	spx_data->op.e_o_s = 1;

	// Flush all pages left to be written
	while (ogg_stream_flush(&spx_data->os, &spx_data->og))
	{
		ret = oe_write_page( spx_data );
	}
	
	spx_data->enc_output.size_enc_stream = 0L;
	if ( spx_data->size_enc_data > spx_data->enc_output.total_enc_size )
	{
		long copy_size = spx_data->size_enc_data - spx_data->enc_output.total_enc_size;
		if ( copy_size > spx_data->enc_output.max_block_size ) {
			copy_size = spx_data->enc_output.max_block_size;
		}
		spx_data->enc_output.size_enc_stream = copy_size;
		memcpy(spx_data->enc_output.enc_stream, spx_data->enc_data + spx_data->enc_output.total_enc_size, copy_size * sizeof(char) );
		spx_data->enc_output.total_enc_size += copy_size;
	}

	return 0;
}


/**
 * complete speex wide-band speech decoding process
 */
HCILAB_PUBLIC HCI_SPEEX_API int
PowerASR_SPEEX_completeWBDecoding( SPEEX_DEC_DATA *spx_data )			///< (i/o) speex decoding data
{
	int ret = 0;

	if ( 0 == spx_data ) return -1;

	spx_data->op.b_o_s = 0;
	spx_data->op.e_o_s = 1;

	speex_decode_wideband_frame_data( spx_data, spx_data->unproc_data, spx_data->len_unproc_data );
	
	spx_data->dec_output.size_dec_stream = 0;
	if ( spx_data->len_rec_wave > spx_data->dec_output.total_dec_size )
	{
		long copy_size = spx_data->len_rec_wave - spx_data->dec_output.total_dec_size;
		if ( copy_size > MAX_DEC_OUT_SIZE ) {
			copy_size = MAX_DEC_OUT_SIZE;
		}
		if ( copy_size % spx_data->dec_output.block_size ) {
			copy_size -= (copy_size % spx_data->dec_output.block_size);
		}
		if ( copy_size ) {
			spx_data->dec_output.size_dec_stream = copy_size;
			memcpy(spx_data->dec_output.dec_stream, spx_data->rec_wave + spx_data->dec_output.total_dec_size, copy_size * sizeof(short) );
			spx_data->dec_output.total_dec_size += copy_size;
		}
	}

	return 0;
}


/**
 * encode wide-band speech stream using speex encoder
 */
HCILAB_PUBLIC HCI_SPEEX_API int
PowerASR_SPEEX_doWBEncoding(SPEEX_ENC_DATA *spx_data,			///< (i/o) speex encoding data
							short *wavBuf,						///< (i) speech stream
							long nLenWavBuf)					///< (i) length of speech stream in sample counts
{
	short input[ENC_FRAME_SIZE];
	int len_input = 0;
	long proc_pt = 0L;
	int nbBytes = 0;
	int ret = 0;

	if ( 0 == spx_data ) return -1;

	spx_data->enc_output.size_enc_stream = 0L;

	if ( nLenWavBuf <= 0L ) return 0;

	spx_data->total_samples += (int)nLenWavBuf;

	if ( spx_data->len_unproc_data ) {
		len_input = spx_data->len_unproc_data;
		memcpy( input, spx_data->unproc_data, len_input*sizeof(short) );
		if ( (len_input + nLenWavBuf) > spx_data->frame_size ) {
			memcpy( input + len_input, wavBuf, (spx_data->frame_size - len_input) * sizeof(short) );
			spx_data->len_unproc_data = 0;
			proc_pt = spx_data->frame_size - len_input;

			speex_encode_wideband_frame_sample( spx_data, input );
		}
		else {
			memcpy( spx_data->unproc_data + len_input, wavBuf, nLenWavBuf * sizeof(short) );
			spx_data->len_unproc_data += nLenWavBuf;
			proc_pt = nLenWavBuf;
		}
	}

	while ( (proc_pt+spx_data->frame_size) <= nLenWavBuf ) {
		memcpy( input, wavBuf + proc_pt, spx_data->frame_size * sizeof(short) );
		spx_data->len_unproc_data = 0;
		proc_pt += spx_data->frame_size;

		speex_encode_wideband_frame_sample( spx_data, input );
	}

	if ( proc_pt < nLenWavBuf ) {
		memcpy( spx_data->unproc_data, wavBuf + proc_pt, (nLenWavBuf - proc_pt) * sizeof(short) );
		spx_data->len_unproc_data = nLenWavBuf - proc_pt;
	}

	if ( spx_data->size_enc_data > spx_data->enc_output.total_enc_size )
	{
		long copy_size = spx_data->size_enc_data - spx_data->enc_output.total_enc_size;
		if ( copy_size > spx_data->enc_output.max_block_size ) {
			copy_size = spx_data->enc_output.max_block_size;
		}
		spx_data->enc_output.size_enc_stream = copy_size;
		memcpy(spx_data->enc_output.enc_stream, spx_data->enc_data + spx_data->enc_output.total_enc_size, copy_size * sizeof(char) );
		spx_data->enc_output.total_enc_size += copy_size;
	}

	return 0;
}


/**
 * decode encoded speech stream using speex decoder
 */
HCILAB_PUBLIC HCI_SPEEX_API int
PowerASR_SPEEX_doWBDecoding(SPEEX_DEC_DATA *spx_data,			///< (i/o) speex decoding data
							char *encData,						///< (i) encoded data stream
							long nDataSize)						///< (i) size of encoded data stream
{
	char input[ENC_FRAME_SIZE];
	int len_input = 0;
	long proc_pt = 0L;
	int nbBytes = 0;
	int ret = 0;
	
	if ( 0 == spx_data ) return -1;

	spx_data->dec_output.size_dec_stream = 0;

	if ( nDataSize <= 0L ) return 0;

	if ( spx_data->len_unproc_data ) {
		len_input = spx_data->len_unproc_data;
		memcpy( input, spx_data->unproc_data, len_input*sizeof(char) );
		if ( (len_input + nDataSize) > spx_data->frame_bit_size ) {
			memcpy( input + len_input, encData, (spx_data->frame_bit_size - len_input) * sizeof(char) );
			spx_data->len_unproc_data = 0;
			proc_pt = spx_data->frame_bit_size - len_input;
			
			speex_decode_wideband_frame_data( spx_data, input, spx_data->frame_bit_size );
		}
		else {
			memcpy( spx_data->unproc_data + len_input, encData, nDataSize * sizeof(char) );
			spx_data->len_unproc_data += nDataSize;
			proc_pt = nDataSize;
		}
	}

	while ( (proc_pt+spx_data->frame_bit_size) <= nDataSize ) {
		memcpy( input, encData + proc_pt, spx_data->frame_bit_size * sizeof(char) );
		spx_data->len_unproc_data = 0;
		proc_pt += spx_data->frame_bit_size;
		
		speex_decode_wideband_frame_data( spx_data, input, spx_data->frame_bit_size );
	}
	
	if ( proc_pt < nDataSize ) {
		memcpy( spx_data->unproc_data, encData + proc_pt, (nDataSize - proc_pt) * sizeof(char) );
		spx_data->len_unproc_data = nDataSize - proc_pt;
	}
	
	if ( spx_data->len_rec_wave > spx_data->dec_output.total_dec_size )
	{
		long copy_size = spx_data->len_rec_wave - spx_data->dec_output.total_dec_size;
		if ( copy_size > MAX_DEC_OUT_SIZE ) {
			copy_size = MAX_DEC_OUT_SIZE;
		}
		if ( copy_size % spx_data->dec_output.block_size ) {
			copy_size -= (copy_size % spx_data->dec_output.block_size);
		}
		if ( copy_size ) {
			spx_data->dec_output.size_dec_stream = copy_size;
			memcpy(spx_data->dec_output.dec_stream, spx_data->rec_wave + spx_data->dec_output.total_dec_size, copy_size * sizeof(short) );
			spx_data->dec_output.total_dec_size += copy_size;
		}
	}

	return 0;
}


/**
 * encode a single frame speech samples by speex
 */
HCILAB_PRIVATE int
speex_encode_wideband_frame_sample(SPEEX_ENC_DATA *spx_data,			///< (i/o) speex encoding data
								   short *input)						///< (i) input wave buffer
{
	int nbBytes = 0;
	int ret = 0;
	char cbits[MAX_FRAME_BYTES];
	
	if ( 0 == spx_data ) return -1;
	if ( 0 == input ) return -1;
	
	speex_encode_int( spx_data->st, input, &spx_data->bits );
	spx_data->nb_encoded += spx_data->frame_size;
	spx_data->id += 1;
	
	speex_bits_insert_terminator(&spx_data->bits);
	nbBytes = speex_bits_write(&spx_data->bits, cbits, MAX_FRAME_BYTES);
	speex_bits_reset(&spx_data->bits);
	spx_data->op.packet = (unsigned char *)cbits;
	spx_data->op.bytes  = nbBytes;
	spx_data->op.b_o_s  = 0;
	spx_data->op.e_o_s = 0;
	spx_data->op.granulepos = (spx_data->id+1) * spx_data->frame_size - spx_data->lookahead;
	if (spx_data->op.granulepos > spx_data->total_samples)
		spx_data->op.granulepos = spx_data->total_samples;
	spx_data->op.packetno = 2 + spx_data->id;
	ogg_stream_packetin(&spx_data->os, &spx_data->op);
	
	// Write all new pages (most likely 0 or 1)
	while ( ogg_stream_pageout(&spx_data->os, &spx_data->og) )
	{
		ret = oe_write_page( spx_data );
	}

	return 0;
}


/**
 * decode a single frame data sream by speex decoder
 */
HCILAB_PRIVATE int
speex_decode_wideband_frame_data(SPEEX_DEC_DATA *spx_data,		///< (i/o) speex decoding data
								 char *bit_stream,				///< (i) input frame data
								 int size_bit_stream)			///< (i) size of input frame data
{
	char *data = 0;
	int i = 0, j = 0;
	int nb_read = size_bit_stream;
	ogg_int64_t page_granule = 0;
	int page_nb_packets = 0;
	int skip_samples = 0;
	short output[MAX_FRAME_SIZE];

	if ( 0 == spx_data ) return -1;

	// Get the ogg buffer for writing
	data = ogg_sync_buffer( &spx_data->oy, DEC_FRAME_SIZE );
	if ( nb_read ) {
		memcpy(data, bit_stream, size_bit_stream*sizeof(char));
	}
	ogg_sync_wrote( &spx_data->oy, nb_read );

	// Loop for all complete pages we got (most likely only one)
	while ( ogg_sync_pageout( &spx_data->oy, &spx_data->og ) == 1 )
	{
		int packet_no = 0;

		if ( spx_data->stream_init == 0 ) {
			ogg_stream_init( &spx_data->os, ogg_page_serialno(&spx_data->og) );
			spx_data->stream_init = 1;
		}
		if ( ogg_page_serialno(&spx_data->og) != spx_data->os.serialno ) {
			// so all streams are read.
			ogg_stream_reset_serialno( &spx_data->os, ogg_page_serialno(&spx_data->og) );
		}

		// Add page to the bit-stream
		ogg_stream_pagein( &spx_data->os, &spx_data->og );
		page_granule = ogg_page_granulepos( &spx_data->og );
		page_nb_packets = ogg_page_packets( &spx_data->og );
		if ( page_granule>0 && spx_data->frame_size )
		{
			/* FIXME: shift the granule values if --force-* is specified */
            skip_samples = spx_data->frame_size * (page_nb_packets*spx_data->granule_frame_size*spx_data->nframes 
						   - (page_granule-spx_data->last_granule))/spx_data->granule_frame_size;
            if ( ogg_page_eos(&spx_data->og) )
				skip_samples = -skip_samples;
			/*else if (!ogg_page_bos(&og))
				skip_samples = 0;*/
		} else
		{
			skip_samples = 0;
		}

		spx_data->last_granule = page_granule;

		// Extract all available packets
		packet_no = 0;
		while ( !spx_data->eos && ogg_stream_packetout(&spx_data->os, &spx_data->op) == 1 )
		{
			if (spx_data->op.bytes>=5 && !memcmp(spx_data->op.packet, "Speex", 5)) {
				spx_data->speex_serialno = spx_data->os.serialno;
			}
			if (spx_data->speex_serialno == -1 || spx_data->os.serialno != spx_data->speex_serialno)
				break;
			//If first packet, process as Speex header
			if (spx_data->packet_count==0)
			{
				spx_data->st = process_header(&spx_data->op,
											  spx_data->enh_enabled, 
											  &spx_data->frame_size, 
											  &spx_data->granule_frame_size, 
											  &spx_data->rate, 
											  &spx_data->nframes, 
											  spx_data->forceMode, 
											  &spx_data->channels, 
											  &spx_data->extra_headers);

				if ( !spx_data->st ) return -1;

				speex_decoder_ctl(spx_data->st, SPEEX_GET_LOOKAHEAD, &spx_data->lookahead);

				if ( !spx_data->nframes ) spx_data->nframes = 1;

			} else if ( spx_data->packet_count == 1 )
			{
				;
            } else if ( spx_data->packet_count <= 1 + spx_data->extra_headers )
            {
               // Ignore extra headers 
            } else {
               int lost=0;
               packet_no++;

               // End of stream condition
               if (spx_data->op.e_o_s && spx_data->os.serialno == spx_data->speex_serialno) // don't care for anything except speex eos
                  spx_data->eos = 1;
	       
               // Copy Ogg packet to Speex bitstream
               speex_bits_read_from(&spx_data->bits, (char*)spx_data->op.packet, spx_data->op.bytes);
               for ( j = 0 ; j != spx_data->nframes ; j++ )
               {
					int ret;
					// Decode frame
					ret = speex_decode_int(spx_data->st, &spx_data->bits, output);

					if (ret==-1) break;

					if (ret==-2)
					{
						fprintf (stderr, "Decoding error: corrupted stream?\n");
						break;
					}
					if ( speex_bits_remaining( &spx_data->bits ) < 0 )
					{
						fprintf (stderr, "Decoding overflow: corrupted stream?\n");
						break;
					}

					{
						int frame_offset = 0;
						int new_frame_size = spx_data->frame_size;

						if (packet_no == 1 && j==0 && skip_samples > 0)
						{
							new_frame_size -= skip_samples + spx_data->lookahead;
							frame_offset = skip_samples + spx_data->lookahead;
						}
						if (packet_no == page_nb_packets && skip_samples < 0)
						{
							int packet_length = spx_data->nframes * spx_data->frame_size + skip_samples + spx_data->lookahead;
							new_frame_size = packet_length - j * spx_data->frame_size;
							if ( new_frame_size < 0 )
								new_frame_size = 0;
							if ( new_frame_size > spx_data->frame_size )
								new_frame_size = spx_data->frame_size;
						}
						if ( new_frame_size > 0 )
						{
							if ( spx_data->len_rec_wave ) {
								spx_data->rec_wave = (short *) realloc( spx_data->rec_wave, 
													(spx_data->len_rec_wave + new_frame_size) * sizeof(short) );
							}
							else  {
								spx_data->rec_wave = (short *) calloc( new_frame_size, sizeof(short) );
							}
							memcpy( spx_data->rec_wave + spx_data->len_rec_wave, output + frame_offset, new_frame_size*sizeof(short) );	
							spx_data->len_rec_wave += new_frame_size;
						}
					}
				}
			}
            spx_data->packet_count++;
		}
	}

	return 0;
}

/**
 * Write an Ogg page to a file pointer
 */
HCILAB_PRIVATE int
oe_write_page(SPEEX_ENC_DATA *spx_data)		///< (i/o) speex encoding data
{
	ogg_page *page = 0;
	int query_size = 0;
	
	if ( !spx_data ) return 0;
	
	page = &spx_data->og;
	
	query_size = page->header_len + page->body_len;
	if ( !query_size ) return 0;
	
	if ( spx_data->size_enc_data ) 
	{
		spx_data->enc_data = (unsigned char *) realloc( spx_data->enc_data, spx_data->size_enc_data + query_size );
	}
	else
	{
		spx_data->enc_data = (unsigned char *) calloc( query_size, sizeof(unsigned char) );
	}
	
	memcpy( spx_data->enc_data + spx_data->size_enc_data, page->header, page->header_len );
	spx_data->size_enc_data += page->header_len;
	
	memcpy( spx_data->enc_data + spx_data->size_enc_data, page->body, page->body_len );
	spx_data->size_enc_data += page->body_len;
	
	return query_size;
}


/**
 * process speex header
 */
HCILAB_PRIVATE void *
process_header(ogg_packet *op, 
			   spx_int32_t enh_enabled, 
			   spx_int32_t *frame_size, 
			   int *granule_frame_size, 
			   spx_int32_t *rate, 
			   int *nframes, 
			   int forceMode, 
			   int *channels, 
			   int *extra_headers)
{
   void *st = 0;
   const SpeexMode *mode = 0;
   SpeexHeader *header = 0;
   int modeID = 0;
//   SpeexCallback callback;
      
   header = speex_packet_to_header( (char*)op->packet, op->bytes );
   if ( !header )
   {
      fprintf (stderr, "Cannot read header\n");
      return NULL;
   }
   if (header->mode >= SPEEX_NB_MODES || header->mode<0)
   {
      fprintf (stderr, "Mode number %d does not (yet/any longer) exist in this version\n", 
               header->mode);
      free(header);
      return NULL;
   }
      
   modeID = header->mode;
   if (forceMode!=-1)
      modeID = forceMode;

   mode = speex_lib_get_mode (modeID);
   
   if (header->speex_version_id > 1)
   {
      fprintf (stderr, "This file was encoded with Speex bit-stream version %d, which I don't know how to decode\n", header->speex_version_id);
      free(header);
      return NULL;
   }

   if (mode->bitstream_version < header->mode_bitstream_version)
   {
      fprintf (stderr, "The file was encoded with a newer version of Speex. You need to upgrade in order to play it.\n");
      free(header);
      return NULL;
   }
   if (mode->bitstream_version > header->mode_bitstream_version) 
   {
      fprintf (stderr, "The file was encoded with an older version of Speex. You would need to downgrade the version in order to play it.\n");
      free(header);
      return NULL;
   }
   
   st = speex_decoder_init(mode);
   if (!st)
   {
      fprintf (stderr, "Decoder initialization failed.\n");
      free(header);
      return NULL;
   }
   speex_decoder_ctl(st, SPEEX_SET_ENH, &enh_enabled);
   speex_decoder_ctl(st, SPEEX_GET_FRAME_SIZE, frame_size);
   *granule_frame_size = *frame_size;

   if (!*rate)
      *rate = header->rate;
   /* Adjust rate if --force-* options are used */
   if (forceMode!=-1)
   {
      if (header->mode < forceMode)
      {
         *rate <<= (forceMode - header->mode);
         *granule_frame_size >>= (forceMode - header->mode);
      }
      if (header->mode > forceMode)
      {
         *rate >>= (header->mode - forceMode);
         *granule_frame_size <<= (header->mode - forceMode);
      }
   }


   speex_decoder_ctl(st, SPEEX_SET_SAMPLING_RATE, rate);

   *nframes = header->frames_per_packet;

   if (*channels==-1)
      *channels = header->nb_channels;

   *extra_headers = header->extra_headers;

   free(header);

   return st;
}


/*                 
 Comments will be stored in the Vorbis style.            
 It is describled in the "Structure" section of
    http://www.xiph.org/ogg/vorbis/doc/v-comment.html

The comment header is decoded as follows:
  1) [vendor_length] = read an unsigned integer of 32 bits
  2) [vendor_string] = read a UTF-8 vector as [vendor_length] octets
  3) [user_comment_list_length] = read an unsigned integer of 32 bits
  4) iterate [user_comment_list_length] times {
     5) [length] = read an unsigned integer of 32 bits
     6) this iteration's user comment = read a UTF-8 vector as [length] octets
     }
  7) [framing_bit] = read a single bit as boolean
  8) if ( [framing_bit]  unset or end of packet ) then ERROR
  9) done.

  If you have troubles, please write to ymnk@jcraft.com.
 */

#define readint(buf, base) (((buf[base+3]<<24)&0xff000000)| \
                           ((buf[base+2]<<16)&0xff0000)| \
                           ((buf[base+1]<<8)&0xff00)| \
  	           	    (buf[base]&0xff))
#define writeint(buf, base, val) do{ buf[base+3]=((val)>>24)&0xff; \
                                     buf[base+2]=((val)>>16)&0xff; \
                                     buf[base+1]=((val)>>8)&0xff; \
                                     buf[base]=(val)&0xff; \
                                 }while(0)

/**
 * Initialize OGG header
 */
HCILAB_PRIVATE void
comment_init(char **comments,
			 int* length,
			 char *vendor_string)
{
  int vendor_length=strlen(vendor_string);
  int user_comment_list_length=0;
  int len=4+vendor_length+4;
  char *p=(char*)malloc(len);
  if(p==NULL){
     fprintf (stderr, "malloc failed in comment_init()\n");
     exit(1);
  }
  writeint(p, 0, vendor_length);
  memcpy(p+4, vendor_string, vendor_length);
  writeint(p, 4+vendor_length, user_comment_list_length);
  *length=len;
  *comments=p;
}


/**
 * Add OGG header
 */
HCILAB_PRIVATE void
comment_add(char **comments, 
			int* length, 
			char *tag, 
			char *val)
{
  char* p=*comments;
  int vendor_length=readint(p, 0);
  int user_comment_list_length=readint(p, 4+vendor_length);
  int tag_len=(tag?strlen(tag):0);
  int val_len=strlen(val);
  int len=(*length)+4+tag_len+val_len;

  p=(char*)realloc(p, len);
  if(p==NULL){
     fprintf (stderr, "realloc failed in comment_add()\n");
     exit(1);
  }

  writeint(p, *length, tag_len+val_len);      /* length of comment */
  if(tag) memcpy(p+*length+4, tag, tag_len);  /* comment */
  memcpy(p+*length+4+tag_len, val, val_len);  /* comment */
  writeint(p, 4+vendor_length, user_comment_list_length+1);

  *comments=p;
  *length=len;
}
#undef readint
#undef writeint
