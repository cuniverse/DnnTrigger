cmake_minimum_required(VERSION 2.8.11)
project(DnnTrigger)

# set(CMAKE_SYSTEM_NAME ARM)
# set(CMAKE_SYSTEM_VERSION 21) # API level
# set(CMAKE_ANDROID_ARCH_ABI armeabi-v7a)
# set(CMAKE_ANDROID_ARCH_ABI arm64-v8a)
# set(CMAKE_ANDROID_ARCH_ABI arm64-v8a)
# set(CMAKE_ANDROID_NDK /android/ndk)
# set(CMAKE_ANDROID_STL_TYPE gnustl_static)

# set(TARGET_ARCH armv8-a)
if(DEFINED CMAKE_SYSTEM_PROCESSOR)
set(OUT_DIR "${CMAKE_HOME_DIRECTORY}/bin/${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
else()
set(OUT_DIR "${CMAKE_HOME_DIRECTORY}/bin/${CMAKE_SYSTEM_NAME}")
endif()


set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUT_DIR})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11 -O3 -ffast-math -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC")

# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11 -O3 -ffast-math -fPIC -march=armv7-a")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC -march=armv7-a")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,'$ORIGIN/:$$ORIGIN/'" )

add_subdirectory(Feat2Pass Feat2Pass)
add_subdirectory(FrontEnd FrontEnd)
add_subdirectory(dnn_trigger_decoder dnn_trigger_decoder)
#add_subdirectory(trg_android_tester/selvywakeup)

add_subdirectory (trg_file_tester trg_file_tester)
add_subdirectory (trg_mic_tester trg_mic_tester)
#add_subdirectory (trg_octo_tester trg_octo_tester)

