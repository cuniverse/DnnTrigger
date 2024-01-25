cmake_minimum_required(VERSION 2.8.11)

SET (FRONTENDDEF 
	"LINUX"
)

add_library(Feat2Pass STATIC
	src/pffft.c
	src/base/binary_io.c
	src/base/case.c
	src/base/filename.c
	src/base/genrand.c
	src/base/glist.c
	src/base/hash.c
	src/base/hash_table.c
	src/base/hci_malloc.c
	src/base/hci_msg.c
	src/base/linklist.c
	src/base/parse_config.c
	src/base/str2words.c
	src/base/utf8.c
	src/basic_op/basic_op.c
	src/epd/epd_main.c
	src/epd/hci_epd.c
	src/mfcc2feat/fx_agc.c
	src/mfcc2feat/fx_cms.c
	src/mfcc2feat/fx_compressframe.c
	src/mfcc2feat/fx_delta.c
	src/mfcc2feat/fx_dropframe.c
	src/mfcc2feat/fx_mfcc2feat.c
	src/mfcc2feat/fx_quantizer.c
	src/mfcc2feat/hci_fx_mfcc2feat.c
	src/wave2mfcc/fx_hamming.c
	src/wave2mfcc/fx_mfcc.c
	src/wave2mfcc/fx_sigproc.c
	src/wave2mfcc/hci_wave2mfcc.c
	src/wiener/BufferIn.c
	src/wiener/classifyFrame.c
	src/wiener/computePCorr.c
	src/wiener/dsrAfeVad.c
	src/wiener/hci_wiener.c
	src/wiener/MelProc.c
	src/wiener/NoiseSup.c
	src/wiener/ParmInterface.c
	src/wiener/pitch.c
	src/wiener/preProc.c
)

set_property(TARGET Feat2Pass PROPERTY C_STANDARD 11)
set_property(TARGET Feat2Pass PROPERTY C_STANDARD_REQUIRED ON)

target_compile_definitions(Feat2Pass PRIVATE ${FRONTENDDEF})

target_include_directories(Feat2Pass PUBLIC 
	include
)
