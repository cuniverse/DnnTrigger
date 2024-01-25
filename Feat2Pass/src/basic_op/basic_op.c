
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
 *	@file	basic_op.c
 *	@ingroup basic_op_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	APIs for basic fixed-point arithmetic operators
 */


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifndef WIN32
#include <string.h> // memset(), strcpy() etc.
#endif

#include "basic_op/basic_op.h"
#include "basic_op/hci_logadd.h"
#include "base/hci_msg.h"
#include "base/hci_macro.h"
#include "base/hci_malloc.h"

#define TPI_CONST		11796480	/**< Q15/32bit <- 360	 */	
#define AG_CONST		19898		/**< Q15/16bit <- 0.6072529350 */
#define TBL512_OFFSET	6

HCILAB_PRIVATE hci_flag bOverflow = 0;
HCILAB_PRIVATE hci_flag bCarry = 0;

typedef struct 
{
	hci_logadd_t logAddTbl[MAX_LOG_TABLE+1];	///< log-addition table (Q15.32)
	hci_int16 fixedLogAddTbl[MAX_LOG_TABLE+1];	///< log-addition table to compute state LL (100 x log)
	hci_flag madeLAddTbl;						///< flag to indicate whether general log-addition table was already made.
	hci_flag madeFixedLAddTbl;					///< flag to indicate whether stateLL log-addition table was already made.
} BasicOP_Inner;

// local functions
#ifdef __cplusplus
extern "C" {
#endif 

/**
 * Limit the 32 bit input to the range of a 16 bit word.
 *
 * @return 16 bit short signed integer output value
 */

HCILAB_PRIVATE hci_int16 
_saturate(hci_int32 L_var1);		///< 32 bit long signed integer

/**
 *	create log-addition table
 */
HCILAB_PRIVATE void 
_createLogAdditionTable(hci_logadd_t *pLogAddTbl);

/**
 *	create log-addition table to compute state LLs
 */
HCILAB_PRIVATE void 
_createLogAdditionTableForStateLL(hci_int16 *pLogAddTbl);

#ifdef __cplusplus
}
#endif

/**
 *	create a new basic operator
 *
 *	@return pointer to a newly created basic operator
 */
HCILAB_PUBLIC HCI_BASICOP_API PowerASR_BasicOP*
PowerASR_BasicOP_new()
{
	PowerASR_BasicOP *pBasicOP = 0;
	BasicOP_Inner *pInner = 0;

	pBasicOP = (PowerASR_BasicOP *) hci_malloc( sizeof(PowerASR_BasicOP) );

	if ( pBasicOP ) {
		memset(pBasicOP, 0, sizeof(PowerASR_BasicOP));

		pInner = (BasicOP_Inner *) hci_malloc( sizeof(BasicOP_Inner) );
		if ( pInner ) {
			memset(pInner, 0, sizeof(BasicOP_Inner));
			pBasicOP->pInner = (void *)pInner;
			pInner->madeLAddTbl = FALSE;
			pInner->madeFixedLAddTbl = FALSE;
		}
	}
	else {
		HCIMSG_ERROR("cannot create PowerASR Basic Operator.\n");
	}

	return pBasicOP;
}


/**
 *	delete delete the basic operator.
 */
HCILAB_PUBLIC HCI_BASICOP_API void
PowerASR_BasicOP_delete(PowerASR_BasicOP *pThis)		///< (i/o) pointer to the basic operator
{
	BasicOP_Inner *pInner = 0;

	if (0 == pThis) {
		return;
	}

	pInner = (BasicOP_Inner *) pThis->pInner;

	if ( pInner ) hci_free(pInner);
	hci_free(pThis);
}


/**
 *	return the pointer of log-addition table.
 *	if pointer is null, make a new log-addition table.
 *
 *	@return pointer of log-addition table
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_logadd_t*
PowerASR_BasicOP_getLogAdditionTable(PowerASR_BasicOP *pThis)
{
	BasicOP_Inner *pInner = 0;

	if (0 == pThis) {
		return 0;
	}

	pInner = (BasicOP_Inner *) pThis->pInner;

	if (FALSE == pInner->madeLAddTbl) {
		_createLogAdditionTable(pInner->logAddTbl);
		pInner->madeLAddTbl = TRUE;
	}

	return pInner->logAddTbl;
}


/**
 *	return the pointer of log-addition table to compute state log-likelihoods (scaled by 100).
 *	if pointer is null, make a new log-addition table.
 *
 *	@return pointer of log-addition table for state LL.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int16*
PowerASR_BasicOP_getLogAdditionTableForStateLL(PowerASR_BasicOP *pThis)
{
	BasicOP_Inner *pInner = 0;

	if (0 == pThis) {
		return 0;
	}

	pInner = (BasicOP_Inner *) pThis->pInner;

	if (FALSE == pInner->madeFixedLAddTbl) {
		_createLogAdditionTableForStateLL(pInner->fixedLogAddTbl);
		pInner->madeFixedLAddTbl = TRUE;
	}

	return pInner->fixedLogAddTbl;
}


/**
 *	Performs the addition (var1+var2) with overflow control and saturation.
 *	the 16 bit result is set at +32767 when overflow occurs or at -32768 when underflow occurs.
 *
 *  Complexity weight : 1
 *
 *	@return Return 16-bit short signed integer output value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int16
PowerASR_BasicOP_add_16_16(hci_int16 var1,		///< 16 bit short signed integer
						   hci_int16 var2)		///< 16 bit short signed integer
{
    hci_int16 var_out = 0;
    hci_int32 L_sum = 0;

    L_sum = (hci_int32) var1 + var2;
    //var_out = _saturate (L_sum);
	var_out = PowerASR_BasicOP_extractLSB_32_16(L_sum);

    return (var_out);
}

/**
 *	Performs the subtraction (var1-var2) with overflow control and saturation.
 *	the 16 bit result is set at +32767 when overflow occurs or at -32768 when underflow occurs.
 *
 *  Complexity weight : 1
 *
 *	@return Return 16-bit short signed integer output value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int16
PowerASR_BasicOP_subtract_16_16(hci_int16 var1,		///< 16 bit short signed integer
								hci_int16 var2)		///< 16 bit short signed integer
{
    hci_int16 var_out = 0;
    hci_int32 L_diff = 0;

    L_diff = (hci_int32) (var1 - var2);
    //var_out = _saturate (L_diff);
	var_out = PowerASR_BasicOP_extractLSB_32_16(L_diff);

    return (var_out);
}

/**
 *	Absolute value of var1.
 *	abs_16_16(-32768) = 32767.
 *
 *  Complexity weight : 1
 *
 *	@return Return 16-bit short signed integer output value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int16
PowerASR_BasicOP_abs_16_16(hci_int16 var1)		///< 16 bit short signed integer
{
    hci_int16 var_out = 0;

    if (var1 == MIN_INT16) {
        var_out = MAX_INT16;
    }
    else {
        if (var1 < 0) {
            var_out = (hci_int16)-var1;
        }
        else {
            var_out = var1;
        }
    }

    return (var_out);
}

/**
 *	Arithmetically shift the 16 bit input var1 left var2 positions.
 *
 *	Zero fill the var2 LSB of the result.
 *	If var2 is negative, arithmetically shift var1 right by -var2 with sign extension.
 *	Saturate the result in case of underflows or overflows.
 *
 *  Complexity weight : 1
 *
 *	@return Return 16-bit short signed integer output value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int16
PowerASR_BasicOP_shiftLeft_16_16(hci_int16 var1,	///< 16 bit short signed integer
								 hci_int16 var2)	///< 16 bit short signed integer
{
    hci_int16 var_out = 0;
    hci_int32 result = 0;

    if (var2 < 0) {
        if (var2 < -16)
            var2 = -16;
        var_out = PowerASR_BasicOP_shiftRight_16_16 (var1, (hci_int16)-var2);
    }
    else {
        result = (hci_int32) var1 *((hci_int32) 1 << var2);

        //if ((var2 > 15 && var1 != 0) || (result != (hci_int32) ((hci_int16) result))) {
        //    bOverflow = 1;
        //    var_out = (hci_int16)((var1 > 0) ? MAX_INT16 : MIN_INT16);
        //}
        //else {
        //    var_out = PowerASR_BasicOP_extractLSB_32_16(result);
        //}
		var_out = PowerASR_BasicOP_extractLSB_32_16(result);
    }

    return (var_out);
}


/**
 *	Arithmetically shift the 16 bit input var1 right var2 positions with sign extension.
 *
 *	If var2 is negative, arithmetically shift var1 left by -var2 with sign extension.
 *	Saturate the result in case of underflows or overflows.
 *
 *  Complexity weight : 1
 *
 *	@return Return 16-bit short signed integer output value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int16
PowerASR_BasicOP_shiftRight_16_16(hci_int16 var1,		///< 16 bit short signed integer
								  hci_int16 var2)		///< 16 bit short signed integer
{
    hci_int16 var_out = 0;

    if (var2 < 0) {
        if (var2 < -16)
            var2 = -16;
        var_out = PowerASR_BasicOP_shiftLeft_16_16 (var1, (hci_int16)-var2);
    }
    else {
        //if (var2 >= 15) {
        //    var_out = (hci_int16)((var1 < 0) ? -1 : 0);
        //}
        //else {
            if (var1 < 0) {
                var_out = (hci_int16)(~((~var1) >> var2));
            }
            else {
                var_out = (hci_int16)(var1 >> var2);
            }
        //}
    }

    return (var_out);
}


/**
 *	Performs the multiplication of var1 by var2 and gives a 16 bit result which is scaled i.e.:
 *		- multiply_16_16(var1,var2) = extractLSB_32_16(L_shr((var1 times var2),15)) and
 *		- multiply_16_16(-32768,-32768) = 32767.
 *
 *  Complexity weight : 1
 *
 *	@return Return 16-bit short signed integer output value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int16
PowerASR_BasicOP_multiply_16_16(hci_int16 var1,		///< 16 bit short signed integer
								hci_int16 var2)		///< 16 bit short signed integer
{
    hci_int16 var_out = 0;
    hci_int32 L_product = 0;

    L_product = (hci_int32) var1 *(hci_int32) var2;

    L_product = (L_product & (hci_int32) 0xffff8000L) >> 15;

    if (L_product & (hci_int32) 0x00010000L)
        L_product = L_product | (hci_int32) 0xffff0000L;

    //var_out = _saturate (L_product);
	var_out = PowerASR_BasicOP_extractLSB_32_16(L_product);

    return (var_out);
}


/**
 *	Returns the 32 bit result of the multiplication of var1 times var2 with one shift left i.e.:
 *		- multiply_16_32(var1,var2) = shiftLeft_32_32((var1 times var2),1) and
 *		- multiply_16_32(-32768,-32768) = 2147483647.
 *
 *  Complexity weight : 1
 *
 *	@return Return 32-bit long signed integer output value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int32
PowerASR_BasicOP_multiply_16_32(hci_int16 var1,		///< 16 bit short signed integer
								hci_int16 var2)		///< 16 bit short signed integer
{
    hci_int32 L_var_out = 0;

    L_var_out = (hci_int32) var1 *(hci_int32) var2;

    if (L_var_out != 0x40000000L) {
        L_var_out *= 2;
    }
    else {
        bOverflow = 1;
        L_var_out = MAX_INT32;
    }

    return (L_var_out);
}


/**
 *	Negate var1 with saturation, saturate in the case where input is -32768.
 *		- negate_16_16(var1) = subtract_16_16(0,var1).
 *
 *  Complexity weight : 1
 *
 *	@return Return 16-bit short signed integer output value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int16
PowerASR_BasicOP_negate_16_16(hci_int16 var1)		///< 16 bit short signed integer
{
    hci_int16 var_out = 0;

    var_out = (hci_int16)((var1 == MIN_INT16) ? MAX_INT16 : -var1);

    return (var_out);
}


/**
 *	Return the 16 MSB of L_var1.
 *
 *  Complexity weight : 1
 *
 *	@return Return 16-bit short signed integer output value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int16
PowerASR_BasicOP_extractMSB_32_16(hci_int32 L_var1)		///< 32 bit long signed integer
{
    hci_int16 var_out = 0;

    var_out = (hci_int16) (L_var1 >> 16);

    return (var_out);
}

/**
 *	Return the 16 LSB of L_var1.
 *
 *  Complexity weight : 1
 *
 *	@return Return 16-bit short signed integer output value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int16
PowerASR_BasicOP_extractLSB_32_16(hci_int32 L_var1)		///< 32 bit long signed integer
{
    hci_int16 var_out = 0;

    if (L_var1 > MAX_INT16) {
		var_out = MAX_INT16;
	}
	else if (L_var1 < MIN_INT16) {
		var_out = MIN_INT16;
	}
	else {
		var_out = (hci_int16) L_var1;
	}

    return (var_out);
}


/**
 *	Round the lower 16 bits of the 32 bit input number into the MS 16 bits with saturation.
 *
 *	Shift the resulting bits right by 16 and return the 16 bit number:
 *		- round_32_16(L_var1) = extractMSB_32_16(add_32_32(L_var1,32768))
 *
 *  Complexity weight : 1
 *
 *	@return Return 16-bit short signed integer output value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int16
PowerASR_BasicOP_round_32_16(hci_int32 L_var1)		///< 32 bit long signed integer
{
    hci_int16 var_out = 0;
    hci_int32 L_rounded = 0;

    L_rounded = PowerASR_BasicOP_add_32_32(L_var1, (hci_int32) 0x00008000L);

    var_out = PowerASR_BasicOP_extractMSB_32_16 (L_rounded);

    return (var_out);
}


/**
 *	Multiply var1 by var2 and shift the result left by 1.
 *	Add the 32 bit result to L_var3 with saturation, return a 32 bit result:
 *		- multiplyAddConst_16_32_32(L_var3,var1,var2) = add_32_32(L_var3,multiply_16_32(var1,var2))
 *
 *  Complexity weight : 1
 *
 *	@return Return 32-bit long signed integer output value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int32
PowerASR_BasicOP_multiplyAddConst_16_32_32(hci_int32 L_var3,	///< 32 bit long signed integer
										   hci_int16 var1,		///< 16 bit short signed integer
										   hci_int16 var2)		///< 16 bit short signed integer
{
    hci_int32 L_var_out = 0;
    hci_int32 L_product = 0;

    L_product = PowerASR_BasicOP_multiply_16_32(var1, var2);

    L_var_out = PowerASR_BasicOP_add_32_32(L_var3, L_product);

    return (L_var_out);
}


/**
 *	Multiply var1 by var2 and shift the result left by 1.
 *	Subtract the 32 bit result to L_var3 with saturation, return a 32 bit result:
 *		- multiplySubtractConst_16_32_32(L_var3,var1,var2) = sub_32_32(L_var3,multiply_16_32(var1,var2)).
 *
 *  Complexity weight : 1
 *
 *	@return Return 32-bit long signed integer output value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int32
PowerASR_BasicOP_multiplySubtractConst_16_32_32(hci_int32 L_var3,	///< 32 bit long signed integer
												hci_int16 var1,		///< 16 bit short signed integer
												hci_int16 var2)		///< 16 bit short signed integer
{
    hci_int32 L_var_out = 0;
    hci_int32 L_product = 0;

    L_product = PowerASR_BasicOP_multiply_16_32(var1, var2);

    L_var_out = PowerASR_BasicOP_subtract_32_32(L_var3, L_product);

    return (L_var_out);
}


/**
 *	Multiply var1 by var2 and shift the result left by 1.
 *	Add the 32 bit result to L_var3 without saturation, return a 32 bit result.
 *	Generate carry and overflow values :
 *		- multiplyAddConstXSaturation_16_32_32(L_var3,var1,var2) = addWithCarry_32_32(L_var3,multiply_16_32(var1,var2))
 *
 *  Complexity weight : 1
 *
 *	@return Return 32-bit long signed integer output value.
 *	@warning In some cases the Carry flag has to be cleared or set before using operators 
 *	which take into account its value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int32
PowerASR_BasicOP_multiplyAddConstXSaturation_16_32_32(hci_int32 L_var3,		///< 32 bit long signed integer
													  hci_int16 var1,		///< 16 bit short signed integer
													  hci_int16 var2)		///< 16 bit short signed integer
{
    hci_int32 L_var_out = 0;
    hci_int32 L_product = 0;

    L_product = PowerASR_BasicOP_multiply_16_32(var1, var2);

    L_var_out = PowerASR_BasicOP_addWithCarry_32_32(L_var3, L_product);

    return (L_var_out);
}


/**
 *	Multiply var1 by var2 and shift the result left by 1.
 *	Subtract the 32 bit result to L_var3 without saturation, return a 32 bit result:
 *	Generate carry and overflow values :
 *		- multiplySubtractConstXSaturation_16_32_32(L_var3,var1,var2) = subtractWithCarry_32_32(L_var3,multiply_16_32(var1,var2)).
 *
 *  Complexity weight : 1
 *
 *	@return Return 32-bit long signed integer output value.
 *	@warning In some cases the Carry flag has to be cleared or set before using operators 
 *	which take into account its value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int32
PowerASR_BasicOP_multiplySubtractConstXSaturation_16_32_32(hci_int32 L_var3,	///< 32 bit long signed integer
														   hci_int16 var1,		///< 16 bit short signed integer
														   hci_int16 var2)		///< 16 bit short signed integer
{
    hci_int32 L_var_out = 0;

    L_var_out = PowerASR_BasicOP_multiply_16_32(var1, var2);

    L_var_out = PowerASR_BasicOP_subtractWithCarry_32_32(L_var3, L_var_out);

    return (L_var_out);
}


/**
 *	Performs the 32 bits addition of the two 32 bits variables (L_var1+L_var2) with overflow control and saturation;
 *
 *   the result is set at +2147483647 when overflow occurs or at -2147483648 when underflow occurs.
 *
 *  Complexity weight : 2
 *
 *	@return Return 32-bit long signed integer output value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int32
PowerASR_BasicOP_add_32_32(hci_int32 L_var1,	///< 32 bit long signed integer
						   hci_int32 L_var2)	///< 32 bit long signed integer
{
    hci_int32 L_var_out = 0;

    L_var_out = L_var1 + L_var2;

    if (((L_var1 ^ L_var2) & MIN_INT32) == 0) {
        if ((L_var_out ^ L_var1) & MIN_INT32) {
            L_var_out = (L_var1 < 0) ? MIN_INT32 : MAX_INT32;
            bOverflow = 1;
        }
    }

    return (L_var_out);
}


/**
 *	Performs the 32 bits subtraction of the two 32 bits variables (L_var1-L_var2) with overflow control and saturation;
 *
 *   the result is set at +2147483647 when overflow occurs or at -2147483648 when underflow occurs.
 *
 *  Complexity weight : 2
 *
 *	@return Return 32-bit long signed integer output value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int32
PowerASR_BasicOP_subtract_32_32(hci_int32 L_var1,	///< 32 bit long signed integer
								hci_int32 L_var2)	///< 32 bit long signed integer
{
    hci_int32 L_var_out = 0;

    L_var_out = L_var1 - L_var2;

    if (((L_var1 ^ L_var2) & MIN_INT32) != 0) {
        if ((L_var_out ^ L_var1) & MIN_INT32) {
            L_var_out = (L_var1 < 0L) ? MIN_INT32 : MAX_INT32;
            bOverflow = 1;
        }
    }

    return (L_var_out);
}


/**
 *	Performs the 32 bits addition of the two 32 bits variables (L_var1+L_var2) with carry.
 *
 *	No saturation. Generate carry and Overflow values.
 *	The carry and overflow values are binary variables which can be tested and assigned values.
 *
 *  Complexity weight : 2
 *
 *	@return Return 32-bit long signed integer output value.
 *	@warning In some cases the Carry flag has to be cleared or set before using
 *	operators which take into account its value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int32
PowerASR_BasicOP_addWithCarry_32_32(hci_int32 L_var1,	///< 32 bit long signed integer
									hci_int32 L_var2)	///< 32 bit long signed integer
{
    hci_int32 L_var_out = 0;
    hci_int32 L_test = 0;
    hci_flag carry_int = 0;

    L_var_out = L_var1 + L_var2 + bCarry;

    L_test = L_var1 + L_var2;

    if ((L_var1 > 0) && (L_var2 > 0) && (L_test < 0)) {
        bOverflow = 1;
        carry_int = 0;
    }
    else {
        if ((L_var1 < 0) && (L_var2 < 0)) {
            if (L_test >= 0) {
                bOverflow = 1;
                carry_int = 1;
			}
            else {
                bOverflow = 0;
                carry_int = 1;
			}
        }
        else {
            if (((L_var1 ^ L_var2) < 0) && (L_test >= 0)) {
                bOverflow = 0;
                carry_int = 1;
            }
            else {
                bOverflow = 0;
                carry_int = 0;
            }
        }
    }

    if (bCarry) {
        if (L_test == MAX_INT32) {
            bOverflow = 1;
            bCarry = carry_int;
        }
        else {
            if (L_test == (hci_int32) 0xFFFFFFFFL) {
                bCarry = 1;
            }
            else {
                bCarry = carry_int;
            }
        }
    }
    else {
        bCarry = carry_int;
    }

    return (L_var_out);
}


/**
 *	Performs the 32 bits subtraction of the two 32 bits variables (L_var1-L_var2) with carry (borrow).
 *
 *	No saturation. Generate carry and Overflow values.
 *	The carry and overflow values are binary variables which can be tested and assigned values.
 *
 *  Complexity weight : 2
 *
 *	@return Return 32-bit long signed integer output value.
 *	@warning In some cases the Carry flag has to be cleared or set before using
 *	operators which take into account its value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int32
PowerASR_BasicOP_subtractWithCarry_32_32(hci_int32 L_var1,	///< 32 bit long signed integer
										 hci_int32 L_var2)	///< 32 bit long signed integer
{
    hci_int32 L_var_out = 0;
    hci_int32 L_test = 0;
    hci_int32 carry_int = 0;

    if (bCarry) {
        bCarry = 0;
        if (L_var2 != MIN_INT32) {
            L_var_out = PowerASR_BasicOP_addWithCarry_32_32(L_var1, -L_var2);
        }
        else {
            L_var_out = L_var1 - L_var2;
            if (L_var1 > 0L) {
                bOverflow = 1;
                bCarry = 0;
            }
        }
    }
    else {
        L_var_out = L_var1 - L_var2 - (hci_int32) 0X00000001L;
        L_test = L_var1 - L_var2;

        if ((L_test < 0) && (L_var1 > 0) && (L_var2 < 0)) {
            bOverflow = 1;
            carry_int = 0;
        }
        else if ((L_test > 0) && (L_var1 < 0) && (L_var2 > 0)) {
            bOverflow = 1;
            carry_int = 1;
        }
        else if ((L_test > 0) && ((L_var1 ^ L_var2) > 0)) {
            bOverflow = 0;
            carry_int = 1;
        }
        if (L_test == MIN_INT32) {
            bOverflow = 1;
            bCarry = carry_int;
        }
        else {
            bCarry = carry_int;
        }
    }

    return (L_var_out);
}


/**
 *	Negate the 32 bit variable L_var1 with saturation;
 *	 - saturate in the case where input is -2147483648 (0x8000 0000).
 *
 *  Complexity weight : 2
 *
 *	@return Return 32-bit long signed integer output value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int32
PowerASR_BasicOP_negate_32_32(hci_int32 L_var1)		///< 32 bit long signed integer
{
    hci_int32 L_var_out = 0;

    L_var_out = (L_var1 == MIN_INT32) ? MAX_INT32 : -L_var1;

    return (L_var_out);
}


/**
 *	Same as multiply_16_16 with rounding, i.e.:
 *		- multiplyWithRound_16_16(var1,var2) = extractLSB_32_16(shiftRight_32_32(((var1 * var2) + 16384),15))
 *		- multiplyWithRound_16_16(-32768,-32768) = 32767.
 *
 *  Complexity weight : 2
 *
 *	@return Return 16-bit short signed integer output value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int16
PowerASR_BasicOP_multiplyWithRound_16_16(hci_int16 var1,	///< 16 bit short signed integer
										 hci_int16 var2)		///< 16 bit short signed integer
{
    hci_int16 var_out = 0;
    hci_int32 L_product_arr = 0;

    L_product_arr = (hci_int32) var1 *(hci_int32) var2;       /* product */
    L_product_arr += (hci_int32) 0x00004000L;      /* round */
    L_product_arr &= (hci_int32) 0xffff8000L;
    L_product_arr >>= 15;       /* shift */

    if (L_product_arr & (hci_int32) 0x00010000L) {  /* sign extend when necessary */
        L_product_arr |= (hci_int32) 0xffff0000L;
    }

    //var_out = _saturate (L_product_arr);
	var_out = PowerASR_BasicOP_extractLSB_32_16(L_product_arr);

    return (var_out);
}


