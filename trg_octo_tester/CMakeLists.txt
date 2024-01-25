cmake_minimum_required(VERSION 2.8.11)

link_directories (${CMAKE_HOME_DIRECTORY}/lib)

add_executable (TrgOctoTester
	main.cpp
)

target_compile_definitions(TrgOctoTester PRIVATE
	"LINUX"
)

target_include_directories(TrgOctoTester PUBLIC 
	../
	../include
	../include/Linux
	../Feat2Pass/include
	../FrontEnd/include
	../dnn_trigger_decoder/include
)

set_property(TARGET TrgOctoTester PROPERTY C_STANDARD 11)
set_property(TARGET TrgOctoTester PROPERTY C_STANDARD_REQUIRED ON)
set_property(TARGET TrgOctoTester PROPERTY CXX_STANDARD 11)
set_property(TARGET TrgOctoTester PROPERTY CXX_STANDARD_REQUIRED ON)

target_link_libraries (TrgOctoTester
	#DnnTrigger
	-Xlinker --start-group
	Feat2Pass
	FrontEnd
	SelvyWakeup
	-Xlinker --end-group

	m
	rt
	pthread
	Octopus

	sndfile
)
