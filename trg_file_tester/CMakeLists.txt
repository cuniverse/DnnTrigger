cmake_minimum_required(VERSION 2.8.11)

# find_package(SndFile REQUIRED)

add_executable (TrgFileTester
	main.cpp
)

target_compile_definitions(TrgFileTester PRIVATE
	"LINUX"
)

target_include_directories(TrgFileTester PUBLIC 
	../
	../include
	../Feat2Pass/include
	../FrontEnd/include
	../dnn_trigger_decoder/include
)

set_property(TARGET TrgFileTester PROPERTY C_STANDARD 11)
set_property(TARGET TrgFileTester PROPERTY C_STANDARD_REQUIRED ON)
set_property(TARGET TrgFileTester PROPERTY CXX_STANDARD 11)
set_property(TARGET TrgFileTester PROPERTY CXX_STANDARD_REQUIRED ON)

target_link_libraries (TrgFileTester
	SelvyWakeup
	sndfile
)