/**
 *	Arithmetically shift the 32 bit input L_var1 left var2 positions.
 *	Zero fill the var2 LSB of the result.
 *	If var2 is negative, arithmetically shift L_var1 right by -var2 with sign extension.
 *	Saturate the result in case of underflows or overflows.
 *
 *  Complexity weight : 2
 *
 *	@return Return 32-bit long signed integer output value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int32
PowerASR_BasicOP_shiftLeft_32_32(hci_int32 L_var1,		///< 32 bit long signed integer
								 hci_int16 var2)		///< 16 bit short signed integer
{
    hci_int32 L_var_out = 0;

    if (var2 <= 0) {
        if (var2 < -32)
            var2 = -32;
        L_var_out = PowerASR_BasicOP_shiftRight_32_32 (L_var1, (hci_int16)-var2);
    }
    else {
        for (; var2 > 0; var2--) {
            if (L_var1 > (hci_int32) 0X3fffffffL) {
                bOverflow = 1;
                L_var_out = MAX_INT32;
                break;
            }
            else {
                if (L_var1 < (hci_int32) 0xc0000000L) {
                    bOverflow = 1;
                    L_var_out = MIN_INT32;
                    break;
                }
            }
            L_var1 *= 2;
            L_var_out = L_var1;
        }
    }

    return (L_var_out);
}


/**
 *	Arithmetically shift the 32 bit input L_var1 right var2 positions with sign extension.
 *	If var2 is negative, arithmetically shift L_var1 left by -var2 and zero fill the -var2 LSB of the result.
 *	Saturate the result in case of underflows or overflows.
 *
 *  Complexity weight : 2
 *
 *	@return Return 32-bit long signed integer output value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int32
PowerASR_BasicOP_shiftRight_32_32(hci_int32 L_var1,		///< 32 bit long signed integer
								  hci_int16 var2)		///< 16 bit short signed integer
{
    hci_int32 L_var_out = 0;

    if (var2 < 0) {
        if (var2 < -32)
            var2 = -32;
        L_var_out = PowerASR_BasicOP_shiftLeft_32_32 (L_var1, (hci_int16)-var2);
    }
    else {
        if (var2 >= 31) {
            L_var_out = (L_var1 < 0L) ? -1 : 0;
        }
        else {
            if (L_var1 < 0) {
                L_var_out = ~((~L_var1) >> var2);
            }
            else {
                L_var_out = L_var1 >> var2;
            }
        }
    }

    return (L_var_out);
}


/**
 *	Same as shiftRight_16_16(var1,var2) but with rounding.
 *	Saturate the result in case of underflows or overflows :
 *		- If var2 is greater than zero :
 *			if (subtract_16_16(shiftLeft_16_16(shiftRight_16_16(var1,var2),1),shiftRight_16_16(var1,subtract_16_16(var2,1)))) is equal to zero
 *				then shiftRightWithRound_16_16(var1,var2) = shiftRight_16_16(var1,var2)
 *			else shiftRightWithRound_16_16(var1,var2) = add_16_16(shiftRight_16_16(var1,var2),1)
 *		- If var2 is less than or equal to zero :
 *			shiftRightWithRound_16_16(var1,var2) = shiftRight_16_16(var1,var2).
 *
 *  Complexity weight : 2
 *
 *	@return Return 16-bit short signed integer output value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int16
PowerASR_BasicOP_shiftRightWithRound_16_16(hci_int16 var1,		///< 16 bit short signed integer
										   hci_int16 var2)		///< 16 bit short signed integer
{
    hci_int16 var_out = 0;

    if (var2 > 15) {
        var_out = 0;
    }
    else {
        var_out = PowerASR_BasicOP_shiftRight_16_16(var1, var2);

        if (var2 > 0) {
            if ((var1 & ((hci_int16) 1 << (var2 - 1))) != 0) {
                var_out++;
            }
        }
    }

    return (var_out);
}


/**
 *	Multiply var1 by var2 and shift the result left by 1.
 *	Add the 32 bit result to L_var3 with saturation.
 *	Round the LS 16 bits of the result into the MS 16 bits with saturation and shift the result right by 16.
 *	Return a 16 bit result.
 *		- multiplyAddConstWithRound_16_32_16(L_var3,var1,var2) = round_32_16(multiplyAddConst_16_32_32(L_var3,var1,var2))
 *
 *  Complexity weight : 2
 *
 *	@return Return 16-bit short signed integer output value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int16
PowerASR_BasicOP_multiplyAddConstWithRound_16_32_16(hci_int32 L_var3,	///< 32 bit long signed integer
													hci_int16 var1,		///< 16 bit short signed integer
													hci_int16 var2)		///< 16 bit short signed integer
{
    hci_int16 var_out = 0;

	L_var3 = PowerASR_BasicOP_multiplyAddConst_16_32_32(L_var3, var1, var2);

    L_var3 = PowerASR_BasicOP_add_32_32(L_var3, (hci_int32) 0x00008000L);

	var_out = PowerASR_BasicOP_extractMSB_32_16(L_var3);

    return (var_out);
}


/**
 *	Multiply var1 by var2 and shift the result left by 1.
 *	Subtract the 32 bit result to L_var3 with saturation.
 *	Round the LS 16 bits of the result into the MS 16 bits with saturation and shift the result right by 16.
 *	Return a 16 bit result.
 *		- multiplySubtractConstWithRound_16_32_16(L_var3,var1,var2) = round_32_16(multiplySubtractConst_16_32_32(L_var3,var1,var2))
 *
 *  Complexity weight : 2
 *
 *	@return Return 16-bit short signed integer output value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int16
PowerASR_BasicOP_multiplySubtractConstWithRound_16_32_16(hci_int32 L_var3,	///< 32 bit long signed integer
														 hci_int16 var1,	///< 16 bit short signed integer
														 hci_int16 var2)	///< 16 bit short signed integer
{
    hci_int16 var_out = 0;

	L_var3 = PowerASR_BasicOP_multiplySubtractConst_16_32_32(L_var3, var1, var2);

    L_var3 = PowerASR_BasicOP_add_32_32(var1, (hci_int32) 0x00008000L);

    var_out = PowerASR_BasicOP_extractMSB_32_16(L_var3);

    return (var_out);
}


/**
 *	Deposit the 16 bit var1 into the 16 MS bits of the 32 bit output.
 *	The 16 LS bits of the output are zeroed.
 *
 *  Complexity weight : 2
 *
 *	@return Return 32-bit long signed integer output value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int32
PowerASR_BasicOP_depositMSB_16_32(hci_int16 var1)		///< 16 bit short signed integer
{
    hci_int32 L_var_out = 0;

    L_var_out = (hci_int32) var1 << 16;

    return (L_var_out);
}


/**
 *	Deposit the 16 bit var1 into the 16 LS bits of the 32 bit output.
 *	The 16 MS bits of the output are sign extended.
 *
 *  Complexity weight : 2
 *
 *	@return Return 32-bit long signed integer output value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int32
PowerASR_BasicOP_depositLSB_16_32(hci_int16 var1)		///< 16 bit short signed integer
{
    hci_int32 L_var_out = 0;

    L_var_out = (hci_int32) var1;

    return (L_var_out);
}


/**
 *	Same as shiftRight_32_32(L_var1,var2) but with rounding.
 *	Saturate the result in case of underflows or overflows :
 *		- If var2 is greater than zero :
 *			if (subtract_32_32(shiftLeft_32_32(shiftRight_32_32(L_var1,var2),1),shiftRight_32_32(L_var1,subtract_16_16(var2,1)))) is equal to zero
 *				then shiftRightWithRound_32_32(L_var1,var2) = shiftRight_32_32(L_var1,var2)
 *			else shiftRightWithRound_32_32(L_var1,var2) = add_32_32(shiftRight_32_32(L_var1,var2),1)
 *		- If var2 is less than or equal to zero : 
 *			shiftRightWithRound_32_32(L_var1,var2) = shiftRight_32_32(L_var1,var2).
 *
 *  Complexity weight : 3
 *
 *	@return Return 32-bit long signed integer output value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int32
PowerASR_BasicOP_shiftRightWithRound_32_32(hci_int32 L_var1,	///< 32 bit long signed integer
										   hci_int16 var2)		///< 16 bit short signed integer
{
    hci_int32 L_var_out = 0;

    if (var2 > 31) {
        L_var_out = 0;
    }
    else {
        L_var_out = PowerASR_BasicOP_shiftRight_32_32(L_var1, var2);

        if (var2 > 0) {
            if ((L_var1 & ((hci_int32) 1 << (var2 - 1))) != 0) {
                L_var_out++;
            }
        }
    }

    return (L_var_out);
}


/**
 *	Absolute value of L_var1.
 *	Saturate in case where the input is -214783648.
 *
 *  Complexity weight : 3
 *
 *	@return Return 32-bit long signed integer output value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int32
PowerASR_BasicOP_abs_32_32(hci_int32 L_var1)	///< 32 bit long signed integer
{
    hci_int32 L_var_out = 0;

    if (L_var1 == MIN_INT32) {
        L_var_out = MAX_INT32;
    }
    else {
        if (L_var1 < 0) {
            L_var_out = -L_var1;
        }
        else {
            L_var_out = L_var1;
        }
    }

    return (L_var_out);
}


/**
 *	32 bit L_var1 is set to 2147483647 if an overflow occurred or
 *	to -2147483648 if an underflow occurred on the most recent addWithCarry_32_32,
 *	subtractWithCarry_32_32, multiplyAddConstXSaturation_16_32_32 or PowerASR_BasicOP_multiplySubtractConstXSaturation_16_32_32 operations.
 *	The carry and overflow values are binary values which can be tested and assigned values.
 *
 *  Complexity weight : 4
 *
 *	@return Return 32-bit long signed integer output value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int32
PowerASR_BasicOP_saturate_32_32(hci_int32 L_var1)	///< 32 bit long signed integer
{
    hci_int32 L_var_out = 0;

    L_var_out = L_var1;

    if (bOverflow) {

        if (bCarry) {
            L_var_out = MIN_INT32;
        }
        else {
            L_var_out = MAX_INT32;
        }

        bCarry = 0;
        bOverflow = 0;
    }

    return (L_var_out);
}


/**
 *	Produces the number of left shift needed to normalize the 16 bit variable var1
 *	for positive values on the interval with minimum of 16384 and
 *	maximum of 32767, and for negative values on the interval with minimum
 *	of -32768 and maximum of -16384; in order to normalize the result,
 *	the following operation must be done :
 *		- norm_var1 = shiftLeft_16_16(var1,getNormalShiftCount_16_16(var1)).
 *
 *  Complexity weight : 15
 *
 *	@return Return 16-bit short signed integer output value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int16
PowerASR_BasicOP_getNormalShiftCount_16_16(hci_int16 var1)		///< 16 bit short signed integer
{
	hci_int16 var_out = 0;

    if (var1 == 0) {
        var_out = 0;
    }
    else {
        if (var1 == -1) {
            var_out = 15;
        }
        else {
            if (var1 < 0) {
                var1 = (hci_int16)~var1;
            }
            for (var_out = 0; var1 < 0x4000; var_out++) {
                var1 <<= 1;
            }
        }
    }

    return (var_out);
}


/**
 *	Produces the number of left shift needed to normalize the 16 bit variable array var_vector
 *	for positive values on the interval with minimum of 16384 and
 *	maximum of 32767, and for negative values on the interval with minimum
 *	of -32768 and maximum of -16384; in order to normalize the result,
 *	the following operation must be done :
 *		- norm_var1 = shiftLeft_16_16(var1,getNormalShiftCount_16_16(var1)).
 *
 *  Complexity weight : 15
 *
 *	@return Return 16-bit short signed integer output value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int16
PowerASR_BasicOP_getNormalShiftCountOfVector_16_16(hci_int16 *var_vector,	///< array of 16 bit short signed integer
												   hci_int16 vector_size,	///< array size
												   hci_int16 heads)			///< head value
{
	register hci_int16 i = 0;
	hci_int16 var_max = 0;
	hci_int16 var_abs = 0;
	hci_int16 var_out = 0;
	hci_int16 var_round = 0;

	for(i = 0; i < vector_size; i++) {
		var_abs = PowerASR_BasicOP_abs_16_16(var_vector[i]);
		var_max = HCI_MAX(var_max, var_abs);
	}

	var_out = PowerASR_BasicOP_getNormalShiftCount_16_16(var_max) - heads;
	for(i = 0; i < vector_size; i++) {
		if (var_out >= 0) {
			var_vector[i] = PowerASR_BasicOP_shiftLeft_16_16(var_vector[i], var_out);
		}
		else {
			if (var_out < -15) {
				var_vector[i] = 0;
			}
			else {
				var_round = PowerASR_BasicOP_shiftLeft_16_16(var_vector[i], (hci_int16)(var_out+1)) & 0x1;
				var_vector[i] = PowerASR_BasicOP_add_16_16(PowerASR_BasicOP_shiftLeft_16_16(var_vector[i], var_out), var_round);
			}
		}
	}

    return (var_out);
}


/**
 *	Produces a result which is the fractional integer division of var1 by var2.
 *	var1 and var2 must be positive and var2 must be greater or equal to var1;
 *	the result is positive (leading bit equal to 0) and truncated to 16 bits.
 *	If var1 = var2 then div(var1,var2) = 32767.
 *
 *  Complexity weight : 18
 *
 *	@return Return 16-bit short signed integer output value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int16
PowerASR_BasicOP_divide_16_16(hci_int16 var1,		///< 16 bit short signed integer
							  hci_int16 var2)		///< 16 bit short signed integer
{
    hci_int16 var_out = 0;
	hci_int16 iteration = 0;
	hci_int32 L_num = 0;
    hci_int32 L_denom = 0;

    if ((var1 > var2) || (var1 < 0) || (var2 < 0)) {
		HCIMSG_ERROR("Division Error var1=%d  var2=%d\n", var1, var2);
        return 0;
    }
    if (var2 == 0) {
        HCIMSG_ERROR("Division by 0, Fatal error \n");
        return 0;
    }
    if (var1 == 0) {
        var_out = 0;
    }
    else {
        if (var1 == var2) {
            var_out = MAX_INT16;
        }
        else {
            L_num = PowerASR_BasicOP_depositLSB_16_32(var1);
            L_denom = PowerASR_BasicOP_depositLSB_16_32(var2);

            for (iteration = 0; iteration < 15; iteration++) {
                var_out <<= 1;
                L_num <<= 1;

                if (L_num >= L_denom)
                {
                    L_num = PowerASR_BasicOP_subtract_32_32(L_num, L_denom);
                    var_out = PowerASR_BasicOP_add_16_16(var_out, 1);
                }
            }
        }
    }

    return (var_out);
}


/**
 *	Produces the number of left shift needed to normalize the 32 bit variable L_var1
 *	for positive values on the interval with minimum of 1073741824 and
 *	maximum of 2147483647, and for negative values on the interval with minimum
 *	of -2147483648 and maximum of -1073741824; in order to normalize the result,
 *	the following operation must be done :
 *		- norm_L_var1 = shiftLeft_32_32(L_var1,getNormalShiftCount_32_16(L_var1)).
 *
 *  Complexity weight : 30
 *
 *	@return Return 16-bit short signed integer output value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int16
PowerASR_BasicOP_getNormalShiftCount_32_16(hci_int32 L_var1)		///< 32 bit long signed integer
{
	hci_int16 var_out = 0;

    if (L_var1 == 0) {
        var_out = 0;
    }
    else {
        if (L_var1 == (hci_int32) 0xffffffffL) {
            var_out = 31;
        }
        else {
            if (L_var1 < 0) {
                L_var1 = ~L_var1;
            }
            for (var_out = 0; L_var1 < (hci_int32) 0x40000000L; var_out++) {
                L_var1 <<= 1;
            }
        }
    }

    return (var_out);
}


/**
 *	separate a 32 bit long integer into two 16 bit DPF.
 *
 *  Complexity weight : 4
 */
