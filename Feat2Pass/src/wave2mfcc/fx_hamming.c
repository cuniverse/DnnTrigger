
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
 *	@file	fx_hamming.c
 *	@ingroup wave2mfcc_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	Hamming windowing library
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "base/hci_type.h"
#include "basic_op/basic_op.h"
#include "wave2mfcc/fx_hamming.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**
 *	create Hamming window vector
 */
void FX_SigProc_createHammingWindow(hci_hamming_t *pHamming,	///< (o) hamming window vector
									hci_int16 nFrameWidth)		///< (i) frame width
{
#ifdef FIXED_POINT_FE
	if ( nFrameWidth == 240 ) {
		pHamming[0] = 2623;
		pHamming[1] = 2633;
		pHamming[2] = 2654;
		pHamming[3] = 2685;
		pHamming[4] = 2726;
		pHamming[5] = 2777;
		pHamming[6] = 2839;
		pHamming[7] = 2911;
		pHamming[8] = 2993;
		pHamming[9] = 3085;
		pHamming[10] = 3187;
		pHamming[11] = 3299;
		pHamming[12] = 3421;
		pHamming[13] = 3553;
		pHamming[14] = 3694;
		pHamming[15] = 3845;
		pHamming[16] = 4006;
		pHamming[17] = 4176;
		pHamming[18] = 4355;
		pHamming[19] = 4543;
		pHamming[20] = 4741;
		pHamming[21] = 4947;
		pHamming[22] = 5162;
		pHamming[23] = 5385;
		pHamming[24] = 5617;
		pHamming[25] = 5857;
		pHamming[26] = 6106;
		pHamming[27] = 6362;
		pHamming[28] = 6626;
		pHamming[29] = 6897;
		pHamming[30] = 7177;
		pHamming[31] = 7463;
		pHamming[32] = 7756;
		pHamming[33] = 8056;
		pHamming[34] = 8363;
		pHamming[35] = 8676;
		pHamming[36] = 8995;
		pHamming[37] = 9320;
		pHamming[38] = 9651;
		pHamming[39] = 9988;
		pHamming[40] = 10329;
		pHamming[41] = 10676;
		pHamming[42] = 11028;
		pHamming[43] = 11384;
		pHamming[44] = 11744;
		pHamming[45] = 12109;
		pHamming[46] = 12477;
		pHamming[47] = 12849;
		pHamming[48] = 13224;
		pHamming[49] = 13603;
		pHamming[50] = 13984;
		pHamming[51] = 14368;
		pHamming[52] = 14754;
		pHamming[53] = 15142;
		pHamming[54] = 15531;
		pHamming[55] = 15923;
		pHamming[56] = 16315;
		pHamming[57] = 16708;
		pHamming[58] = 17102;
		pHamming[59] = 17497;
		pHamming[60] = 17891;
		pHamming[61] = 18286;
		pHamming[62] = 18680;
		pHamming[63] = 19073;
		pHamming[64] = 19466;
		pHamming[65] = 19857;
		pHamming[66] = 20247;
		pHamming[67] = 20635;
		pHamming[68] = 21021;
		pHamming[69] = 21404;
		pHamming[70] = 21786;
		pHamming[71] = 22164;
		pHamming[72] = 22539;
		pHamming[73] = 22911;
		pHamming[74] = 23280;
		pHamming[75] = 23644;
		pHamming[76] = 24005;
		pHamming[77] = 24361;
		pHamming[78] = 24712;
		pHamming[79] = 25059;
		pHamming[80] = 25401;
		pHamming[81] = 25737;
		pHamming[82] = 26068;
		pHamming[83] = 26393;
		pHamming[84] = 26713;
		pHamming[85] = 27026;
		pHamming[86] = 27332;
		pHamming[87] = 27632;
		pHamming[88] = 27926;
		pHamming[89] = 28212;
		pHamming[90] = 28491;
		pHamming[91] = 28762;
		pHamming[92] = 29027;
		pHamming[93] = 29283;
		pHamming[94] = 29531;
		pHamming[95] = 29771;
		pHamming[96] = 30003;
		pHamming[97] = 30227;
		pHamming[98] = 30442;
		pHamming[99] = 30648;
		pHamming[100] = 30845;
		pHamming[101] = 31033;
		pHamming[102] = 31213;
		pHamming[103] = 31382;
		pHamming[104] = 31543;
		pHamming[105] = 31694;
		pHamming[106] = 31835;
		pHamming[107] = 31967;
		pHamming[108] = 32089;
		pHamming[109] = 32201;
		pHamming[110] = 32303;
		pHamming[111] = 32395;
		pHamming[112] = 32477;
		pHamming[113] = 32549;
		pHamming[114] = 32611;
		pHamming[115] = 32663;
		pHamming[116] = 32704;
		pHamming[117] = 32735;
		pHamming[118] = 32755;
		pHamming[119] = 32766;
		pHamming[120] = 32766;
		pHamming[121] = 32755;
		pHamming[122] = 32735;
		pHamming[123] = 32704;
		pHamming[124] = 32663;
		pHamming[125] = 32611;
		pHamming[126] = 32549;
		pHamming[127] = 32477;
		pHamming[128] = 32395;
		pHamming[129] = 32303;
		pHamming[130] = 32201;
		pHamming[131] = 32089;
		pHamming[132] = 31967;
		pHamming[133] = 31835;
		pHamming[134] = 31694;
		pHamming[135] = 31543;
		pHamming[136] = 31382;
		pHamming[137] = 31213;
		pHamming[138] = 31033;
		pHamming[139] = 30845;
		pHamming[140] = 30648;
		pHamming[141] = 30442;
		pHamming[142] = 30227;
		pHamming[143] = 30003;
		pHamming[144] = 29771;
		pHamming[145] = 29531;
		pHamming[146] = 29283;
		pHamming[147] = 29027;
		pHamming[148] = 28762;
		pHamming[149] = 28491;
		pHamming[150] = 28212;
		pHamming[151] = 27926;
		pHamming[152] = 27632;
		pHamming[153] = 27332;
		pHamming[154] = 27026;
		pHamming[155] = 26713;
		pHamming[156] = 26393;
		pHamming[157] = 26068;
		pHamming[158] = 25737;
		pHamming[159] = 25401;
		pHamming[160] = 25059;
		pHamming[161] = 24712;
		pHamming[162] = 24361;
		pHamming[163] = 24005;
		pHamming[164] = 23644;
		pHamming[165] = 23280;
		pHamming[166] = 22911;
		pHamming[167] = 22539;
		pHamming[168] = 22164;
		pHamming[169] = 21786;
		pHamming[170] = 21404;
		pHamming[171] = 21021;
		pHamming[172] = 20635;
		pHamming[173] = 20247;
		pHamming[174] = 19857;
		pHamming[175] = 19466;
		pHamming[176] = 19073;
		pHamming[177] = 18680;
		pHamming[178] = 18286;
		pHamming[179] = 17891;
		pHamming[180] = 17497;
		pHamming[181] = 17102;
		pHamming[182] = 16708;
		pHamming[183] = 16315;
		pHamming[184] = 15923;
		pHamming[185] = 15531;
		pHamming[186] = 15142;
		pHamming[187] = 14754;
		pHamming[188] = 14368;
		pHamming[189] = 13984;
		pHamming[190] = 13603;
		pHamming[191] = 13224;
		pHamming[192] = 12849;
		pHamming[193] = 12477;
		pHamming[194] = 12109;
		pHamming[195] = 11744;
		pHamming[196] = 11384;
		pHamming[197] = 11028;
		pHamming[198] = 10676;
		pHamming[199] = 10329;
		pHamming[200] = 9988;
		pHamming[201] = 9651;
		pHamming[202] = 9320;
		pHamming[203] = 8995;
		pHamming[204] = 8676;
		pHamming[205] = 8363;
		pHamming[206] = 8056;
		pHamming[207] = 7756;
		pHamming[208] = 7463;
		pHamming[209] = 7177;
		pHamming[210] = 6897;
		pHamming[211] = 6626;
		pHamming[212] = 6362;
		pHamming[213] = 6106;
		pHamming[214] = 5857;
		pHamming[215] = 5617;
		pHamming[216] = 5385;
		pHamming[217] = 5162;
		pHamming[218] = 4947;
		pHamming[219] = 4741;
		pHamming[220] = 4543;
		pHamming[221] = 4355;
		pHamming[222] = 4176;
		pHamming[223] = 4006;
		pHamming[224] = 3845;
		pHamming[225] = 3694;
		pHamming[226] = 3553;
		pHamming[227] = 3421;
		pHamming[228] = 3299;
		pHamming[229] = 3187;
		pHamming[230] = 3085;
		pHamming[231] = 2993;
		pHamming[232] = 2911;
		pHamming[233] = 2839;
		pHamming[234] = 2777;
		pHamming[235] = 2726;
		pHamming[236] = 2685;
		pHamming[237] = 2654;
		pHamming[238] = 2633;
		pHamming[239] = 2623;
	}
	else if (nFrameWidth == 480) {
		pHamming[0] = 2622;
		pHamming[1] = 2624;
		pHamming[2] = 2629;
		pHamming[3] = 2637;
		pHamming[4] = 2648;
		pHamming[5] = 2660;
		pHamming[6] = 2676;
		pHamming[7] = 2694;
		pHamming[8] = 2715;
		pHamming[9] = 2738;
		pHamming[10] = 2764;
		pHamming[11] = 2792;
		pHamming[12] = 2823;
		pHamming[13] = 2856;
		pHamming[14] = 2892;
		pHamming[15] = 2931;
		pHamming[16] = 2972;
		pHamming[17] = 3015;
		pHamming[18] = 3061;
		pHamming[19] = 3110;
		pHamming[20] = 3161;
		pHamming[21] = 3214;
		pHamming[22] = 3270;
		pHamming[23] = 3329;
		pHamming[24] = 3390;
		pHamming[25] = 3453;
		pHamming[26] = 3519;
		pHamming[27] = 3587;
		pHamming[28] = 3658;
		pHamming[29] = 3731;
		pHamming[30] = 3807;
		pHamming[31] = 3885;
		pHamming[32] = 3965;
		pHamming[33] = 4047;
		pHamming[34] = 4132;
		pHamming[35] = 4220;
		pHamming[36] = 4309;
		pHamming[37] = 4401;
		pHamming[38] = 4495;
		pHamming[39] = 4592;
		pHamming[40] = 4690;
		pHamming[41] = 4791;
		pHamming[42] = 4894;
		pHamming[43] = 5000;
		pHamming[44] = 5107;
		pHamming[45] = 5217;
		pHamming[46] = 5328;
		pHamming[47] = 5442;
		pHamming[48] = 5558;
		pHamming[49] = 5676;
		pHamming[50] = 5796;
		pHamming[51] = 5919;
		pHamming[52] = 6043;
		pHamming[53] = 6169;
		pHamming[54] = 6297;
		pHamming[55] = 6427;
		pHamming[56] = 6559;
		pHamming[57] = 6693;
		pHamming[58] = 6829;
		pHamming[59] = 6967;
		pHamming[60] = 7106;
		pHamming[61] = 7247;
		pHamming[62] = 7391;
		pHamming[63] = 7535;
		pHamming[64] = 7682;
		pHamming[65] = 7830;
		pHamming[66] = 7980;
		pHamming[67] = 8132;
		pHamming[68] = 8285;
		pHamming[69] = 8440;
		pHamming[70] = 8597;
		pHamming[71] = 8755;
		pHamming[72] = 8915;
		pHamming[73] = 9076;
		pHamming[74] = 9238;
		pHamming[75] = 9402;
		pHamming[76] = 9568;
		pHamming[77] = 9735;
		pHamming[78] = 9903;
		pHamming[79] = 10072;
		pHamming[80] = 10243;
		pHamming[81] = 10416;
		pHamming[82] = 10589;
		pHamming[83] = 10764;
		pHamming[84] = 10939;
		pHamming[85] = 11116;
		pHamming[86] = 11294;
		pHamming[87] = 11474;
		pHamming[88] = 11654;
		pHamming[89] = 11835;
		pHamming[90] = 12017;
		pHamming[91] = 12201;
		pHamming[92] = 12385;
		pHamming[93] = 12570;
		pHamming[94] = 12756;
		pHamming[95] = 12943;
		pHamming[96] = 13130;
		pHamming[97] = 13319;
		pHamming[98] = 13508;
		pHamming[99] = 13698;
		pHamming[100] = 13888;
		pHamming[101] = 14080;
		pHamming[102] = 14272;
		pHamming[103] = 14464;
		pHamming[104] = 14657;
		pHamming[105] = 14850;
		pHamming[106] = 15044;
		pHamming[107] = 15239;
		pHamming[108] = 15434;
		pHamming[109] = 15629;
		pHamming[110] = 15825;
		pHamming[111] = 16021;
		pHamming[112] = 16217;
		pHamming[113] = 16413;
		pHamming[114] = 16610;
		pHamming[115] = 16807;
		pHamming[116] = 17004;
		pHamming[117] = 17201;
		pHamming[118] = 17398;
		pHamming[119] = 17596;
		pHamming[120] = 17793;
		pHamming[121] = 17990;
		pHamming[122] = 18187;
		pHamming[123] = 18384;
		pHamming[124] = 18582;
		pHamming[125] = 18778;
		pHamming[126] = 18975;
		pHamming[127] = 19172;
		pHamming[128] = 19368;
		pHamming[129] = 19564;
		pHamming[130] = 19759;
		pHamming[131] = 19955;
		pHamming[132] = 20149;
		pHamming[133] = 20344;
		pHamming[134] = 20538;
		pHamming[135] = 20731;
		pHamming[136] = 20924;
		pHamming[137] = 21117;
		pHamming[138] = 21309;
		pHamming[139] = 21500;
		pHamming[140] = 21691;
		pHamming[141] = 21880;
		pHamming[142] = 22070;
		pHamming[143] = 22258;
		pHamming[144] = 22446;
		pHamming[145] = 22632;
		pHamming[146] = 22818;
		pHamming[147] = 23004;
		pHamming[148] = 23188;
		pHamming[149] = 23371;
		pHamming[150] = 23553;
		pHamming[151] = 23735;
		pHamming[152] = 23915;
		pHamming[153] = 24094;
		pHamming[154] = 24272;
		pHamming[155] = 24449;
		pHamming[156] = 24625;
		pHamming[157] = 24799;
		pHamming[158] = 24973;
		pHamming[159] = 25145;
		pHamming[160] = 25316;
		pHamming[161] = 25485;
		pHamming[162] = 25654;
		pHamming[163] = 25821;
		pHamming[164] = 25986;
		pHamming[165] = 26150;
		pHamming[166] = 26313;
		pHamming[167] = 26474;
		pHamming[168] = 26633;
		pHamming[169] = 26791;
		pHamming[170] = 26948;
		pHamming[171] = 27103;
		pHamming[172] = 27256;
		pHamming[173] = 27408;
		pHamming[174] = 27558;
		pHamming[175] = 27706;
		pHamming[176] = 27853;
		pHamming[177] = 27998;
		pHamming[178] = 28141;
		pHamming[179] = 28282;
		pHamming[180] = 28422;
		pHamming[181] = 28559;
		pHamming[182] = 28695;
		pHamming[183] = 28829;
		pHamming[184] = 28961;
		pHamming[185] = 29091;
		pHamming[186] = 29219;
		pHamming[187] = 29346;
		pHamming[188] = 29470;
		pHamming[189] = 29592;
		pHamming[190] = 29712;
		pHamming[191] = 29830;
		pHamming[192] = 29946;
		pHamming[193] = 30060;
		pHamming[194] = 30172;
		pHamming[195] = 30281;
		pHamming[196] = 30389;
		pHamming[197] = 30494;
		pHamming[198] = 30597;
		pHamming[199] = 30698;
		pHamming[200] = 30797;
		pHamming[201] = 30893;
		pHamming[202] = 30987;
		pHamming[203] = 31079;
		pHamming[204] = 31169;
		pHamming[205] = 31256;
		pHamming[206] = 31341;
		pHamming[207] = 31423;
		pHamming[208] = 31504;
		pHamming[209] = 31582;
		pHamming[210] = 31657;
		pHamming[211] = 31730;
		pHamming[212] = 31801;
		pHamming[213] = 31869;
		pHamming[214] = 31935;
		pHamming[215] = 31998;
		pHamming[216] = 32059;
		pHamming[217] = 32118;
		pHamming[218] = 32174;
		pHamming[219] = 32228;
		pHamming[220] = 32279;
		pHamming[221] = 32327;
		pHamming[222] = 32373;
		pHamming[223] = 32417;
		pHamming[224] = 32458;
		pHamming[225] = 32496;
		pHamming[226] = 32532;
		pHamming[227] = 32566;
		pHamming[228] = 32597;
		pHamming[229] = 32625;
		pHamming[230] = 32651;
		pHamming[231] = 32674;
		pHamming[232] = 32694;
		pHamming[233] = 32712;
		pHamming[234] = 32728;
		pHamming[235] = 32741;
		pHamming[236] = 32751;
		pHamming[237] = 32759;
		pHamming[238] = 32764;
		pHamming[239] = 32767;
		pHamming[240] = 32767;
		pHamming[241] = 32764;
		pHamming[242] = 32759;
		pHamming[243] = 32751;
		pHamming[244] = 32741;
		pHamming[245] = 32728;
		pHamming[246] = 32712;
		pHamming[247] = 32694;
		pHamming[248] = 32674;
		pHamming[249] = 32651;
		pHamming[250] = 32625;
		pHamming[251] = 32597;
		pHamming[252] = 32566;
		pHamming[253] = 32532;
		pHamming[254] = 32496;
		pHamming[255] = 32458;
		pHamming[256] = 32417;
		pHamming[257] = 32373;
		pHamming[258] = 32327;
		pHamming[259] = 32279;
		pHamming[260] = 32228;
		pHamming[261] = 32174;
		pHamming[262] = 32118;
		pHamming[263] = 32059;
		pHamming[264] = 31998;
		pHamming[265] = 31935;
		pHamming[266] = 31869;
		pHamming[267] = 31801;
		pHamming[268] = 31730;
		pHamming[269] = 31657;
		pHamming[270] = 31582;
		pHamming[271] = 31504;
		pHamming[272] = 31423;
		pHamming[273] = 31341;
		pHamming[274] = 31256;
		pHamming[275] = 31169;
		pHamming[276] = 31079;
		pHamming[277] = 30987;
		pHamming[278] = 30893;
		pHamming[279] = 30797;
		pHamming[280] = 30698;
		pHamming[281] = 30597;
		pHamming[282] = 30494;
		pHamming[283] = 30389;
		pHamming[284] = 30281;
		pHamming[285] = 30172;
		pHamming[286] = 30060;
		pHamming[287] = 29946;
		pHamming[288] = 29830;
		pHamming[289] = 29712;
		pHamming[290] = 29592;
		pHamming[291] = 29470;
		pHamming[292] = 29346;
		pHamming[293] = 29219;
		pHamming[294] = 29091;
		pHamming[295] = 28961;
		pHamming[296] = 28829;
		pHamming[297] = 28695;
		pHamming[298] = 28559;
		pHamming[299] = 28422;
		pHamming[300] = 28282;
		pHamming[301] = 28141;
		pHamming[302] = 27998;
		pHamming[303] = 27853;
		pHamming[304] = 27706;
		pHamming[305] = 27558;
		pHamming[306] = 27408;
		pHamming[307] = 27256;
		pHamming[308] = 27103;
		pHamming[309] = 26948;
		pHamming[310] = 26791;
		pHamming[311] = 26633;
		pHamming[312] = 26474;
		pHamming[313] = 26313;
		pHamming[314] = 26150;
		pHamming[315] = 25986;
		pHamming[316] = 25821;
		pHamming[317] = 25654;
		pHamming[318] = 25485;
		pHamming[319] = 25316;
		pHamming[320] = 25145;
		pHamming[321] = 24973;
		pHamming[322] = 24799;
		pHamming[323] = 24625;
		pHamming[324] = 24449;
		pHamming[325] = 24272;
		pHamming[326] = 24094;
		pHamming[327] = 23915;
		pHamming[328] = 23735;
		pHamming[329] = 23553;
		pHamming[330] = 23371;
		pHamming[331] = 23188;
		pHamming[332] = 23004;
		pHamming[333] = 22818;
		pHamming[334] = 22632;
		pHamming[335] = 22446;
		pHamming[336] = 22258;
		pHamming[337] = 22070;
		pHamming[338] = 21880;
		pHamming[339] = 21691;
		pHamming[340] = 21500;
		pHamming[341] = 21309;
		pHamming[342] = 21117;
		pHamming[343] = 20924;
		pHamming[344] = 20731;
		pHamming[345] = 20538;
		pHamming[346] = 20344;
		pHamming[347] = 20149;
		pHamming[348] = 19955;
		pHamming[349] = 19759;
		pHamming[350] = 19564;
		pHamming[351] = 19368;
		pHamming[352] = 19172;
		pHamming[353] = 18975;
		pHamming[354] = 18778;
		pHamming[355] = 18582;
		pHamming[356] = 18384;
		pHamming[357] = 18187;
		pHamming[358] = 17990;
		pHamming[359] = 17793;
		pHamming[360] = 17596;
		pHamming[361] = 17398;
		pHamming[362] = 17201;
		pHamming[363] = 17004;
		pHamming[364] = 16807;
		pHamming[365] = 16610;
		pHamming[366] = 16413;
		pHamming[367] = 16217;
		pHamming[368] = 16021;
		pHamming[369] = 15825;
		pHamming[370] = 15629;
		pHamming[371] = 15434;
		pHamming[372] = 15239;
		pHamming[373] = 15044;
		pHamming[374] = 14850;
		pHamming[375] = 14657;
		pHamming[376] = 14464;
		pHamming[377] = 14272;
		pHamming[378] = 14080;
		pHamming[379] = 13888;
		pHamming[380] = 13698;
		pHamming[381] = 13508;
		pHamming[382] = 13319;
		pHamming[383] = 13130;
		pHamming[384] = 12943;
		pHamming[385] = 12756;
		pHamming[386] = 12570;
		pHamming[387] = 12385;
		pHamming[388] = 12201;
		pHamming[389] = 12017;
		pHamming[390] = 11835;
		pHamming[391] = 11654;
		pHamming[392] = 11474;
		pHamming[393] = 11294;
		pHamming[394] = 11116;
		pHamming[395] = 10939;
		pHamming[396] = 10764;
		pHamming[397] = 10589;
		pHamming[398] = 10416;
		pHamming[399] = 10243;
		pHamming[400] = 10072;
		pHamming[401] = 9903;
		pHamming[402] = 9735;
		pHamming[403] = 9568;
		pHamming[404] = 9402;
		pHamming[405] = 9238;
		pHamming[406] = 9076;
		pHamming[407] = 8915;
		pHamming[408] = 8755;
		pHamming[409] = 8597;
		pHamming[410] = 8440;
		pHamming[411] = 8285;
		pHamming[412] = 8132;
		pHamming[413] = 7980;
		pHamming[414] = 7830;
		pHamming[415] = 7682;
		pHamming[416] = 7535;
		pHamming[417] = 7391;
		pHamming[418] = 7247;
		pHamming[419] = 7106;
		pHamming[420] = 6967;
		pHamming[421] = 6829;
		pHamming[422] = 6693;
		pHamming[423] = 6559;
		pHamming[424] = 6427;
		pHamming[425] = 6297;
		pHamming[426] = 6169;
		pHamming[427] = 6043;
		pHamming[428] = 5919;
		pHamming[429] = 5796;
		pHamming[430] = 5676;
		pHamming[431] = 5558;
		pHamming[432] = 5442;
		pHamming[433] = 5328;
		pHamming[434] = 5217;
		pHamming[435] = 5107;
		pHamming[436] = 5000;
		pHamming[437] = 4894;
		pHamming[438] = 4791;
		pHamming[439] = 4690;
		pHamming[440] = 4592;
		pHamming[441] = 4495;
		pHamming[442] = 4401;
		pHamming[443] = 4309;
		pHamming[444] = 4220;
		pHamming[445] = 4132;
		pHamming[446] = 4047;
		pHamming[447] = 3965;
		pHamming[448] = 3885;
		pHamming[449] = 3807;
		pHamming[450] = 3731;
		pHamming[451] = 3658;
		pHamming[452] = 3587;
		pHamming[453] = 3519;
		pHamming[454] = 3453;
		pHamming[455] = 3390;
		pHamming[456] = 3329;
		pHamming[457] = 3270;
		pHamming[458] = 3214;
		pHamming[459] = 3161;
		pHamming[460] = 3110;
		pHamming[461] = 3061;
		pHamming[462] = 3015;
		pHamming[463] = 2972;
		pHamming[464] = 2931;
		pHamming[465] = 2892;
		pHamming[466] = 2856;
		pHamming[467] = 2823;
		pHamming[468] = 2792;
		pHamming[469] = 2764;
		pHamming[470] = 2738;
		pHamming[471] = 2715;
		pHamming[472] = 2694;
		pHamming[473] = 2676;
		pHamming[474] = 2660;
		pHamming[475] = 2648;
		pHamming[476] = 2637;
		pHamming[477] = 2629;
		pHamming[478] = 2624;
		pHamming[479] = 2622;
	}
	else {
		register hci_int16 i = 0;
		hci_int16 ham_cst046_q15 = 0;
		hci_int16 ham_cst054_q15 = 0;
		hci_int16 cnst = 0;
		hci_int16 cos_val = 0;
		hci_int16 tmp16 = 0;
		hci_int32 L_deg = 0;

		/*
		 *  conversion ...
		 *   i) pGpre->hamWin[i] = 0.54 - 0.46 * cos(a*(i-1))
		 *
		 *	    ham_cst046_q10 : Q10/16bit <-0.46
		 *      ham_cst054_q09 : Q0/16bit <-0.54
		 *  
		 *  ii) cos(a*(i-1))
		 *
		 *      cosine argument conversion from radian to degree
		 *	    a*(i-1) = {TPI/(frameSize-1)}*(i-1)
		 *              = (PI/180)*{360/(frameSize-1)}*(i-1)
		 *
		 *		{360/(frameSize-1)}*(i-1) <- angle in degree
		 *		cnst = {360/(frameSize-1)} : Q14/16bit
		 *		
		 *  [VERIFIED]
		 *	
		 *		value : pGpre->amWin
		 *	max error : 0.000190
		 *	min error : 0.000001
		 *	avg error : 0.000052
		 *
		 */
	
		ham_cst046_q15 = 15073;						/* Q15/16bit <- 0.46 */
		ham_cst054_q15 = 17694;						/* Q15/16bit <- 0.54 */

		cnst = PowerASR_BasicOP_divide_16_16(180, PowerASR_BasicOP_subtract_16_16(nFrameWidth, 1));		/* Q14/16bit */
		for(i = 0; i < nFrameWidth; i++)
		{		
			L_deg = PowerASR_BasicOP_multiply_16_32(cnst, i);								/* Q15/32bit */
			cos_val = PowerASR_BasicOP_fixedCosine_32_16(L_deg, 0);							/* Q15/16bit */
			tmp16 = PowerASR_BasicOP_multiplyWithRound_16_16(ham_cst046_q15, cos_val);		/* Q15/16bit */		
			pHamming[i] = PowerASR_BasicOP_subtract_16_16(ham_cst054_q15, tmp16);			/* Q15/16bit */
		}
	}
#else	// !FIXED_POINT_FE
	hci_int16 n = 0;
	double ftmp = 0;
	double hamming_double = 0;
	for ( n = 0; n < nFrameWidth; n++) {
		ftmp = M_PI * 2.0 * ((double)n + 0.5) / (double)nFrameWidth;
		hamming_double = 0.54 - 0.46 * cos(ftmp);
		pHamming[n] = (hci_hamming_t) hamming_double;
	}
#endif	// #ifdef FIXED_POINT_FE
}


