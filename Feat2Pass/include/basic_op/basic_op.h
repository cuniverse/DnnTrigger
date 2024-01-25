
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
 *	@file	basic_op.h
 *	@ingroup basic_op_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	APIs for basic fixed-point arithmetic operators
 */

#ifndef __HCILAB_BASIC_OP_H__
#define __HCILAB_BASIC_OP_H__

#include "base/hci_type.h"
#include "basic_op/hci_logadd.h"

#if defined(HCI_MSC_32)
#ifdef HCI_BASICOP_EXPORTS
#define HCI_BASICOP_API __declspec(dllexport)
#elif defined(HCI_BASICOP_IMPORTS)
#define HCI_BASICOP_API __declspec(dllimport)
#else	// in case of static library
#define HCI_BASICOP_API
#endif // #ifdef HCI_BASICOP_EXPORTS
#elif defined(HCI_OS2)
#define HCI_BASICOP_API
#else
#define HCI_BASICOP_API HCI_USER
#endif

/** PowerASR Basic Operator */
typedef struct _PowerASR_BasicOP {
	void *pInner;
} PowerASR_BasicOP;

#ifdef __cplusplus
extern "C" {
#endif

/**
 *	create a new basic operator
 *
 *	@return pointer to a newly created basic operator
 */
HCILAB_PUBLIC HCI_BASICOP_API PowerASR_BasicOP*
PowerASR_BasicOP_new(
);

/**
 *	delete delete the basic operator.
 */
HCILAB_PUBLIC HCI_BASICOP_API void
PowerASR_BasicOP_delete(PowerASR_BasicOP *pThis		///< (i/o) pointer to the basic operator
);

/**
 *	return the pointer of log-addition table.
 *	if pointer is null, make a new log-addition table.
 *
 *	@return pointer of log-addition table
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_logadd_t*
PowerASR_BasicOP_getLogAdditionTable(PowerASR_BasicOP *pThis
);

/**
 *	return the pointer of log-addition table to compute state log-likelihoods (scaled by 100).
 *	if pointer is null, make a new log-addition table.
 *
 *	@return pointer of log-addition table for state LL.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int16*
PowerASR_BasicOP_getLogAdditionTableForStateLL(PowerASR_BasicOP *pThis
);

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
						   hci_int16 var2		///< 16 bit short signed integer
);

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
								hci_int16 var2		///< 16 bit short signed integer
);

/**
 *	Absolute value of var1.
 *	abs_16_16(-32768) = 32767.
 *
 *  Complexity weight : 1
 *
 *	@return Return 16-bit short signed integer output value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int16
PowerASR_BasicOP_abs_16_16(hci_int16 var1		///< 16 bit short signed integer
);

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
								 hci_int16 var2		///< 16 bit short signed integer
);

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
								  hci_int16 var2		///< 16 bit short signed integer
);

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
								hci_int16 var2		///< 16 bit short signed integer
);

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
								hci_int16 var2		///< 16 bit short signed integer
);

/**
 *	Negate var1 with saturation, saturate in the case where input is -32768.
 *		- negate_16_16(var1) = subtract_16_16(0,var1).
 *
 *  Complexity weight : 1
 *
 *	@return Return 16-bit short signed integer output value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int16
PowerASR_BasicOP_negate_16_16(hci_int16 var1		///< 16 bit short signed integer
);

/**
 *	Return the 16 MSB of L_var1.
 *
 *  Complexity weight : 1
 *
 *	@return Return 16-bit short signed integer output value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int16
PowerASR_BasicOP_extractMSB_32_16(hci_int32 L_var1		///< 32 bit long signed integer
);

/**
 *	Return the 16 LSB of L_var1.
 *
 *  Complexity weight : 1
 *
 *	@return Return 16-bit short signed integer output value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int16
PowerASR_BasicOP_extractLSB_32_16(hci_int32 L_var1		///< 32 bit long signed integer
);

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
PowerASR_BasicOP_round_32_16(hci_int32 L_var1		///< 32 bit long signed integer
);

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
										   hci_int16 var2		///< 16 bit short signed integer
);

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
												hci_int16 var2		///< 16 bit short signed integer
);

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
													  hci_int16 var2		///< 16 bit short signed integer
);

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
														   hci_int16 var2		///< 16 bit short signed integer
);

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
						   hci_int32 L_var2		///< 32 bit long signed integer
);

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
								hci_int32 L_var2	///< 32 bit long signed integer
);

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
									hci_int32 L_var2	///< 32 bit long signed integer
);

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
										 hci_int32 L_var2	///< 32 bit long signed integer
);

/**
 *	Negate the 32 bit variable L_var1 with saturation;
 *	 - saturate in the case where input is -2147483648 (0x8000 0000).
 *
 *  Complexity weight : 2
 *
 *	@return Return 32-bit long signed integer output value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int32
PowerASR_BasicOP_negate_32_32(hci_int32 L_var1		///< 32 bit long signed integer
);

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
										 hci_int16 var2		///< 16 bit short signed integer
);

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
								 hci_int16 var2			///< 16 bit short signed integer
);

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
								  hci_int16 var2		///< 16 bit short signed integer
);

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
										   hci_int16 var2		///< 16 bit short signed integer
);

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
													hci_int16 var2		///< 16 bit short signed integer
);

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
														 hci_int16 var2		///< 16 bit short signed integer
);

/**
 *	Deposit the 16 bit var1 into the 16 MS bits of the 32 bit output.
 *	The 16 LS bits of the output are zeroed.
 *
 *  Complexity weight : 2
 *
 *	@return Return 32-bit long signed integer output value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int32
PowerASR_BasicOP_depositMSB_16_32(hci_int16 var1		///< 16 bit short signed integer
);

/**
 *	Deposit the 16 bit var1 into the 16 LS bits of the 32 bit output.
 *	The 16 MS bits of the output are sign extended.
 *
 *  Complexity weight : 2
 *
 *	@return Return 32-bit long signed integer output value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int32
PowerASR_BasicOP_depositLSB_16_32(hci_int16 var1		///< 16 bit short signed integer
);

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
										   hci_int16 var2		///< 16 bit short signed integer
);

/**
 *	Absolute value of L_var1.
 *	Saturate in case where the input is -214783648.
 *
 *  Complexity weight : 3
 *
 *	@return Return 32-bit long signed integer output value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int32
PowerASR_BasicOP_abs_32_32(hci_int32 L_var1		///< 32 bit long signed integer
);

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
PowerASR_BasicOP_saturate_32_32(hci_int32 L_var1	///< 32 bit long signed integer
);

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
PowerASR_BasicOP_getNormalShiftCount_16_16(hci_int16 var1		///< 16 bit short signed integer
);

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
												   hci_int16 heads			///< head value
);

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
							  hci_int16 var2		///< 16 bit short signed integer
);

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
PowerASR_BasicOP_getNormalShiftCount_32_16(hci_int32 L_var1		///< 32 bit long signed integer
);

/**
 *	separate a 32 bit long integer into two 16 bit DPF.
 *
 *  Complexity weight : 4
 */
HCILAB_PUBLIC HCI_BASICOP_API void
PowerASR_BasicOP_separateBits_32_16(hci_int32 L_var,		///< (i) 32 bit long signed integer
									hci_int16 *hi_var,		///< (o) 16 bit MSB of 32 bit long singed integer
									hci_int16 *lo_var		///< (o) 16 bit LSB of 32 bit long singed integer
);

/**
 *	Produce a 32 bit long signed integer from two 16 bit DPFs.
 *
 *  Complexity weight : 2
 *
 *	@return Return 32-bit long signed integer output value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int32
PowerASR_BasicOP_composeBits_16_32(hci_int16 var_hi,	///< (i) 16 bit MSB short signed integer
								   hci_int16 var_lo		///< (i) 16 bit LSB short signed integer
);

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
									 hci_int16 var_lo2		///< 16 bit LSB part of second long signed integer
);

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
								   hci_int16 var2		///< second 16 bit short signed integer
);

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
							  hci_int16 var_denom_lo	///< 16 bit LSB part of long signed integer (denominator)
);

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
										  hci_int16 var3	///< 16 bit short signed integer
);

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
									   hci_int16 var3		///< 16 bit short signed integer
);

/**
 *	CORDIC-based Q14/16bit sin/cos value.
 *
 *  Complexity weight : ?
 *
 *	@return Return 16-bit short signed integer output value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int16
PowerASR_BasicOP_fixedCosine_32_16(hci_int32 L_var1,	///< sin(or cos) argument(Q15/32bit). (must be not radian but degree)
								   hci_int16 bCosSin	///< sin/cos flag bit (cosine bCosSin = 0, sine bCosSin = 1)
);

/**
 *	Returns the 16 bit result of the square-root of L_var1.
 *
 *  Complexity weight : ?
 *
 *	@return Return 16-bit short signed integer output value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int16
PowerASR_BasicOP_fixedSQRT_32_16(hci_int32 L_var1	///< 32 bit long signed integer
);

/**
 *	Returns the 32 bit result of the square-root of L_var1.
 *
 *  Complexity weight : ?
 *
 *	@return Return 32-bit long signed integer output value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int32
PowerASR_BasicOP_fixedSQRT_32_32(hci_int32 L_var1	///< 32 bit long signed integer
);

/**
 *	Returns the 32 bit result (Q15.32) of the log value of L_var1.
 *
 *  Complexity weight : ?
 *
 *	@return Return 32-bit long signed integer output value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int32
PowerASR_BasicOP_fixedLOG_32_32(hci_int32 L_var1,		///< 32 bit long signed integer
								hci_int16 var_qform		///< 16 bit short signed integer
);

/**
 *	Approximation of log in base 2 i.e. output = 1024 * ln(|x|) / ln(2)
 *
 *  Complexity weight : ?
 *
 *	@return Return 16-bit short signed integer output value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int16
PowerASR_BasicOP_fixedLOG_2(hci_int32 L_Input		///< 32 bit long signed integer
);

/**
 *	take the log base 2 of input and divide by 32 and return;
 *	i.e. output = log2(input)/32
 *
 *  Complexity weight : ?
 *
 *	@return Return 32-bit long signed integer output value.
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_int16
PowerASR_BasicOP_fixedLOG2_32_16(hci_int32 L_Input		///< 32 bit long signed integer
);

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
							  hci_logadd_t *tableLAdd	///< log-addition table
);

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
								  const hci_int16 *tableLAdd	///< log-addition table
);

/**
 *	Set carry value
 */
HCILAB_PUBLIC HCI_BASICOP_API void
PowerASR_BasicOP_setCarry(hci_flag var_carry	///< new carry value
);

/**
 *	Get carry value
 *
 *	@return carry value
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_flag
PowerASR_BasicOP_getCarry(void);

/**
 *	Set overflow value
 */
HCILAB_PUBLIC HCI_BASICOP_API void
PowerASR_BasicOP_setOverflow(hci_flag var_overflow	///< new overflow value
);

/**
 *	Get overflow value
 *
 *	@return overflow value
 */
HCILAB_PUBLIC HCI_BASICOP_API hci_flag
PowerASR_BasicOP_getOverflow(void);


#ifdef __cplusplus
}
#endif

#endif // #ifndef __HCILAB_BASIC_OP_H__