HCILAB_PUBLIC HCI_BASICOP_API void
PowerASR_BasicOP_separateBits_32_16(hci_int32 L_var,		///< (i) 32 bit long signed integer
									hci_int16 *hi_var,		///< (o) 16 bit MSB of 32 bit long singed integer
									hci_int16 *lo_var)		///< (o) 16 bit LSB of 32 bit long singed integer
{
	hci_int32 var_shr = 0;
	hci_int32 var_msu = 0;

    *hi_var = PowerASR_BasicOP_extractMSB_32_16(L_var);

	var_shr = PowerASR_BasicOP_shiftRight_32_32(L_var, 1);
	var_msu = PowerASR_BasicOP_multiplySubtractConst_16_32_32(var_shr, *hi_var, 16384);
    *lo_var = PowerASR_BasicOP_extractLSB_32_16(var_msu);
}


/**
 *	Produce a 32 bit long signed integer from two 16 bit DPFs.
 *
 *  Complexity weight : 2
 *
 *	@return Return 32-bit long signed integer output value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int32
PowerASR_BasicOP_composeBits_16_32(hci_int16 var_hi,		///< (i) 16 bit MSB short signed integer
								   hci_int16 var_lo)		///< (i) 16 bit LSB short signed integer
{
    hci_int32 L_var_out = 0;

    L_var_out = PowerASR_BasicOP_depositMSB_16_32(var_hi);
	L_var_out = PowerASR_BasicOP_multiplyAddConst_16_32_32(L_var_out, var_lo, 1);

	return L_var_out;
}


/**
 *	Produce a 32 bit long signed integer from two 16 bit DPFs.
 *
 *  Complexity weight : 2
 *
 *	@return Return 32-bit long signed integer output value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int32
PowerASR_BasicOP_multiply2DPFs_32_32(hci_int16 var_hi1,		///< 16 bit MSB part of first long signed integer
									 hci_int16 var_lo1,		///< 16 bit LSB part of first long signed integer
									 hci_int16 var_hi2,		///< 16 bit MSB part of second long signed integer
									 hci_int16 var_lo2)		///< 16 bit LSB part of second long signed integer
{
	hci_int32 L_var_out = 0;

	L_var_out = PowerASR_BasicOP_multiply_16_32(var_hi1, var_hi2);
	L_var_out = PowerASR_BasicOP_multiplyAddConst_16_32_32(L_var_out, PowerASR_BasicOP_multiply_16_16(var_hi1, var_lo2), 1);
	L_var_out = PowerASR_BasicOP_multiplyAddConst_16_32_32(L_var_out, PowerASR_BasicOP_multiply_16_16(var_lo1, var_hi2), 1);

	return L_var_out;
}


/**
 *	Multiply a 32 bit MSB/LSB long signed integer to a 16 bit short signed integer.
 *
 *  Complexity weight : 2
 *
 *	@return Return 32-bit long signed integer output value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int32
PowerASR_BasicOP_multiply_32_16_32(hci_int16 var_hi1,	///< 16 bit MSB part of first long signed integer
								   hci_int16 var_lo1,	///< 16 bit LSB part of first long signed integer
								   hci_int16 var2)		///< second 16 bit short signed integer
{
	hci_int32 L_var_out = 0;

	L_var_out = PowerASR_BasicOP_multiply_16_32(var_hi1, var2);
	L_var_out = PowerASR_BasicOP_multiplyAddConst_16_32_32(L_var_out, PowerASR_BasicOP_multiply_16_16(var_lo1, var2), 1);

	return L_var_out;
}


/**
 *	Divide a 32 bit long signed integer to a 32 bit MSB/LSB long signed integer.
 *
 *  Complexity weight : 18
 *
 *	@return Return 32-bit long signed integer output value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int32
PowerASR_BasicOP_divide_32_32(hci_int32 L_var_num,		///< 32 bit long signed integer (numerator)
							  hci_int16 var_denom_hi,	///< 16 bit MSB part of long signed integer (denominator)
							  hci_int16 var_denom_lo)	///< 16 bit LSB part of long signed integer (denominator)
{
    hci_int16 var_approx = 0;
	hci_int16 var_out_hi = 0;
	hci_int16 var_out_lo = 0;
	hci_int16 var_num_hi = 0;
	hci_int16 var_num_lo = 0;
    hci_int32 L_var_out = 0;

    // First approximation: 1 / L_denom = 1/denom_hi
    var_approx = PowerASR_BasicOP_divide_16_16((hci_int16) 0x3fff, var_denom_hi);

    // 1/L_denom = approx * (2.0 - L_denom * approx)
    L_var_out = PowerASR_BasicOP_multiply_32_16_32(var_denom_hi, var_denom_lo, var_approx);

    L_var_out = PowerASR_BasicOP_subtract_32_32((hci_int32) 0x7fffffffL, L_var_out);

    PowerASR_BasicOP_separateBits_32_16(L_var_out, &var_out_hi, &var_out_lo);

    L_var_out = PowerASR_BasicOP_multiply_32_16_32(var_out_hi, var_out_lo, var_approx);

    // L_num * (1/L_denom)
    PowerASR_BasicOP_separateBits_32_16(L_var_out, &var_out_hi, &var_out_lo);
    PowerASR_BasicOP_separateBits_32_16(L_var_num, &var_num_hi, &var_num_lo);
    L_var_out = PowerASR_BasicOP_multiply2DPFs_32_32(var_num_hi, var_num_lo, var_out_hi, var_out_lo);
    L_var_out = PowerASR_BasicOP_shiftLeft_32_32(L_var_out, 2);

    return (L_var_out);
}


/**
 *	Multiply two short signed integers,
 *	then arithmetically shift the value left var3 positions.
 *
 *  Complexity weight : 2
 *
 *	@return Return 16-bit short signed integer output value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int16
PowerASR_BasicOP_multiplyShiftRight_16_16(hci_int16 var1,	///< 16 bit short signed integer
										  hci_int16 var2,	///< 16 bit short signed integer
										  hci_int16 var3)	///< 16 bit short signed integer
{
	hci_int16 var_out = 0;
	hci_int32 L_var = 0;

	L_var = PowerASR_BasicOP_multiply_16_32(var1, var2);
	L_var = PowerASR_BasicOP_shiftRight_32_32(L_var,var3);
	var_out = PowerASR_BasicOP_extractLSB_32_16(L_var);
	//var_out = (hci_int16)L_var;

	return var_out;
}


/**
 *	Divide a 32 bit long signed integer to a 32 bit MSB/LSB long signed integer,
 *	then arithmetically shift the value left var3 positions.
 *
 *  Complexity weight : 24
 *
 *	@return Return 32-bit long signed integer output value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int32
PowerASR_BasicOP_divideShiftLeft_32_32(hci_int32 L_var1,	///< 32 bit long signed integer
									   hci_int32 L_var2,	///< 32 bit long signed integer
									   hci_int16 var3)		///< 16 bit short signed integer
{
	hci_int32 L_var_out = 0;
	hci_int16 var_sign = 0;
	hci_int16 var_count1 = 0;
	hci_int16 var_count2 = 0;
	hci_int16 var_hi = 0;
	hci_int16 var_lo = 0;
	hci_int16 var_shift = 0;
	hci_int16 var_alpha = 0;
	
	if (0 == L_var1) return L_var_out;
	
	if ((L_var1 > 0 && L_var2 > 0) || (L_var1 < 0 && L_var2 < 0)) {
		var_sign = 1;
	}
	else {
		var_sign = -1;
	}
	
	if (L_var1 < 0) L_var1 = -L_var1;
	if (L_var2 < 0) L_var2 = -L_var2;
	
	var_count1 = PowerASR_BasicOP_getNormalShiftCount_32_16(L_var1) - 1;
	var_count2 = PowerASR_BasicOP_getNormalShiftCount_32_16(L_var2);
	
	if (var_count1 < 0) {
		L_var1 = L_var1 >> (-var_count1);
		L_var2 = L_var2 >> (-var_count1);
		var_count2 = var_count2-var_count1;
		var_count1 = 0;
	}
	
	L_var1 = L_var1 << var_count1;
	L_var2 = L_var2 << var_count2;
	
	PowerASR_BasicOP_separateBits_32_16(L_var2, &var_hi, &var_lo);
	L_var_out = PowerASR_BasicOP_divide_32_32(L_var1, var_hi, var_lo);
	
	var_alpha = var_count2-var_count1;
	var_shift = var3 + var_alpha;
	if (var_shift < 0) var_shift = 0;
	
	L_var_out = L_var_out >> (31-var_shift);
	L_var_out *= var_sign;
	
	return L_var_out;
}


/**
 *	CORDIC-based Q14/16bit sin/cos value.
 *
 *  Complexity weight : ?
 *
 *	@return Return 16-bit short signed integer output value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int16
PowerASR_BasicOP_fixedCosine_32_16(hci_int32 L_var1,	///< sin(or cos) argument(Q15/32bit). (must be not radian but degree)
								   hci_int16 bCosSin)	///< sin/cos flag bit (cosine bCosSin = 0, sine bCosSin = 1)
{
	hci_int16 i = 0;
	hci_int16 x = 0;
	hci_int16 y = 0;
	hci_int16 newx = 0;
	hci_int16 var_out = 0;
	hci_int16 invert = 0;
	hci_int32 L_crt = 0;
	hci_int32 L_withinTPI = 0;
	hci_int32 L_alpha_q15[] = {1474560, 870483, 459939, 233472, 117189,
								58651, 29333, 14667, 7333, 3666, 
								1833, 916, 458};
	hci_int16 alpha_size = 13;

	invert = 0;

	// restrict input degree between 0 and 360
	L_withinTPI = L_var1;
	while (L_withinTPI >= TPI_CONST) {
		L_withinTPI -= TPI_CONST;
	}

	// restrict degree between -90 and 90
	if ((L_withinTPI > 2949120) && (L_withinTPI <= 8847360)) {		// case-1 : 90 < x <= 270
		invert = 1;
		L_crt = -(L_withinTPI - 5898240);
	}
	else if((L_withinTPI > 8847360) && (L_withinTPI <= 11796480)) {	// case2 : 270 < x <= 360 
		L_crt = L_withinTPI - 11796480;
	}
	else {															// case3 : 0 < x <= 90
		L_crt = L_withinTPI;
	}

	// initialize CORDIC rotation mode ... 
	x = AG_CONST;		// AG_CONST * cos(0) 
	y = 0;				// AG_CONST * sin(0) 
	
	// at each step, try to make crt zero ...
	for(i = 0; i < alpha_size; i++) {
		if(L_crt > 0) {
			newx = PowerASR_BasicOP_subtract_16_16(x, PowerASR_BasicOP_shiftRight_16_16(y, i));
			y = PowerASR_BasicOP_add_16_16(y, PowerASR_BasicOP_shiftRight_16_16(x, i));
			x = newx;
			L_crt -= L_alpha_q15[i];
		}
		else {
			newx = PowerASR_BasicOP_add_16_16(x, PowerASR_BasicOP_shiftRight_16_16(y, i));
			y = PowerASR_BasicOP_subtract_16_16(y, PowerASR_BasicOP_shiftRight_16_16(x, i));
			x = newx;
			L_crt += L_alpha_q15[i];
		}
	}

	if(bCosSin)	{				// sine evaluation
		var_out = y;
	}
	else {						// cosine evaluation
		var_out = x;
		if(invert) {
			var_out = -var_out;
		}
	}

	return var_out;				// Q15.16
}


/**
 *	Returns the 16 bit result of the square-root of L_var1.
 *
 *  Complexity weight : ?
 *
 *	@return Return 16-bit short signed integer output value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int16
PowerASR_BasicOP_fixedSQRT_32_16(hci_int32 L_var1)	///< 32 bit long signed integer
{
	hci_int16 var_out = 0;

    register hci_int16	i = 0;
	hci_int16 var_exp = (hci_int16) 0x4000;	// Q14
	hci_int16 var_temp = 0;
	hci_int32 L_var_acc = 0;

	if ( L_var1 > SHRT_MAX * SHRT_MAX ) {
		var_out = SHRT_MAX;
	}
	else {
		for (i = 0 ; i < 14 ; i ++) {
			var_temp = var_out + var_exp;
			L_var_acc = var_temp * var_temp;
			if (L_var1 >= L_var_acc) {
				var_out = var_out + var_exp;
			}
			var_exp = var_exp >> 1;
		}
	}

    return var_out ;
}


/**
 *	Returns the 32 bit result of the square-root of L_var1.
 *
 *  Complexity weight : ?
 *
 *	@return Return 32-bit long signed integer output value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int32
PowerASR_BasicOP_fixedSQRT_32_32(hci_int32 L_var1)	///< 32 bit long signed integer
{
    register unsigned int remHi = 0;	// high part of partial remainder
	register unsigned int remLo = 0;	// low part of partial remainder
	register unsigned int testDiv = 0;
	register unsigned int count = 30;	// Load loop counter
	hci_int32 L_var_out = 0;

	remLo = L_var1;
	
	do {
		remHi = (remHi << 2) | (remLo >> 30); remLo <<= 2;	// get 2 bits of arg 
		L_var_out <<= 1;					// Get ready for the next bit in the root
		testDiv = (L_var_out << 1) + 1;		// Test divisor
		if (remHi >= testDiv) {
			remHi -= testDiv;
			L_var_out += 1;
		}
	} while (count-- != 0);

    return L_var_out;
}


/**
 *	Returns the 32 bit result (Q15.32) of the log value of L_var1.
 *
 *  Complexity weight : ?
 *
 *	@return Return 32-bit long signed integer output value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int32
PowerASR_BasicOP_fixedLOG_32_32(hci_int32 L_var1,		///< 32 bit long signed integer
								hci_int16 var_qform)	///< 16 bit short signed integer
{
	hci_int32 L_var_expof = 0;
	hci_int32 L_var_out = 0;
	hci_int32 L_var_Menti = 0;
	hci_int16 var_offset = 0;
	hci_int16 var_inc = 0;
	hci_int16 var_idx = 0;

	/*
	 *  [comments] LOGF_Q15
	 *
	 *   Q15/32bit format
	 */

	hci_int32	LOGF_Q15[512] = {  0, 64, 128, 191, 255, 318, 382, 445, 508, 571,
								 634, 697, 759, 822, 884, 946, 1008, 1070, 1132, 1194,
								 1256, 1317, 1379, 1440, 1501, 1562, 1623, 1684, 1745, 1805,
								 1866, 1926, 1986, 2047, 2107, 2167, 2227, 2286, 2346, 2405,
								 2465, 2524, 2583, 2642, 2701, 2760, 2819, 2878, 2936, 2995,
								 3053, 3111, 3170, 3228, 3286, 3343, 3401, 3459, 3516, 3574,
								 3631, 3688, 3745, 3802, 3859, 3916, 3973, 4030, 4086, 4143,
								 4199, 4255, 4311, 4367, 4423, 4479, 4535, 4591, 4646, 4702,
								 4757, 4812, 4868, 4923, 4978, 5033, 5088, 5142, 5197, 5252,
								 5306, 5360, 5415, 5469, 5523, 5577, 5631, 5685, 5739, 5792,
								 5846, 5899, 5953, 6006, 6059, 6113, 6166, 6219, 6271, 6324,
								 6377, 6430, 6482, 6535, 6587, 6639, 6692, 6744, 6796, 6848,
								 6900, 6951, 7003, 7055, 7106, 7158, 7209, 7261, 7312, 7363,
								 7414, 7465, 7516, 7567, 7618, 7668, 7719, 7769, 7820, 7870,
								 7920, 7971, 8021, 8071, 8121, 8171, 8221, 8270, 8320, 8370,
								 8419, 8469, 8518, 8567, 8617, 8666, 8715, 8764, 8813, 8862,
								 8910, 8959, 9008, 9056, 9105, 9153, 9202, 9250, 9298, 9346,
								 9394, 9442, 9490, 9538, 9586, 9634, 9681, 9729, 9777, 9824,
								 9871, 9919, 9966, 10013, 10060, 10107, 10154, 10201, 10248, 10295,
								 10342, 10388, 10435, 10481, 10528, 10574, 10620, 10667, 10713, 10759,
								 10805, 10851, 10897, 10943, 10989, 11034, 11080, 11126, 11171, 11217,
								 11262, 11307, 11353, 11398, 11443, 11488, 11533, 11578, 11623, 11668,
								 11713, 11757, 11802, 11847, 11891, 11936, 11980, 12025, 12069, 12113,
								 12157, 12201, 12246, 12290, 12334, 12377, 12421, 12465, 12509, 12552,
								 12596, 12640, 12683, 12726, 12770, 12813, 12856, 12900, 12943, 12986,
								 13029, 13072, 13115, 13158, 13200, 13243, 13286, 13329, 13371, 13414,
								 13456, 13499, 13541, 13583, 13625, 13668, 13710, 13752, 13794, 13836,
								 13878, 13920, 13962, 14003, 14045, 14087, 14128, 14170, 14211, 14253,
								 14294, 14336, 14377, 14418, 14459, 14500, 14541, 14583, 14623, 14664,
								 14705, 14746, 14787, 14828, 14868, 14909, 14950, 14990, 15031, 15071,
								 15111, 15152, 15192, 15232, 15272, 15312, 15353, 15393, 15433, 15473,
								 15512, 15552, 15592, 15632, 15671, 15711, 15751, 15790, 15830, 15869,
								 15909, 15948, 15987, 16027, 16066, 16105, 16144, 16183, 16222, 16261,
								 16300, 16339, 16378, 16417, 16455, 16494, 16533, 16571, 16610, 16649,
								 16687, 16725, 16764, 16802, 16840, 16879, 16917, 16955, 16993, 17031,
								 17069, 17107, 17145, 17183, 17221, 17259, 17297, 17334, 17372, 17410,
								 17447, 17485, 17522, 17560, 17597, 17635, 17672, 17709, 17747, 17784,
								 17821, 17858, 17895, 17932, 17969, 18006, 18043, 18080, 18117, 18154,
								 18190, 18227, 18264, 18300, 18337, 18373, 18410, 18446, 18483, 18519,
								 18556, 18592, 18628, 18664, 18701, 18737, 18773, 18809, 18845, 18881,
								 18917, 18953, 18989, 19025, 19060, 19096, 19132, 19167, 19203, 19239,
								 19274, 19310, 19345, 19381, 19416, 19451, 19487, 19522, 19557, 19593,
								 19628, 19663, 19698, 19733, 19768, 19803, 19838, 19873, 19908, 19943,
								 19977, 20012, 20047, 20082, 20116, 20151, 20185, 20220, 20255, 20289,
								 20323, 20358, 20392, 20427, 20461, 20495, 20529, 20563, 20598, 20632,
								 20666, 20700, 20734, 20768, 20802, 20836, 20870, 20903, 20937, 20971,
								 21005, 21038, 21072, 21106, 21139, 21173, 21206, 21240, 21273, 21307,
								 21340, 21373, 21407, 21440, 21473, 21506, 21540, 21573, 21606, 21639,
								 21672, 21705, 21738, 21771, 21804, 21837, 21870, 21902, 21935, 21968,
								 22001, 22033, 22066, 22099, 22131, 22164, 22196, 22229, 22261, 22294,
								 22326, 22358, 22391, 22423, 22455, 22488, 22520, 22552, 22584, 22616,
								 22648, 22680 };


	var_offset = 31;
	while(var_offset && !(L_var1 & 0x80000000L))
	{
		L_var1 <<= 1;
		var_offset--;
	}

	var_inc = var_offset - var_qform;
	L_var_expof = (hci_int32)(22712 * var_inc);

	L_var1 = (L_var1&0x7fffffff);
	L_var1 = PowerASR_BasicOP_shiftRight_32_32(L_var1, (hci_int16)(16+TBL512_OFFSET));
	var_idx = PowerASR_BasicOP_extractLSB_32_16(L_var1);
	var_idx = HCI_MIN(var_idx,511);
	L_var_Menti = LOGF_Q15[var_idx];
	L_var_out = L_var_expof + L_var_Menti;

	return L_var_out;
}


