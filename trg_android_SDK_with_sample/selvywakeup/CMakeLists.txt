# Sets the minimum version of CMake required to build the native
# library. You should either keep the default value or only pass a
# value of 3.4.0 or lower.

cmake_minimum_required(VERSION 3.4.1)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because system libraries are included in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )


add_library(SelvyWakeup SHARED IMPORTED)
set_target_properties(SelvyWakeup PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/src/main/extlibs/${ANDROID_ABI}/libSelvyWakeup.so)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds it for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
             SelvyWakeupJni

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             # Associated headers in the same location as their source
             # file are automatically included.
             src/main/cpp/com_selvasai_stt_selvywakeup_WakeUpSolid.cpp
)

target_include_directories(SelvyWakeupJni PUBLIC
	src/main/cpp/
)

target_compile_definitions(SelvyWakeupJni PRIVATE
	LINUX
)

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in the
# build script, prebuilt third-party libraries, or system libraries.

link_directories("../extlibs/${ANDROID_ABI}")

target_link_libraries( SelvyWakeupJni ${log-lib} SelvyWakeup )

