cmake_minimum_required(VERSION 2.8.11)

add_executable (TrgMicTester
	trg_mic_tester.cpp
)

target_compile_definitions(TrgMicTester PRIVATE
	"LINUX"
)

target_include_directories(TrgMicTester PUBLIC 
	../
)

set_property(TARGET TrgMicTester PROPERTY C_STANDARD 11)
set_property(TARGET TrgMicTester PROPERTY C_STANDARD_REQUIRED ON)
set_property(TARGET TrgMicTester PROPERTY CXX_STANDARD 11)
set_property(TARGET TrgMicTester PROPERTY CXX_STANDARD_REQUIRED ON)

find_package (OpenAL)
if (OPENAL_FOUND)
	target_include_directories(TrgMicTester PUBLIC ${OPENAL_INCLUDE_DIR})
	target_link_libraries (TrgMicTester ${OPENAL_LIBRARY})

else (OPENAL_FOUND)
	target_include_directories(TrgMicTester PUBLIC
		../include/${CMAKE_SYSTEM_NAME}/${TARGET_ARCH}
		../include/${CMAKE_SYSTEM_NAME}
	)

	find_library(OPENAL_LIBRARY
		NAMES openal OpenAL
		PATHS ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

	target_link_libraries (TrgMicTester ${OPENAL_LIBRARY})
endif (OPENAL_FOUND)

target_link_libraries (TrgMicTester
	SelvyWakeup
)
