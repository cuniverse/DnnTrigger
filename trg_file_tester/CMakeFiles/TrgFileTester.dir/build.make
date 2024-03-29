# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 2.8

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list

# Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The program to use to edit the cache.
CMAKE_EDIT_COMMAND = /usr/bin/ccmake

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/cuniverse/trigger/DnnTrigger/trunk

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/cuniverse/trigger/DnnTrigger/trunk

# Include any dependencies generated for this target.
include trg_file_tester/CMakeFiles/TrgFileTester.dir/depend.make

# Include the progress variables for this target.
include trg_file_tester/CMakeFiles/TrgFileTester.dir/progress.make

# Include the compile flags for this target's objects.
include trg_file_tester/CMakeFiles/TrgFileTester.dir/flags.make

trg_file_tester/CMakeFiles/TrgFileTester.dir/main.cpp.o: trg_file_tester/CMakeFiles/TrgFileTester.dir/flags.make
trg_file_tester/CMakeFiles/TrgFileTester.dir/main.cpp.o: trg_file_tester/main.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/cuniverse/trigger/DnnTrigger/trunk/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object trg_file_tester/CMakeFiles/TrgFileTester.dir/main.cpp.o"
	cd /home/cuniverse/trigger/DnnTrigger/trunk/trg_file_tester && /opt/rh/devtoolset-7/root/usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/TrgFileTester.dir/main.cpp.o -c /home/cuniverse/trigger/DnnTrigger/trunk/trg_file_tester/main.cpp

trg_file_tester/CMakeFiles/TrgFileTester.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/TrgFileTester.dir/main.cpp.i"
	cd /home/cuniverse/trigger/DnnTrigger/trunk/trg_file_tester && /opt/rh/devtoolset-7/root/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/cuniverse/trigger/DnnTrigger/trunk/trg_file_tester/main.cpp > CMakeFiles/TrgFileTester.dir/main.cpp.i

trg_file_tester/CMakeFiles/TrgFileTester.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/TrgFileTester.dir/main.cpp.s"
	cd /home/cuniverse/trigger/DnnTrigger/trunk/trg_file_tester && /opt/rh/devtoolset-7/root/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/cuniverse/trigger/DnnTrigger/trunk/trg_file_tester/main.cpp -o CMakeFiles/TrgFileTester.dir/main.cpp.s

trg_file_tester/CMakeFiles/TrgFileTester.dir/main.cpp.o.requires:
.PHONY : trg_file_tester/CMakeFiles/TrgFileTester.dir/main.cpp.o.requires

trg_file_tester/CMakeFiles/TrgFileTester.dir/main.cpp.o.provides: trg_file_tester/CMakeFiles/TrgFileTester.dir/main.cpp.o.requires
	$(MAKE) -f trg_file_tester/CMakeFiles/TrgFileTester.dir/build.make trg_file_tester/CMakeFiles/TrgFileTester.dir/main.cpp.o.provides.build
.PHONY : trg_file_tester/CMakeFiles/TrgFileTester.dir/main.cpp.o.provides

trg_file_tester/CMakeFiles/TrgFileTester.dir/main.cpp.o.provides.build: trg_file_tester/CMakeFiles/TrgFileTester.dir/main.cpp.o

# Object files for target TrgFileTester
TrgFileTester_OBJECTS = \
"CMakeFiles/TrgFileTester.dir/main.cpp.o"

# External object files for target TrgFileTester
TrgFileTester_EXTERNAL_OBJECTS =

bin/ARM-armv8-a/TrgFileTester: trg_file_tester/CMakeFiles/TrgFileTester.dir/main.cpp.o
bin/ARM-armv8-a/TrgFileTester: trg_file_tester/CMakeFiles/TrgFileTester.dir/build.make
bin/ARM-armv8-a/TrgFileTester: bin/ARM-armv8-a/libSelvyWakeup.so
bin/ARM-armv8-a/TrgFileTester: FrontEnd/libFrontEnd.a
bin/ARM-armv8-a/TrgFileTester: Feat2Pass/libFeat2Pass.a
bin/ARM-armv8-a/TrgFileTester: trg_file_tester/CMakeFiles/TrgFileTester.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX executable ../bin/ARM-armv8-a/TrgFileTester"
	cd /home/cuniverse/trigger/DnnTrigger/trunk/trg_file_tester && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/TrgFileTester.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
trg_file_tester/CMakeFiles/TrgFileTester.dir/build: bin/ARM-armv8-a/TrgFileTester
.PHONY : trg_file_tester/CMakeFiles/TrgFileTester.dir/build

trg_file_tester/CMakeFiles/TrgFileTester.dir/requires: trg_file_tester/CMakeFiles/TrgFileTester.dir/main.cpp.o.requires
.PHONY : trg_file_tester/CMakeFiles/TrgFileTester.dir/requires

trg_file_tester/CMakeFiles/TrgFileTester.dir/clean:
	cd /home/cuniverse/trigger/DnnTrigger/trunk/trg_file_tester && $(CMAKE_COMMAND) -P CMakeFiles/TrgFileTester.dir/cmake_clean.cmake
.PHONY : trg_file_tester/CMakeFiles/TrgFileTester.dir/clean

trg_file_tester/CMakeFiles/TrgFileTester.dir/depend:
	cd /home/cuniverse/trigger/DnnTrigger/trunk && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/cuniverse/trigger/DnnTrigger/trunk /home/cuniverse/trigger/DnnTrigger/trunk/trg_file_tester /home/cuniverse/trigger/DnnTrigger/trunk /home/cuniverse/trigger/DnnTrigger/trunk/trg_file_tester /home/cuniverse/trigger/DnnTrigger/trunk/trg_file_tester/CMakeFiles/TrgFileTester.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : trg_file_tester/CMakeFiles/TrgFileTester.dir/depend

