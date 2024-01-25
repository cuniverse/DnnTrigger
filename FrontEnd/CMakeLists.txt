cmake_minimum_required(VERSION 2.8.11)

SET (FRONTENDDEF 
	"LINUX"
)

ADD_LIBRARY (FrontEnd STATIC
	src/frontend/hci_FrontEnd.c
	src/frontend/powerdsr_frontend.c
	src/hmm/dtree.c
	src/hmm/hci_resource_hmm.c
	src/hmm/load_hmm.c
	src/hmm/phoneseq2statenet.c
	src/hmm/phoneseq2symbolseq.c
	src/lexicon/hci_resource_lexicon.c
	src/lexicon/load_lexicon.c
	src/lexicon/text2lex.c
	src/speex/bits.c
	src/speex/bitwise.c
	src/speex/cb_search.c
	src/speex/exc_10_16_table.c
	src/speex/exc_10_32_table.c
	src/speex/exc_20_32_table.c
	src/speex/exc_5_256_table.c
	src/speex/exc_5_64_table.c
	src/speex/exc_8_128_table.c
	src/speex/filters.c
	src/speex/framing.c
	src/speex/gain_table.c
	src/speex/gain_table_lbr.c
	src/speex/hci_speex.c
	src/speex/hexc_10_32_table.c
	src/speex/hexc_table.c
	src/speex/high_lsp_tables.c
	src/speex/lpc.c
	src/speex/lsp.c
	src/speex/lsp_tables_nb.c
	src/speex/ltp.c
	src/speex/modes.c
	src/speex/modes_wb.c
	src/speex/nb_celp.c
	src/speex/quant_lsp.c
	src/speex/sb_celp.c
	src/speex/speex.c
	src/speex/speex_callbacks.c
	src/speex/speex_header.c
	src/speex/stereo.c
	src/speex/vbr.c
	src/speex/vq.c
	src/speex/window.c
)

target_compile_definitions(FrontEnd PRIVATE ${FRONTENDDEF})

target_include_directories(FrontEnd PUBLIC 
	include
	../Feat2Pass/include
)