/** constants in fixedLOG_2 operator */
#define A0  1
#define A1  2943
#define A2  (-1374)
#define A3  640
#define A4  (-161)
#define A0_xx16 (65536L*A0) 

/**
 *	Approximation of log in base 2 i.e. output = 1024 * ln(|x|) / ln(2)
 *
 *  Complexity weight : ?
 *
 *	@return Return 16-bit short signed integer output value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int16
PowerASR_BasicOP_fixedLOG_2(hci_int32 L_Input)		///< 32 bit long signed integer
{
	hci_int16 var_out = 0;
    hci_int32 L_var_log2 = 0;
    hci_int16 var_exp = 0;
    hci_int16 var_mant = 0;
	
    L_Input = PowerASR_BasicOP_abs_32_32(L_Input);
    if( L_Input == 0 ) { 
        return var_out; 
    } 
	
    /* exponent */
    if(PowerASR_BasicOP_subtract_32_32(L_Input,0x8000L) < 0) {
        hci_int16 var_xs = PowerASR_BasicOP_extractLSB_32_16(L_Input) ;
        var_exp = PowerASR_BasicOP_getNormalShiftCount_16_16(var_xs);
        var_mant = PowerASR_BasicOP_shiftLeft_16_16(var_xs, var_exp);
        var_exp = PowerASR_BasicOP_subtract_16_16(14, var_exp);
    } else {
        hci_int16 var_xh = PowerASR_BasicOP_extractMSB_32_16(L_Input);
        if(var_xh == 0) {
            var_exp = 15 ;
        } else {
            var_exp = PowerASR_BasicOP_getNormalShiftCount_16_16(var_xh);
        }
        var_mant = PowerASR_BasicOP_extractMSB_32_16(PowerASR_BasicOP_shiftLeft_32_32(L_Input, var_exp));
        var_exp = PowerASR_BasicOP_subtract_16_16(30, var_exp);
    }
    var_mant = PowerASR_BasicOP_shiftLeft_16_16((hci_int16) ( var_mant & (hci_int16) 0x3fff) , (hci_int16) 1 ) ;
    {   /* polynomial approximation */
        hci_int16 var_m2 = PowerASR_BasicOP_multiply_16_16(var_mant,var_mant) ;
        hci_int16 var_m3 = PowerASR_BasicOP_multiply_16_16(var_mant,var_m2);
        hci_int16 var_m4 = PowerASR_BasicOP_multiply_16_16(var_m2,var_m2) ;
        L_var_log2 = PowerASR_BasicOP_multiplyAddConst_16_32_32(PowerASR_BasicOP_multiplyAddConst_16_32_32(PowerASR_BasicOP_multiplyAddConst_16_32_32(PowerASR_BasicOP_multiplyAddConst_16_32_32(A0_xx16,A1,var_mant),A2,var_m2),A3,var_m3),A4,var_m4);
        L_var_log2 = PowerASR_BasicOP_add_16_16(PowerASR_BasicOP_extractMSB_32_16(PowerASR_BasicOP_shiftRight_32_32(L_var_log2,1)),PowerASR_BasicOP_shiftLeft_16_16(var_exp,10));
    }
	var_out = (hci_int16) L_var_log2;

    return var_out;
}


