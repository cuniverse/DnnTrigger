cmake_minimum_required(VERSION 2.8.11)


set(OUT_DIR "${CMAKE_HOME_DIRECTORY}/OUT/bin")

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUT_DIR})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11 -O3 -ffast-math -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,'$ORIGIN/:$$ORIGIN/'" )


add_library(SelvyWakeup SHARED IMPORTED)
set_target_properties(SelvyWakeup PROPERTIES IMPORTED_LOCATION ./lib/libSelvyWakeup.so)


add_executable (Trgger_Sample
	main.cpp
)

target_compile_definitions(Trgger_Sample PRIVATE
	"LINUX"
)

target_include_directories(Trgger_Sample PUBLIC 
	./include
)

set_property(TARGET Trgger_Sample PROPERTY C_STANDARD 11)
set_property(TARGET Trgger_Sample PROPERTY C_STANDARD_REQUIRED ON)
set_property(TARGET Trgger_Sample PROPERTY CXX_STANDARD 11)
set_property(TARGET Trgger_Sample PROPERTY CXX_STANDARD_REQUIRED ON)



target_link_libraries (Trgger_Sample
	SelvyWakeup
	sndfile
)
