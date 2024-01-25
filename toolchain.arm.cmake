
SET(CROSSS_PREFIX "aarch64-linux-gnu-")

SET(CMAKE_C_COMPILER "${CROSSS_PREFIX}gcc")
SET(CMAKE_CXX_COMPILER "${CROSSS_PREFIX}g++")
SET(CMAKE_LINKER "${CROSSS_PREFIX}ld")
SET(CMAKE_NM "${CROSSS_PREFIX}nm")
SET(CMAKE_OBJCOPY "${CROSSS_PREFIX}objcopy")
SET(CMAKE_OBJDUMP "${CROSSS_PREFIX}objdump")
SET(CMAKE_RANLIB "${CROSSS_PREFIX}ranlib")

INCLUDE(CMakeForceCompiler)
CMAKE_FORCE_C_COMPILER("${CROSSS_PREFIX}gcc" GNU)
CMAKE_FORCE_CXX_COMPILER("${CROSSS_PREFIX}g++" GNU)

# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=armv7-a")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=armv7-a")

set(CMAKE_FIND_ROOT_PATH /usr/aarch64-linux-gnu)
# set(CMAKE_INCLUDE_PATH  /usr/include/aarch64-linux-gnu)
# set(CMAKE_LIBRARY_PATH  /usr/lib/aarch64-linux-gnu)
set(CMAKE_PROGRAM_PATH  /usr/bin/aarch64-linux-gnu)

set(TARGET_ARCH armv8-a)

# SET(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
# SET(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)
# SET(CMAKE_LINKER aarch64-linux-gnu-ld)
# SET(CMAKE_NM aarch64-linux-gnu-nm)
# SET(CMAKE_OBJCOPY aarch64-linux-gnu-objcopy)
# SET(CMAKE_OBJDUMP aarch64-linux-gnu-objdump)
# SET(CMAKE_RANLIB aarch64-linux-gnu-ranlib)