/**
 *	take the log base 2 of input and divide by 32 and return;
 *	i.e. output = log2(input)/32
 *
 *  Complexity weight : ?
 *
 *	@return Return 32-bit long signed integer output value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int16
PowerASR_BasicOP_fixedLOG2_32_16(hci_int32 L_Input)		///< 32 bit long signed integer
{

	const static hci_int16 swC0 = -0x2b2a;
	const static hci_int16 swC1 = 0x7fc5;
	const static hci_int16 swC2 = -0x54d0;

	hci_int16 iShiftCnt = 0;
	hci_int16 swInSqrd = 0;
	hci_int16 swIn = 0;
	hci_int16 var_out = 0;

	/* normalize input and store shifts required */
	/* ----------------------------------------- */

	iShiftCnt = PowerASR_BasicOP_getNormalShiftCount_32_16(L_Input);
	L_Input = PowerASR_BasicOP_shiftLeft_32_32(L_Input, iShiftCnt);
	iShiftCnt = PowerASR_BasicOP_add_16_16(iShiftCnt, 1);
	iShiftCnt = PowerASR_BasicOP_negate_16_16(iShiftCnt);

	/* calculate x*x*c0 */
	/* ---------------- */

	swIn = PowerASR_BasicOP_extractMSB_32_16(L_Input);
	swInSqrd = PowerASR_BasicOP_multiplyWithRound_16_16(swIn, swIn);
	L_Input = PowerASR_BasicOP_multiply_16_16(swInSqrd, swC0);

	/* add x*c1 */
	/* --------- */

	L_Input = PowerASR_BasicOP_multiplyAddConst_16_32_32(L_Input, swIn, swC1);

	/* add c2 */
	/* ------ */

	L_Input = PowerASR_BasicOP_add_32_32(L_Input, PowerASR_BasicOP_depositMSB_16_32(swC2));

	/* apply *(4/32) */
	/* ------------- */

	L_Input = PowerASR_BasicOP_shiftRight_32_32(L_Input, 3);
	L_Input = L_Input & 0x03ffffff;
	iShiftCnt = PowerASR_BasicOP_shiftLeft_16_16(iShiftCnt, 10);
	L_Input = PowerASR_BasicOP_add_32_32(L_Input, PowerASR_BasicOP_depositMSB_16_32(iShiftCnt));
	var_out = PowerASR_BasicOP_round_32_16(L_Input);

	/* return log */
	/* ---------- */

	return var_out;
}


/**
 *	perform log-addition operation by table look-up
 *	i.e. output = log(A+B) = LAddTable(logA, logB)
 *
 *  Complexity weight : ?
 *
 *	@return Return 32-bit long signed integer output value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_logadd_t
PowerASR_BasicOP_addLOG_32_32(hci_logadd_t L_var1,		///< 32 bit long signed integer
							  hci_logadd_t L_var2,		///< 32 bit long signed integer
							  hci_logadd_t *tableLAdd)	///< log-addition table
{
	hci_logadd_t _d = HCI_ABS((L_var1)-(L_var2));
	hci_logadd_t _max = HCI_MAX((L_var1),(L_var2));
	hci_logadd_t L_var_out = 0;

	L_var_out = ((_d) < LOG_DIFF ? _max+tableLAdd[LOG_IDX(_d)] : _max);

	return L_var_out;
}


/**
 *	perform log-addition operation by table look-up
 *	i.e. output = log(A+B) = LAddTable(logA, logB)
 *
 *  Complexity weight : ?
 *
 *	@return Return 16-bit short signed integer output value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int16
PowerASR_BasicOP_FixedLogAddition(const hci_int16 var1,			///< 16 bit short signed integer
								  const hci_int16 var2,			///< 16 bit short signed integer
								  const hci_int16 *tableLAdd)	///< log-addition table
{
	hci_int16 _d = 0;
	hci_int16 _max = HCI_MAX(var1,var2);
	hci_int16 var_out = _max;

	_d = PowerASR_BasicOP_abs_16_16(PowerASR_BasicOP_subtract_16_16(var1, var2));
	if ((_d) < FIXED_LOG_DIFF) {
		var_out = PowerASR_BasicOP_add_16_16(_max, tableLAdd[_d]);
	}

	return var_out;
}


/**
 *	Set carry value
 */
HCILAB_PUBLIC HCI_BASICOP_API void
PowerASR_BasicOP_setCarry(hci_flag var_carry)	///< new carry value
{
	bCarry = var_carry & 1;
}


/**
 *	Get carry value
 *
 *	@return carry value
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_flag
PowerASR_BasicOP_getCarry(void)
{
	return bCarry;
}


/**
 *	Set overflow value
 */
HCILAB_PUBLIC HCI_BASICOP_API void
PowerASR_BasicOP_setOverflow(hci_flag var_overflow)	///< new overflow value
{
	bOverflow = var_overflow & 1;
}


/**
 *	Get overflow value
 *
 *	@return overflow value
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_flag
PowerASR_BasicOP_getOverflow(void)
{
	return bOverflow;
}


/**
 * Limit the 32 bit input to the range of a 16 bit word.
 *
 * @return 16 bit short signed integer output value
 */

HCILAB_PRIVATE hci_int16 
_saturate(hci_int32 L_var1)		///< 32 bit long signed integer
{
    hci_int16 var_out = 0;

    if (L_var1 > 0X00007fffL) {
        bOverflow = 1;
        var_out = MAX_INT16;
    }
    else if (L_var1 < (hci_int32) 0xffff8000L) {
        bOverflow = 1;
        var_out = MIN_INT16;
    }
    else {
        var_out = PowerASR_BasicOP_extractLSB_32_16(L_var1);
    }

    return (var_out);
}

/**
 *	create log-addition table
 */