/**
 *	apply Hamming windowing
 */
void FX_SigProc_applyHamming(hci_mfcc16 *pSample,		///< (i/o) input/output samples
							 hci_hamming_t *pHamming,	///< (i) hamming window vector
							 hci_int16 nFrameWidth)		///< (i) window length
{
	hci_mfcc16 *pRawBuf = pSample;
	hci_mfcc16 *pLastBuf = pSample + nFrameWidth;
	hci_hamming_t *pWgt = pHamming;

#ifdef FIXED_POINT_FE

	hci_int32 L_val_prod = 0;

	/*
	 *	[COMMENTS]
	 *
	 *	pGpre->hamWin : Q15/16bit
	 *			    s : Q(sft)/16bit <- pPre->inBuf
	 *
	 *	[VERIFIED]
	 *
	 *		value : s
	 *	max error : 0.035500
	 *	min error : 0.000065
	 *	avg error : 0.008560
	 *
	 */

	while (pRawBuf < pLastBuf) {
		L_val_prod = PowerASR_BasicOP_multiply_16_32(*pRawBuf,*pWgt);			// Q(sft+16)/32bit
		*pRawBuf = PowerASR_BasicOP_round_32_16(L_val_prod);					// Q(sft)/16bit
		pRawBuf++; pWgt++;
		L_val_prod = PowerASR_BasicOP_multiply_16_32(*pRawBuf,*pWgt);			// Q(sft+16)/32bit
		*pRawBuf = PowerASR_BasicOP_round_32_16(L_val_prod);					// Q(sft)/16bit
		pRawBuf++; pWgt++;
		L_val_prod = PowerASR_BasicOP_multiply_16_32(*pRawBuf,*pWgt);			// Q(sft+16)/32bit
		*pRawBuf = PowerASR_BasicOP_round_32_16(L_val_prod);					// Q(sft)/16bit
		pRawBuf++; pWgt++;
		L_val_prod = PowerASR_BasicOP_multiply_16_32(*pRawBuf,*pWgt);			// Q(sft+16)/32bit
		*pRawBuf = PowerASR_BasicOP_round_32_16(L_val_prod);					// Q(sft)/16bit
		pRawBuf++; pWgt++;
	}

#else	// !FIXED_POINT_FE

	while (pRawBuf < pLastBuf) {
		*pRawBuf *= (*pWgt);
		pRawBuf++; pWgt++;
		*pRawBuf *= (*pWgt);
		pRawBuf++; pWgt++;
		*pRawBuf *= (*pWgt);
		pRawBuf++; pWgt++;
		*pRawBuf *= (*pWgt);
		pRawBuf++; pWgt++;
	}

#endif	// #ifdef FIXED_POINT_FE

}
