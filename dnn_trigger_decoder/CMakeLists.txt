cmake_minimum_required(VERSION 2.8.11)

ADD_LIBRARY (SelvyWakeup SHARED
	clog.cpp
	dnn_decoder.cpp
	dnn_trigger.cpp
	mono_trigger.cpp
	feat_2pass.cpp
	detector_word.cpp
	detector_mono.cpp
	SizedQueue.cpp
	Selvy_Trigger_API.cpp
	src/minIni.cpp
	src/bp_train.c
	src/deepnet_base.c
	src/deepnet_common.c
)

set_property(TARGET SelvyWakeup PROPERTY C_STANDARD 11)
set_property(TARGET SelvyWakeup PROPERTY C_STANDARD_REQUIRED ON)
set_property(TARGET SelvyWakeup PROPERTY CXX_STANDARD 11)
set_property(TARGET SelvyWakeup PROPERTY CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC")

target_compile_definitions(SelvyWakeup PRIVATE
	LINUX
)

target_include_directories(SelvyWakeup PUBLIC
	include
    ../include
	../FrontEnd/include
	../Feat2Pass/include
)

target_link_libraries (SelvyWakeup
	FrontEnd
	Feat2Pass
)