HCILAB_PRIVATE void 
_createLogAdditionTable(hci_logadd_t *pLogAddTbl)
{

#ifdef FIXED_POINT_FE
	pLogAddTbl[0] = 22713;
	pLogAddTbl[1] = 22458;
	pLogAddTbl[2] = 22205;
	pLogAddTbl[3] = 21954;
	pLogAddTbl[4] = 21705;
	pLogAddTbl[5] = 21458;
	pLogAddTbl[6] = 21213;
	pLogAddTbl[7] = 20970;
	pLogAddTbl[8] = 20729;
	pLogAddTbl[9] = 20489;
	pLogAddTbl[10] = 20252;
	pLogAddTbl[11] = 20017;
	pLogAddTbl[12] = 19784;
	pLogAddTbl[13] = 19553;
	pLogAddTbl[14] = 19324;
	pLogAddTbl[15] = 19097;
	pLogAddTbl[16] = 18872;
	pLogAddTbl[17] = 18649;
	pLogAddTbl[18] = 18427;
	pLogAddTbl[19] = 18208;
	pLogAddTbl[20] = 17991;
	pLogAddTbl[21] = 17776;
	pLogAddTbl[22] = 17562;
	pLogAddTbl[23] = 17351;
	pLogAddTbl[24] = 17141;
	pLogAddTbl[25] = 16934;
	pLogAddTbl[26] = 16728;
	pLogAddTbl[27] = 16524;
	pLogAddTbl[28] = 16322;
	pLogAddTbl[29] = 16122;
	pLogAddTbl[30] = 15924;
	pLogAddTbl[31] = 15728;
	pLogAddTbl[32] = 15534;
	pLogAddTbl[33] = 15342;
	pLogAddTbl[34] = 15151;
	pLogAddTbl[35] = 14963;
	pLogAddTbl[36] = 14776;
	pLogAddTbl[37] = 14591;
	pLogAddTbl[38] = 14408;
	pLogAddTbl[39] = 14227;
	pLogAddTbl[40] = 14047;
	pLogAddTbl[41] = 13870;
	pLogAddTbl[42] = 13694;
	pLogAddTbl[43] = 13520;
	pLogAddTbl[44] = 13348;
	pLogAddTbl[45] = 13177;
	pLogAddTbl[46] = 13009;
	pLogAddTbl[47] = 12842;
	pLogAddTbl[48] = 12676;
	pLogAddTbl[49] = 12513;
	pLogAddTbl[50] = 12351;
	pLogAddTbl[51] = 12192;
	pLogAddTbl[52] = 12033;
	pLogAddTbl[53] = 11877;
	pLogAddTbl[54] = 11722;
	pLogAddTbl[55] = 11569;
	pLogAddTbl[56] = 11417;
	pLogAddTbl[57] = 11268;
	pLogAddTbl[58] = 11119;
	pLogAddTbl[59] = 10973;
	pLogAddTbl[60] = 10828;
	pLogAddTbl[61] = 10685;
	pLogAddTbl[62] = 10543;
	pLogAddTbl[63] = 10403;
	pLogAddTbl[64] = 10264;
	pLogAddTbl[65] = 10128;
	pLogAddTbl[66] = 9992;
	pLogAddTbl[67] = 9858;
	pLogAddTbl[68] = 9726;
	pLogAddTbl[69] = 9595;
	pLogAddTbl[70] = 9466;
	pLogAddTbl[71] = 9338;
	pLogAddTbl[72] = 9212;
	pLogAddTbl[73] = 9087;
	pLogAddTbl[74] = 8964;
	pLogAddTbl[75] = 8842;
	pLogAddTbl[76] = 8722;
	pLogAddTbl[77] = 8603;
	pLogAddTbl[78] = 8486;
	pLogAddTbl[79] = 8369;
	pLogAddTbl[80] = 8255;
	pLogAddTbl[81] = 8141;
	pLogAddTbl[82] = 8029;
	pLogAddTbl[83] = 7919;
	pLogAddTbl[84] = 7810;
	pLogAddTbl[85] = 7702;
	pLogAddTbl[86] = 7595;
	pLogAddTbl[87] = 7490;
	pLogAddTbl[88] = 7386;
	pLogAddTbl[89] = 7283;
	pLogAddTbl[90] = 7182;
	pLogAddTbl[91] = 7082;
	pLogAddTbl[92] = 6983;
	pLogAddTbl[93] = 6885;
	pLogAddTbl[94] = 6789;
	pLogAddTbl[95] = 6693;
	pLogAddTbl[96] = 6599;
	pLogAddTbl[97] = 6507;
	pLogAddTbl[98] = 6415;
	pLogAddTbl[99] = 6325;
	pLogAddTbl[100] = 6235;
	pLogAddTbl[101] = 6147;
	pLogAddTbl[102] = 6060;
	pLogAddTbl[103] = 5974;
	pLogAddTbl[104] = 5889;
	pLogAddTbl[105] = 5806;
	pLogAddTbl[106] = 5723;
	pLogAddTbl[107] = 5642;
	pLogAddTbl[108] = 5561;
	pLogAddTbl[109] = 5482;
	pLogAddTbl[110] = 5403;
	pLogAddTbl[111] = 5326;
	pLogAddTbl[112] = 5250;
	pLogAddTbl[113] = 5174;
	pLogAddTbl[114] = 5100;
	pLogAddTbl[115] = 5027;
	pLogAddTbl[116] = 4954;
	pLogAddTbl[117] = 4883;
	pLogAddTbl[118] = 4813;
	pLogAddTbl[119] = 4743;
	pLogAddTbl[120] = 4675;
	pLogAddTbl[121] = 4607;
	pLogAddTbl[122] = 4540;
	pLogAddTbl[123] = 4475;
	pLogAddTbl[124] = 4410;
	pLogAddTbl[125] = 4346;
	pLogAddTbl[126] = 4282;
	pLogAddTbl[127] = 4220;
	pLogAddTbl[128] = 4159;
	pLogAddTbl[129] = 4098;
	pLogAddTbl[130] = 4038;
	pLogAddTbl[131] = 3979;
	pLogAddTbl[132] = 3921;
	pLogAddTbl[133] = 3864;
	pLogAddTbl[134] = 3807;
	pLogAddTbl[135] = 3751;
	pLogAddTbl[136] = 3696;
	pLogAddTbl[137] = 3642;
	pLogAddTbl[138] = 3589;
	pLogAddTbl[139] = 3536;
	pLogAddTbl[140] = 3484;
	pLogAddTbl[141] = 3433;
	pLogAddTbl[142] = 3382;
	pLogAddTbl[143] = 3332;
	pLogAddTbl[144] = 3283;
	pLogAddTbl[145] = 3235;
	pLogAddTbl[146] = 3187;
	pLogAddTbl[147] = 3140;
	pLogAddTbl[148] = 3093;
	pLogAddTbl[149] = 3047;
	pLogAddTbl[150] = 3002;
	pLogAddTbl[151] = 2958;
	pLogAddTbl[152] = 2914;
	pLogAddTbl[153] = 2871;
	pLogAddTbl[154] = 2828;
	pLogAddTbl[155] = 2786;
	pLogAddTbl[156] = 2744;
	pLogAddTbl[157] = 2704;
	pLogAddTbl[158] = 2663;
	pLogAddTbl[159] = 2624;
	pLogAddTbl[160] = 2585;
	pLogAddTbl[161] = 2546;
	pLogAddTbl[162] = 2508;
	pLogAddTbl[163] = 2471;
	pLogAddTbl[164] = 2434;
	pLogAddTbl[165] = 2397;
	pLogAddTbl[166] = 2361;
	pLogAddTbl[167] = 2326;
	pLogAddTbl[168] = 2291;
	pLogAddTbl[169] = 2257;
	pLogAddTbl[170] = 2223;
	pLogAddTbl[171] = 2190;
	pLogAddTbl[172] = 2157;
	pLogAddTbl[173] = 2124;
	pLogAddTbl[174] = 2092;
	pLogAddTbl[175] = 2061;
	pLogAddTbl[176] = 2030;
	pLogAddTbl[177] = 2000;
	pLogAddTbl[178] = 1969;
	pLogAddTbl[179] = 1940;
	pLogAddTbl[180] = 1911;
	pLogAddTbl[181] = 1882;
	pLogAddTbl[182] = 1853;
	pLogAddTbl[183] = 1825;
	pLogAddTbl[184] = 1798;
	pLogAddTbl[185] = 1771;
	pLogAddTbl[186] = 1744;
	pLogAddTbl[187] = 1718;
	pLogAddTbl[188] = 1692;
	pLogAddTbl[189] = 1666;
	pLogAddTbl[190] = 1641;
	pLogAddTbl[191] = 1616;
	pLogAddTbl[192] = 1592;
	pLogAddTbl[193] = 1568;
	pLogAddTbl[194] = 1544;
	pLogAddTbl[195] = 1520;
	pLogAddTbl[196] = 1497;
	pLogAddTbl[197] = 1475;
	pLogAddTbl[198] = 1452;
	pLogAddTbl[199] = 1430;
	pLogAddTbl[200] = 1408;
	pLogAddTbl[201] = 1387;
	pLogAddTbl[202] = 1366;
	pLogAddTbl[203] = 1345;
	pLogAddTbl[204] = 1325;
	pLogAddTbl[205] = 1305;
	pLogAddTbl[206] = 1285;
	pLogAddTbl[207] = 1265;
	pLogAddTbl[208] = 1246;
	pLogAddTbl[209] = 1227;
	pLogAddTbl[210] = 1208;
	pLogAddTbl[211] = 1190;
	pLogAddTbl[212] = 1172;
	pLogAddTbl[213] = 1154;
	pLogAddTbl[214] = 1136;
	pLogAddTbl[215] = 1119;
	pLogAddTbl[216] = 1102;
	pLogAddTbl[217] = 1085;
	pLogAddTbl[218] = 1069;
	pLogAddTbl[219] = 1052;
	pLogAddTbl[220] = 1036;
	pLogAddTbl[221] = 1020;
	pLogAddTbl[222] = 1005;
	pLogAddTbl[223] = 989;
	pLogAddTbl[224] = 974;
	pLogAddTbl[225] = 959;
	pLogAddTbl[226] = 945;
	pLogAddTbl[227] = 930;
	pLogAddTbl[228] = 916;
	pLogAddTbl[229] = 902;
	pLogAddTbl[230] = 888;
	pLogAddTbl[231] = 875;
	pLogAddTbl[232] = 861;
	pLogAddTbl[233] = 848;
	pLogAddTbl[234] = 835;
	pLogAddTbl[235] = 822;
	pLogAddTbl[236] = 810;
	pLogAddTbl[237] = 797;
	pLogAddTbl[238] = 785;
	pLogAddTbl[239] = 773;
	pLogAddTbl[240] = 761;
	pLogAddTbl[241] = 750;
	pLogAddTbl[242] = 738;
	pLogAddTbl[243] = 727;
	pLogAddTbl[244] = 716;
	pLogAddTbl[245] = 705;
	pLogAddTbl[246] = 694;
	pLogAddTbl[247] = 683;
	pLogAddTbl[248] = 673;
	pLogAddTbl[249] = 662;
	pLogAddTbl[250] = 652;
	pLogAddTbl[251] = 642;
	pLogAddTbl[252] = 632;
	pLogAddTbl[253] = 623;
	pLogAddTbl[254] = 613;
	pLogAddTbl[255] = 604;
	pLogAddTbl[256] = 594;
	pLogAddTbl[257] = 585;
	pLogAddTbl[258] = 576;
	pLogAddTbl[259] = 567;
	pLogAddTbl[260] = 559;
	pLogAddTbl[261] = 550;
	pLogAddTbl[262] = 541;
	pLogAddTbl[263] = 533;
	pLogAddTbl[264] = 525;
	pLogAddTbl[265] = 517;
	pLogAddTbl[266] = 509;
	pLogAddTbl[267] = 501;
	pLogAddTbl[268] = 493;
	pLogAddTbl[269] = 486;
	pLogAddTbl[270] = 478;
	pLogAddTbl[271] = 471;
	pLogAddTbl[272] = 464;
	pLogAddTbl[273] = 456;
	pLogAddTbl[274] = 449;
	pLogAddTbl[275] = 442;
	pLogAddTbl[276] = 436;
	pLogAddTbl[277] = 429;
	pLogAddTbl[278] = 422;
	pLogAddTbl[279] = 416;
	pLogAddTbl[280] = 409;
	pLogAddTbl[281] = 403;
	pLogAddTbl[282] = 397;
	pLogAddTbl[283] = 391;
	pLogAddTbl[284] = 385;
	pLogAddTbl[285] = 379;
	pLogAddTbl[286] = 373;
	pLogAddTbl[287] = 367;
	pLogAddTbl[288] = 362;
	pLogAddTbl[289] = 356;
	pLogAddTbl[290] = 350;
	pLogAddTbl[291] = 345;
	pLogAddTbl[292] = 340;
	pLogAddTbl[293] = 334;
	pLogAddTbl[294] = 329;
	pLogAddTbl[295] = 324;
	pLogAddTbl[296] = 319;
	pLogAddTbl[297] = 314;
	pLogAddTbl[298] = 309;
	pLogAddTbl[299] = 305;
	pLogAddTbl[300] = 300;
	pLogAddTbl[301] = 295;
	pLogAddTbl[302] = 291;
	pLogAddTbl[303] = 286;
	pLogAddTbl[304] = 282;
	pLogAddTbl[305] = 277;
	pLogAddTbl[306] = 273;
	pLogAddTbl[307] = 269;
	pLogAddTbl[308] = 265;
	pLogAddTbl[309] = 261;
	pLogAddTbl[310] = 257;
	pLogAddTbl[311] = 253;
	pLogAddTbl[312] = 249;
	pLogAddTbl[313] = 245;
	pLogAddTbl[314] = 241;
	pLogAddTbl[315] = 237;
	pLogAddTbl[316] = 234;
	pLogAddTbl[317] = 230;
	pLogAddTbl[318] = 227;
	pLogAddTbl[319] = 223;
	pLogAddTbl[320] = 220;
	pLogAddTbl[321] = 216;
	pLogAddTbl[322] = 213;
	pLogAddTbl[323] = 210;
	pLogAddTbl[324] = 206;
	pLogAddTbl[325] = 203;
	pLogAddTbl[326] = 200;
	pLogAddTbl[327] = 197;
	pLogAddTbl[328] = 194;
	pLogAddTbl[329] = 191;
	pLogAddTbl[330] = 188;
	pLogAddTbl[331] = 185;
	pLogAddTbl[332] = 182;
	pLogAddTbl[333] = 179;
	pLogAddTbl[334] = 176;
	pLogAddTbl[335] = 174;
	pLogAddTbl[336] = 171;
	pLogAddTbl[337] = 168;
	pLogAddTbl[338] = 166;
	pLogAddTbl[339] = 163;
	pLogAddTbl[340] = 161;
	pLogAddTbl[341] = 158;
	pLogAddTbl[342] = 156;
	pLogAddTbl[343] = 153;
	pLogAddTbl[344] = 151;
	pLogAddTbl[345] = 149;
	pLogAddTbl[346] = 146;
	pLogAddTbl[347] = 144;
	pLogAddTbl[348] = 142;
	pLogAddTbl[349] = 140;
	pLogAddTbl[350] = 137;
	pLogAddTbl[351] = 135;
	pLogAddTbl[352] = 133;
	pLogAddTbl[353] = 131;
	pLogAddTbl[354] = 129;
	pLogAddTbl[355] = 127;
	pLogAddTbl[356] = 125;
	pLogAddTbl[357] = 123;
	pLogAddTbl[358] = 121;
	pLogAddTbl[359] = 119;
	pLogAddTbl[360] = 117;
	pLogAddTbl[361] = 116;
	pLogAddTbl[362] = 114;
	pLogAddTbl[363] = 112;
	pLogAddTbl[364] = 110;
	pLogAddTbl[365] = 109;
	pLogAddTbl[366] = 107;
	pLogAddTbl[367] = 105;
	pLogAddTbl[368] = 104;
	pLogAddTbl[369] = 102;
	pLogAddTbl[370] = 100;
	pLogAddTbl[371] = 99;
	pLogAddTbl[372] = 97;
	pLogAddTbl[373] = 96;
	pLogAddTbl[374] = 94;
	pLogAddTbl[375] = 93;
	pLogAddTbl[376] = 91;
	pLogAddTbl[377] = 90;
	pLogAddTbl[378] = 89;
	pLogAddTbl[379] = 87;
	pLogAddTbl[380] = 86;
	pLogAddTbl[381] = 85;
	pLogAddTbl[382] = 83;
	pLogAddTbl[383] = 82;
	pLogAddTbl[384] = 81;
	pLogAddTbl[385] = 79;
	pLogAddTbl[386] = 78;
	pLogAddTbl[387] = 77;
	pLogAddTbl[388] = 76;
	pLogAddTbl[389] = 75;
	pLogAddTbl[390] = 73;
	pLogAddTbl[391] = 72;
	pLogAddTbl[392] = 71;
	pLogAddTbl[393] = 70;
	pLogAddTbl[394] = 69;
	pLogAddTbl[395] = 68;
	pLogAddTbl[396] = 67;
	pLogAddTbl[397] = 66;
	pLogAddTbl[398] = 65;
	pLogAddTbl[399] = 64;
	pLogAddTbl[400] = 63;
	pLogAddTbl[401] = 62;
	pLogAddTbl[402] = 61;
	pLogAddTbl[403] = 60;
	pLogAddTbl[404] = 59;
	pLogAddTbl[405] = 58;
	pLogAddTbl[406] = 57;
	pLogAddTbl[407] = 56;
	pLogAddTbl[408] = 55;
	pLogAddTbl[409] = 54;
	pLogAddTbl[410] = 54;
	pLogAddTbl[411] = 53;
	pLogAddTbl[412] = 52;
	pLogAddTbl[413] = 51;
	pLogAddTbl[414] = 50;
	pLogAddTbl[415] = 50;
	pLogAddTbl[416] = 49;
	pLogAddTbl[417] = 48;
	pLogAddTbl[418] = 47;
	pLogAddTbl[419] = 46;
	pLogAddTbl[420] = 46;
	pLogAddTbl[421] = 45;
	pLogAddTbl[422] = 44;
	pLogAddTbl[423] = 44;
	pLogAddTbl[424] = 43;
	pLogAddTbl[425] = 42;
	pLogAddTbl[426] = 42;
	pLogAddTbl[427] = 41;
	pLogAddTbl[428] = 40;
	pLogAddTbl[429] = 40;
	pLogAddTbl[430] = 39;
	pLogAddTbl[431] = 38;
	pLogAddTbl[432] = 38;
	pLogAddTbl[433] = 37;
	pLogAddTbl[434] = 37;
	pLogAddTbl[435] = 36;
	pLogAddTbl[436] = 36;
	pLogAddTbl[437] = 35;
	pLogAddTbl[438] = 34;
	pLogAddTbl[439] = 34;
	pLogAddTbl[440] = 33;
	pLogAddTbl[441] = 33;
	pLogAddTbl[442] = 32;
	pLogAddTbl[443] = 32;
	pLogAddTbl[444] = 31;
	pLogAddTbl[445] = 31;
	pLogAddTbl[446] = 30;
	pLogAddTbl[447] = 30;
	pLogAddTbl[448] = 29;
	pLogAddTbl[449] = 29;
	pLogAddTbl[450] = 28;
	pLogAddTbl[451] = 28;
	pLogAddTbl[452] = 28;
	pLogAddTbl[453] = 27;
	pLogAddTbl[454] = 27;
	pLogAddTbl[455] = 26;
	pLogAddTbl[456] = 26;
	pLogAddTbl[457] = 25;
	pLogAddTbl[458] = 25;
	pLogAddTbl[459] = 25;
	pLogAddTbl[460] = 24;
	pLogAddTbl[461] = 24;
	pLogAddTbl[462] = 24;
	pLogAddTbl[463] = 23;
	pLogAddTbl[464] = 23;
	pLogAddTbl[465] = 22;
	pLogAddTbl[466] = 22;
	pLogAddTbl[467] = 22;
	pLogAddTbl[468] = 21;
	pLogAddTbl[469] = 21;
	pLogAddTbl[470] = 21;
	pLogAddTbl[471] = 20;
	pLogAddTbl[472] = 20;
	pLogAddTbl[473] = 20;
	pLogAddTbl[474] = 19;
	pLogAddTbl[475] = 19;
	pLogAddTbl[476] = 19;
	pLogAddTbl[477] = 18;
	pLogAddTbl[478] = 18;
	pLogAddTbl[479] = 18;
	pLogAddTbl[480] = 18;
	pLogAddTbl[481] = 17;
	pLogAddTbl[482] = 17;
	pLogAddTbl[483] = 17;
	pLogAddTbl[484] = 17;
	pLogAddTbl[485] = 16;
	pLogAddTbl[486] = 16;
	pLogAddTbl[487] = 16;
	pLogAddTbl[488] = 15;
	pLogAddTbl[489] = 15;
	pLogAddTbl[490] = 15;
	pLogAddTbl[491] = 15;
	pLogAddTbl[492] = 15;
	pLogAddTbl[493] = 14;
	pLogAddTbl[494] = 14;
	pLogAddTbl[495] = 14;
	pLogAddTbl[496] = 14;
	pLogAddTbl[497] = 13;
	pLogAddTbl[498] = 13;
	pLogAddTbl[499] = 13;
	pLogAddTbl[500] = 13;
	pLogAddTbl[501] = 13;
	pLogAddTbl[502] = 12;
	pLogAddTbl[503] = 12;
	pLogAddTbl[504] = 12;
	pLogAddTbl[505] = 12;
	pLogAddTbl[506] = 12;
	pLogAddTbl[507] = 11;
	pLogAddTbl[508] = 11;
	pLogAddTbl[509] = 11;
	pLogAddTbl[510] = 11;
	pLogAddTbl[511] = 11;
	pLogAddTbl[512] = 10;

#else	//!FIXED_POINT_FE

	hci_int32 j = 0;
	double  init_value, fvalue, step;
	double  constant, base, ftmp;

	init_value = 0.0;
	step = 1/LOG_PROD;
	base = 1.0001;
	constant = log(base);

    for(j=0,fvalue=init_value;j<MAX_LOG_TABLE;j++,fvalue-=step) {
        ftmp = pow(base,fvalue/constant);
        pLogAddTbl[j] = (float)log(1.0+ftmp);
		//pLogAddTbl[j] = (int)(pLogAddTbl[j]*32768.0f);
    }
    pLogAddTbl[j] = 0.0;

#endif	// #ifdef FIXED_POINT_FE

}

