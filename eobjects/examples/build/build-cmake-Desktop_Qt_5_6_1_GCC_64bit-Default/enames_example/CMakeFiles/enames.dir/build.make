# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.5

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


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

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /coderoot/borromean/eobjects/examples/build/cmake

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /coderoot/borromean/eobjects/examples/build/build-cmake-Desktop_Qt_5_6_1_GCC_64bit-Default

# Include any dependencies generated for this target.
include enames_example/CMakeFiles/enames.dir/depend.make

# Include the progress variables for this target.
include enames_example/CMakeFiles/enames.dir/progress.make

# Include the compile flags for this target's objects.
include enames_example/CMakeFiles/enames.dir/flags.make

enames_example/CMakeFiles/enames.dir/coderoot/borromean/eobjects/examples/enames/code/eobjects_names_example.cpp.o: enames_example/CMakeFiles/enames.dir/flags.make
enames_example/CMakeFiles/enames.dir/coderoot/borromean/eobjects/examples/enames/code/eobjects_names_example.cpp.o: /coderoot/borromean/eobjects/examples/enames/code/eobjects_names_example.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/coderoot/borromean/eobjects/examples/build/build-cmake-Desktop_Qt_5_6_1_GCC_64bit-Default/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object enames_example/CMakeFiles/enames.dir/coderoot/borromean/eobjects/examples/enames/code/eobjects_names_example.cpp.o"
	cd /coderoot/borromean/eobjects/examples/build/build-cmake-Desktop_Qt_5_6_1_GCC_64bit-Default/enames_example && /usr/bin/g++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/enames.dir/coderoot/borromean/eobjects/examples/enames/code/eobjects_names_example.cpp.o -c /coderoot/borromean/eobjects/examples/enames/code/eobjects_names_example.cpp

enames_example/CMakeFiles/enames.dir/coderoot/borromean/eobjects/examples/enames/code/eobjects_names_example.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/enames.dir/coderoot/borromean/eobjects/examples/enames/code/eobjects_names_example.cpp.i"
	cd /coderoot/borromean/eobjects/examples/build/build-cmake-Desktop_Qt_5_6_1_GCC_64bit-Default/enames_example && /usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /coderoot/borromean/eobjects/examples/enames/code/eobjects_names_example.cpp > CMakeFiles/enames.dir/coderoot/borromean/eobjects/examples/enames/code/eobjects_names_example.cpp.i

enames_example/CMakeFiles/enames.dir/coderoot/borromean/eobjects/examples/enames/code/eobjects_names_example.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/enames.dir/coderoot/borromean/eobjects/examples/enames/code/eobjects_names_example.cpp.s"
	cd /coderoot/borromean/eobjects/examples/build/build-cmake-Desktop_Qt_5_6_1_GCC_64bit-Default/enames_example && /usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /coderoot/borromean/eobjects/examples/enames/code/eobjects_names_example.cpp -o CMakeFiles/enames.dir/coderoot/borromean/eobjects/examples/enames/code/eobjects_names_example.cpp.s

enames_example/CMakeFiles/enames.dir/coderoot/borromean/eobjects/examples/enames/code/eobjects_names_example.cpp.o.requires:

.PHONY : enames_example/CMakeFiles/enames.dir/coderoot/borromean/eobjects/examples/enames/code/eobjects_names_example.cpp.o.requires

enames_example/CMakeFiles/enames.dir/coderoot/borromean/eobjects/examples/enames/code/eobjects_names_example.cpp.o.provides: enames_example/CMakeFiles/enames.dir/coderoot/borromean/eobjects/examples/enames/code/eobjects_names_example.cpp.o.requires
	$(MAKE) -f enames_example/CMakeFiles/enames.dir/build.make enames_example/CMakeFiles/enames.dir/coderoot/borromean/eobjects/examples/enames/code/eobjects_names_example.cpp.o.provides.build
.PHONY : enames_example/CMakeFiles/enames.dir/coderoot/borromean/eobjects/examples/enames/code/eobjects_names_example.cpp.o.provides

enames_example/CMakeFiles/enames.dir/coderoot/borromean/eobjects/examples/enames/code/eobjects_names_example.cpp.o.provides.build: enames_example/CMakeFiles/enames.dir/coderoot/borromean/eobjects/examples/enames/code/eobjects_names_example.cpp.o


# Object files for target enames
enames_OBJECTS = \
"CMakeFiles/enames.dir/coderoot/borromean/eobjects/examples/enames/code/eobjects_names_example.cpp.o"

# External object files for target enames
enames_EXTERNAL_OBJECTS =

/coderoot/borromean/bin/mint18/enames: enames_example/CMakeFiles/enames.dir/coderoot/borromean/eobjects/examples/enames/code/eobjects_names_example.cpp.o
/coderoot/borromean/bin/mint18/enames: enames_example/CMakeFiles/enames.dir/build.make
/coderoot/borromean/bin/mint18/enames: /coderoot/borromean/lib/mint18_gcc/libeobjects.a
/coderoot/borromean/bin/mint18/enames: /coderoot/borromean/lib/mint18_gcc/libeosal.a
/coderoot/borromean/bin/mint18/enames: enames_example/CMakeFiles/enames.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/coderoot/borromean/eobjects/examples/build/build-cmake-Desktop_Qt_5_6_1_GCC_64bit-Default/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable /coderoot/borromean/bin/mint18/enames"
	cd /coderoot/borromean/eobjects/examples/build/build-cmake-Desktop_Qt_5_6_1_GCC_64bit-Default/enames_example && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/enames.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
enames_example/CMakeFiles/enames.dir/build: /coderoot/borromean/bin/mint18/enames

.PHONY : enames_example/CMakeFiles/enames.dir/build

enames_example/CMakeFiles/enames.dir/requires: enames_example/CMakeFiles/enames.dir/coderoot/borromean/eobjects/examples/enames/code/eobjects_names_example.cpp.o.requires

.PHONY : enames_example/CMakeFiles/enames.dir/requires

enames_example/CMakeFiles/enames.dir/clean:
	cd /coderoot/borromean/eobjects/examples/build/build-cmake-Desktop_Qt_5_6_1_GCC_64bit-Default/enames_example && $(CMAKE_COMMAND) -P CMakeFiles/enames.dir/cmake_clean.cmake
.PHONY : enames_example/CMakeFiles/enames.dir/clean

enames_example/CMakeFiles/enames.dir/depend:
	cd /coderoot/borromean/eobjects/examples/build/build-cmake-Desktop_Qt_5_6_1_GCC_64bit-Default && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /coderoot/borromean/eobjects/examples/build/cmake /coderoot/borromean/eobjects/examples/enames/build/cmake /coderoot/borromean/eobjects/examples/build/build-cmake-Desktop_Qt_5_6_1_GCC_64bit-Default /coderoot/borromean/eobjects/examples/build/build-cmake-Desktop_Qt_5_6_1_GCC_64bit-Default/enames_example /coderoot/borromean/eobjects/examples/build/build-cmake-Desktop_Qt_5_6_1_GCC_64bit-Default/enames_example/CMakeFiles/enames.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : enames_example/CMakeFiles/enames.dir/depend