/**
 *	create log-addition table to compute state LLs
 */
HCILAB_PRIVATE void 
_createLogAdditionTableForStateLL(hci_int16 *pLogAddTbl)
{

//#ifdef FIXED_POINT_BE

	pLogAddTbl[0] = 69;
	pLogAddTbl[1] = 69;
	pLogAddTbl[2] = 68;
	pLogAddTbl[3] = 68;
	pLogAddTbl[4] = 67;
	pLogAddTbl[5] = 67;
	pLogAddTbl[6] = 66;
	pLogAddTbl[7] = 66;
	pLogAddTbl[8] = 65;
	pLogAddTbl[9] = 65;
	pLogAddTbl[10] = 64;
	pLogAddTbl[11] = 64;
	pLogAddTbl[12] = 63;
	pLogAddTbl[13] = 63;
	pLogAddTbl[14] = 63;
	pLogAddTbl[15] = 62;
	pLogAddTbl[16] = 62;
	pLogAddTbl[17] = 61;
	pLogAddTbl[18] = 61;
	pLogAddTbl[19] = 60;
	pLogAddTbl[20] = 60;
	pLogAddTbl[21] = 59;
	pLogAddTbl[22] = 59;
	pLogAddTbl[23] = 58;
	pLogAddTbl[24] = 58;
	pLogAddTbl[25] = 58;
	pLogAddTbl[26] = 57;
	pLogAddTbl[27] = 57;
	pLogAddTbl[28] = 56;
	pLogAddTbl[29] = 56;
	pLogAddTbl[30] = 55;
	pLogAddTbl[31] = 55;
	pLogAddTbl[32] = 55;
	pLogAddTbl[33] = 54;
	pLogAddTbl[34] = 54;
	pLogAddTbl[35] = 53;
	pLogAddTbl[36] = 53;
	pLogAddTbl[37] = 53;
	pLogAddTbl[38] = 52;
	pLogAddTbl[39] = 52;
	pLogAddTbl[40] = 51;
	pLogAddTbl[41] = 51;
	pLogAddTbl[42] = 51;
	pLogAddTbl[43] = 50;
	pLogAddTbl[44] = 50;
	pLogAddTbl[45] = 49;
	pLogAddTbl[46] = 49;
	pLogAddTbl[47] = 49;
	pLogAddTbl[48] = 48;
	pLogAddTbl[49] = 48;
	pLogAddTbl[50] = 47;
	pLogAddTbl[51] = 47;
	pLogAddTbl[52] = 47;
	pLogAddTbl[53] = 46;
	pLogAddTbl[54] = 46;
	pLogAddTbl[55] = 46;
	pLogAddTbl[56] = 45;
	pLogAddTbl[57] = 45;
	pLogAddTbl[58] = 44;
	pLogAddTbl[59] = 44;
	pLogAddTbl[60] = 44;
	pLogAddTbl[61] = 43;
	pLogAddTbl[62] = 43;
	pLogAddTbl[63] = 43;
	pLogAddTbl[64] = 42;
	pLogAddTbl[65] = 42;
	pLogAddTbl[66] = 42;
	pLogAddTbl[67] = 41;
	pLogAddTbl[68] = 41;
	pLogAddTbl[69] = 41;
	pLogAddTbl[70] = 40;
	pLogAddTbl[71] = 40;
	pLogAddTbl[72] = 40;
	pLogAddTbl[73] = 39;
	pLogAddTbl[74] = 39;
	pLogAddTbl[75] = 39;
	pLogAddTbl[76] = 38;
	pLogAddTbl[77] = 38;
	pLogAddTbl[78] = 38;
	pLogAddTbl[79] = 37;
	pLogAddTbl[80] = 37;
	pLogAddTbl[81] = 37;
	pLogAddTbl[82] = 36;
	pLogAddTbl[83] = 36;
	pLogAddTbl[84] = 36;
	pLogAddTbl[85] = 36;
	pLogAddTbl[86] = 35;
	pLogAddTbl[87] = 35;
	pLogAddTbl[88] = 35;
	pLogAddTbl[89] = 34;
	pLogAddTbl[90] = 34;
	pLogAddTbl[91] = 34;
	pLogAddTbl[92] = 34;
	pLogAddTbl[93] = 33;
	pLogAddTbl[94] = 33;
	pLogAddTbl[95] = 33;
	pLogAddTbl[96] = 32;
	pLogAddTbl[97] = 32;
	pLogAddTbl[98] = 32;
	pLogAddTbl[99] = 32;
	pLogAddTbl[100] = 31;
	pLogAddTbl[101] = 31;
	pLogAddTbl[102] = 31;
	pLogAddTbl[103] = 31;
	pLogAddTbl[104] = 30;
	pLogAddTbl[105] = 30;
	pLogAddTbl[106] = 30;
	pLogAddTbl[107] = 29;
	pLogAddTbl[108] = 29;
	pLogAddTbl[109] = 29;
	pLogAddTbl[110] = 29;
	pLogAddTbl[111] = 28;
	pLogAddTbl[112] = 28;
	pLogAddTbl[113] = 28;
	pLogAddTbl[114] = 28;
	pLogAddTbl[115] = 28;
	pLogAddTbl[116] = 27;
	pLogAddTbl[117] = 27;
	pLogAddTbl[118] = 27;
	pLogAddTbl[119] = 27;
	pLogAddTbl[120] = 26;
	pLogAddTbl[121] = 26;
	pLogAddTbl[122] = 26;
	pLogAddTbl[123] = 26;
	pLogAddTbl[124] = 25;
	pLogAddTbl[125] = 25;
	pLogAddTbl[126] = 25;
	pLogAddTbl[127] = 25;
	pLogAddTbl[128] = 25;
	pLogAddTbl[129] = 24;
	pLogAddTbl[130] = 24;
	pLogAddTbl[131] = 24;
	pLogAddTbl[132] = 24;
	pLogAddTbl[133] = 23;
	pLogAddTbl[134] = 23;
	pLogAddTbl[135] = 23;
	pLogAddTbl[136] = 23;
	pLogAddTbl[137] = 23;
	pLogAddTbl[138] = 22;
	pLogAddTbl[139] = 22;
	pLogAddTbl[140] = 22;
	pLogAddTbl[141] = 22;
	pLogAddTbl[142] = 22;
	pLogAddTbl[143] = 21;
	pLogAddTbl[144] = 21;
	pLogAddTbl[145] = 21;
	pLogAddTbl[146] = 21;
	pLogAddTbl[147] = 21;
	pLogAddTbl[148] = 21;
	pLogAddTbl[149] = 20;
	pLogAddTbl[150] = 20;
	pLogAddTbl[151] = 20;
	pLogAddTbl[152] = 20;
	pLogAddTbl[153] = 20;
	pLogAddTbl[154] = 19;
	pLogAddTbl[155] = 19;
	pLogAddTbl[156] = 19;
	pLogAddTbl[157] = 19;
	pLogAddTbl[158] = 19;
	pLogAddTbl[159] = 19;
	pLogAddTbl[160] = 18;
	pLogAddTbl[161] = 18;
	pLogAddTbl[162] = 18;
	pLogAddTbl[163] = 18;
	pLogAddTbl[164] = 18;
	pLogAddTbl[165] = 18;
	pLogAddTbl[166] = 17;
	pLogAddTbl[167] = 17;
	pLogAddTbl[168] = 17;
	pLogAddTbl[169] = 17;
	pLogAddTbl[170] = 17;
	pLogAddTbl[171] = 17;
	pLogAddTbl[172] = 16;
	pLogAddTbl[173] = 16;
	pLogAddTbl[174] = 16;
	pLogAddTbl[175] = 16;
	pLogAddTbl[176] = 16;
	pLogAddTbl[177] = 16;
	pLogAddTbl[178] = 16;
	pLogAddTbl[179] = 15;
	pLogAddTbl[180] = 15;
	pLogAddTbl[181] = 15;
	pLogAddTbl[182] = 15;
	pLogAddTbl[183] = 15;
	pLogAddTbl[184] = 15;
	pLogAddTbl[185] = 15;
	pLogAddTbl[186] = 14;
	pLogAddTbl[187] = 14;
	pLogAddTbl[188] = 14;
	pLogAddTbl[189] = 14;
	pLogAddTbl[190] = 14;
	pLogAddTbl[191] = 14;
	pLogAddTbl[192] = 14;
	pLogAddTbl[193] = 14;
	pLogAddTbl[194] = 13;
	pLogAddTbl[195] = 13;
	pLogAddTbl[196] = 13;
	pLogAddTbl[197] = 13;
	pLogAddTbl[198] = 13;
	pLogAddTbl[199] = 13;
	pLogAddTbl[200] = 13;
	pLogAddTbl[201] = 13;
	pLogAddTbl[202] = 12;
	pLogAddTbl[203] = 12;
	pLogAddTbl[204] = 12;
	pLogAddTbl[205] = 12;
	pLogAddTbl[206] = 12;
	pLogAddTbl[207] = 12;
	pLogAddTbl[208] = 12;
	pLogAddTbl[209] = 12;
	pLogAddTbl[210] = 12;
	pLogAddTbl[211] = 11;
	pLogAddTbl[212] = 11;
	pLogAddTbl[213] = 11;
	pLogAddTbl[214] = 11;
	pLogAddTbl[215] = 11;
	pLogAddTbl[216] = 11;
	pLogAddTbl[217] = 11;
	pLogAddTbl[218] = 11;
	pLogAddTbl[219] = 11;
	pLogAddTbl[220] = 11;
	pLogAddTbl[221] = 10;
	pLogAddTbl[222] = 10;
	pLogAddTbl[223] = 10;
	pLogAddTbl[224] = 10;
	pLogAddTbl[225] = 10;
	pLogAddTbl[226] = 10;
	pLogAddTbl[227] = 10;
	pLogAddTbl[228] = 10;
	pLogAddTbl[229] = 10;
	pLogAddTbl[230] = 10;
	pLogAddTbl[231] = 9;
	pLogAddTbl[232] = 9;
	pLogAddTbl[233] = 9;
	pLogAddTbl[234] = 9;
	pLogAddTbl[235] = 9;
	pLogAddTbl[236] = 9;
	pLogAddTbl[237] = 9;
	pLogAddTbl[238] = 9;
	pLogAddTbl[239] = 9;
	pLogAddTbl[240] = 9;
	pLogAddTbl[241] = 9;
	pLogAddTbl[242] = 9;
	pLogAddTbl[243] = 8;
	pLogAddTbl[244] = 8;
	pLogAddTbl[245] = 8;
	pLogAddTbl[246] = 8;
	pLogAddTbl[247] = 8;
	pLogAddTbl[248] = 8;
	pLogAddTbl[249] = 8;
	pLogAddTbl[250] = 8;
	pLogAddTbl[251] = 8;
	pLogAddTbl[252] = 8;
	pLogAddTbl[253] = 8;
	pLogAddTbl[254] = 8;
	pLogAddTbl[255] = 8;
	pLogAddTbl[256] = 7;
	pLogAddTbl[257] = 7;
	pLogAddTbl[258] = 7;
	pLogAddTbl[259] = 7;
	pLogAddTbl[260] = 7;
	pLogAddTbl[261] = 7;
	pLogAddTbl[262] = 7;
	pLogAddTbl[263] = 7;
	pLogAddTbl[264] = 7;
	pLogAddTbl[265] = 7;
	pLogAddTbl[266] = 7;
	pLogAddTbl[267] = 7;
	pLogAddTbl[268] = 7;
	pLogAddTbl[269] = 7;
	pLogAddTbl[270] = 7;
	pLogAddTbl[271] = 6;
	pLogAddTbl[272] = 6;
	pLogAddTbl[273] = 6;
	pLogAddTbl[274] = 6;
	pLogAddTbl[275] = 6;
	pLogAddTbl[276] = 6;
	pLogAddTbl[277] = 6;
	pLogAddTbl[278] = 6;
	pLogAddTbl[279] = 6;
	pLogAddTbl[280] = 6;
	pLogAddTbl[281] = 6;
	pLogAddTbl[282] = 6;
	pLogAddTbl[283] = 6;
	pLogAddTbl[284] = 6;
	pLogAddTbl[285] = 6;
	pLogAddTbl[286] = 6;
	pLogAddTbl[287] = 6;
	pLogAddTbl[288] = 5;
	pLogAddTbl[289] = 5;
	pLogAddTbl[290] = 5;
	pLogAddTbl[291] = 5;
	pLogAddTbl[292] = 5;
	pLogAddTbl[293] = 5;
	pLogAddTbl[294] = 5;
	pLogAddTbl[295] = 5;
	pLogAddTbl[296] = 5;
	pLogAddTbl[297] = 5;
	pLogAddTbl[298] = 5;
	pLogAddTbl[299] = 5;
	pLogAddTbl[300] = 5;
	pLogAddTbl[301] = 5;
	pLogAddTbl[302] = 5;
	pLogAddTbl[303] = 5;
	pLogAddTbl[304] = 5;
	pLogAddTbl[305] = 5;
	pLogAddTbl[306] = 5;
	pLogAddTbl[307] = 5;
	pLogAddTbl[308] = 4;
	pLogAddTbl[309] = 4;
	pLogAddTbl[310] = 4;
	pLogAddTbl[311] = 4;
	pLogAddTbl[312] = 4;
	pLogAddTbl[313] = 4;
	pLogAddTbl[314] = 4;
	pLogAddTbl[315] = 4;
	pLogAddTbl[316] = 4;
	pLogAddTbl[317] = 4;
	pLogAddTbl[318] = 4;
	pLogAddTbl[319] = 4;
	pLogAddTbl[320] = 4;
	pLogAddTbl[321] = 4;
	pLogAddTbl[322] = 4;
	pLogAddTbl[323] = 4;
	pLogAddTbl[324] = 4;
	pLogAddTbl[325] = 4;
	pLogAddTbl[326] = 4;
	pLogAddTbl[327] = 4;
	pLogAddTbl[328] = 4;
	pLogAddTbl[329] = 4;
	pLogAddTbl[330] = 4;
	pLogAddTbl[331] = 4;
	pLogAddTbl[332] = 4;
	pLogAddTbl[333] = 4;
	pLogAddTbl[334] = 3;
	pLogAddTbl[335] = 3;
	pLogAddTbl[336] = 3;
	pLogAddTbl[337] = 3;
	pLogAddTbl[338] = 3;
	pLogAddTbl[339] = 3;
	pLogAddTbl[340] = 3;
	pLogAddTbl[341] = 3;
	pLogAddTbl[342] = 3;
	pLogAddTbl[343] = 3;
	pLogAddTbl[344] = 3;
	pLogAddTbl[345] = 3;
	pLogAddTbl[346] = 3;
	pLogAddTbl[347] = 3;
	pLogAddTbl[348] = 3;
	pLogAddTbl[349] = 3;
	pLogAddTbl[350] = 3;
	pLogAddTbl[351] = 3;
	pLogAddTbl[352] = 3;
	pLogAddTbl[353] = 3;
	pLogAddTbl[354] = 3;
	pLogAddTbl[355] = 3;
	pLogAddTbl[356] = 3;
	pLogAddTbl[357] = 3;
	pLogAddTbl[358] = 3;
	pLogAddTbl[359] = 3;
	pLogAddTbl[360] = 3;
	pLogAddTbl[361] = 3;
	pLogAddTbl[362] = 3;
	pLogAddTbl[363] = 3;
	pLogAddTbl[364] = 3;
	pLogAddTbl[365] = 3;
	pLogAddTbl[366] = 3;
	pLogAddTbl[367] = 3;
	pLogAddTbl[368] = 2;
	pLogAddTbl[369] = 2;
	pLogAddTbl[370] = 2;
	pLogAddTbl[371] = 2;
	pLogAddTbl[372] = 2;
	pLogAddTbl[373] = 2;
	pLogAddTbl[374] = 2;
	pLogAddTbl[375] = 2;
	pLogAddTbl[376] = 2;
	pLogAddTbl[377] = 2;
	pLogAddTbl[378] = 2;
	pLogAddTbl[379] = 2;
	pLogAddTbl[380] = 2;
	pLogAddTbl[381] = 2;
	pLogAddTbl[382] = 2;
	pLogAddTbl[383] = 2;
	pLogAddTbl[384] = 2;
	pLogAddTbl[385] = 2;
	pLogAddTbl[386] = 2;
	pLogAddTbl[387] = 2;
	pLogAddTbl[388] = 2;
	pLogAddTbl[389] = 2;
	pLogAddTbl[390] = 2;
	pLogAddTbl[391] = 2;
	pLogAddTbl[392] = 2;
	pLogAddTbl[393] = 2;
	pLogAddTbl[394] = 2;
	pLogAddTbl[395] = 2;
	pLogAddTbl[396] = 2;
	pLogAddTbl[397] = 2;
	pLogAddTbl[398] = 2;
	pLogAddTbl[399] = 2;
	pLogAddTbl[400] = 2;
	pLogAddTbl[401] = 2;
	pLogAddTbl[402] = 2;
	pLogAddTbl[403] = 2;
	pLogAddTbl[404] = 2;
	pLogAddTbl[405] = 2;
	pLogAddTbl[406] = 2;
	pLogAddTbl[407] = 2;
	pLogAddTbl[408] = 2;
	pLogAddTbl[409] = 2;
	pLogAddTbl[410] = 2;
	pLogAddTbl[411] = 2;
	pLogAddTbl[412] = 2;
	pLogAddTbl[413] = 2;
	pLogAddTbl[414] = 2;
	pLogAddTbl[415] = 2;
	pLogAddTbl[416] = 2;
	pLogAddTbl[417] = 2;
	pLogAddTbl[418] = 2;
	pLogAddTbl[419] = 2;
	pLogAddTbl[420] = 1;
	pLogAddTbl[421] = 1;
	pLogAddTbl[422] = 1;
	pLogAddTbl[423] = 1;
	pLogAddTbl[424] = 1;
	pLogAddTbl[425] = 1;
	pLogAddTbl[426] = 1;
	pLogAddTbl[427] = 1;
	pLogAddTbl[428] = 1;
	pLogAddTbl[429] = 1;
	pLogAddTbl[430] = 1;
	pLogAddTbl[431] = 1;
	pLogAddTbl[432] = 1;
	pLogAddTbl[433] = 1;
	pLogAddTbl[434] = 1;
	pLogAddTbl[435] = 1;
	pLogAddTbl[436] = 1;
	pLogAddTbl[437] = 1;
	pLogAddTbl[438] = 1;
	pLogAddTbl[439] = 1;
	pLogAddTbl[440] = 1;
	pLogAddTbl[441] = 1;
	pLogAddTbl[442] = 1;
	pLogAddTbl[443] = 1;
	pLogAddTbl[444] = 1;
	pLogAddTbl[445] = 1;
	pLogAddTbl[446] = 1;
	pLogAddTbl[447] = 1;
	pLogAddTbl[448] = 1;
	pLogAddTbl[449] = 1;
	pLogAddTbl[450] = 1;
	pLogAddTbl[451] = 1;
	pLogAddTbl[452] = 1;
	pLogAddTbl[453] = 1;
	pLogAddTbl[454] = 1;
	pLogAddTbl[455] = 1;
	pLogAddTbl[456] = 1;
	pLogAddTbl[457] = 1;
	pLogAddTbl[458] = 1;
	pLogAddTbl[459] = 1;
	pLogAddTbl[460] = 1;
	pLogAddTbl[461] = 1;
	pLogAddTbl[462] = 1;
	pLogAddTbl[463] = 1;
	pLogAddTbl[464] = 1;
	pLogAddTbl[465] = 1;
	pLogAddTbl[466] = 1;
	pLogAddTbl[467] = 1;
	pLogAddTbl[468] = 1;
	pLogAddTbl[469] = 1;
	pLogAddTbl[470] = 1;
	pLogAddTbl[471] = 1;
	pLogAddTbl[472] = 1;
	pLogAddTbl[473] = 1;
	pLogAddTbl[474] = 1;
	pLogAddTbl[475] = 1;
	pLogAddTbl[476] = 1;
	pLogAddTbl[477] = 1;
	pLogAddTbl[478] = 1;
	pLogAddTbl[479] = 1;
	pLogAddTbl[480] = 1;
	pLogAddTbl[481] = 1;
	pLogAddTbl[482] = 1;
	pLogAddTbl[483] = 1;
	pLogAddTbl[484] = 1;
	pLogAddTbl[485] = 1;
	pLogAddTbl[486] = 1;
	pLogAddTbl[487] = 1;
	pLogAddTbl[488] = 1;
	pLogAddTbl[489] = 1;
	pLogAddTbl[490] = 1;
	pLogAddTbl[491] = 1;
	pLogAddTbl[492] = 1;
	pLogAddTbl[493] = 1;
	pLogAddTbl[494] = 1;
	pLogAddTbl[495] = 1;
	pLogAddTbl[496] = 1;
	pLogAddTbl[497] = 1;
	pLogAddTbl[498] = 1;
	pLogAddTbl[499] = 1;
	pLogAddTbl[500] = 1;
	pLogAddTbl[501] = 1;
	pLogAddTbl[502] = 1;
	pLogAddTbl[503] = 1;
	pLogAddTbl[504] = 1;
	pLogAddTbl[505] = 1;
	pLogAddTbl[506] = 1;
	pLogAddTbl[507] = 1;
	pLogAddTbl[508] = 1;
	pLogAddTbl[509] = 1;
	pLogAddTbl[510] = 1;
	pLogAddTbl[511] = 1;
	pLogAddTbl[512] = 0;

// #else	//!FIXED_POINT_BE
// 
// 	hci_int32 j = 0;
// 	double  init_value, fvalue, step;
// 	double  constant, base, ftmp;
// 
// 	init_value = 0.0;
// 	step = 0.01;
// 	base = 1.0001;
// 	constant = log(base);
// 
//     for(j=0,fvalue=init_value;j<MAX_LOG_TABLE;j++,fvalue-=step) {
//         ftmp = pow(base,fvalue/constant);
//         pLogAddTbl[j] = (hci_int16)(log(1.0+ftmp)*100.0f + 0.5f);
//     }
//     pLogAddTbl[j] = 0;
// 
// #endif	// #ifdef FIXED_POINT_BE

}

// end of file