# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.2

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
CMAKE_COMMAND = /usr/local/Cellar/cmake/3.2.2/bin/cmake

# The command to remove a file.
RM = /usr/local/Cellar/cmake/3.2.2/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/zhuqian/Documents/code/TinyGraphicBox/external

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/zhuqian/Documents/code/TinyGraphicBox/external/googletest

# Include any dependencies generated for this target.
include googletest/googlemock/gtest/CMakeFiles/gtest.dir/depend.make

# Include the progress variables for this target.
include googletest/googlemock/gtest/CMakeFiles/gtest.dir/progress.make

# Include the compile flags for this target's objects.
include googletest/googlemock/gtest/CMakeFiles/gtest.dir/flags.make

googletest/googlemock/gtest/CMakeFiles/gtest.dir/src/gtest-all.cc.o: googletest/googlemock/gtest/CMakeFiles/gtest.dir/flags.make
googletest/googlemock/gtest/CMakeFiles/gtest.dir/src/gtest-all.cc.o: googletest/src/gtest-all.cc
	$(CMAKE_COMMAND) -E cmake_progress_report /Users/zhuqian/Documents/code/TinyGraphicBox/external/googletest/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object googletest/googlemock/gtest/CMakeFiles/gtest.dir/src/gtest-all.cc.o"
	cd /Users/zhuqian/Documents/code/TinyGraphicBox/external/googletest/googletest/googlemock/gtest && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/gtest.dir/src/gtest-all.cc.o -c /Users/zhuqian/Documents/code/TinyGraphicBox/external/googletest/googletest/src/gtest-all.cc

googletest/googlemock/gtest/CMakeFiles/gtest.dir/src/gtest-all.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/gtest.dir/src/gtest-all.cc.i"
	cd /Users/zhuqian/Documents/code/TinyGraphicBox/external/googletest/googletest/googlemock/gtest && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /Users/zhuqian/Documents/code/TinyGraphicBox/external/googletest/googletest/src/gtest-all.cc > CMakeFiles/gtest.dir/src/gtest-all.cc.i

googletest/googlemock/gtest/CMakeFiles/gtest.dir/src/gtest-all.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/gtest.dir/src/gtest-all.cc.s"
	cd /Users/zhuqian/Documents/code/TinyGraphicBox/external/googletest/googletest/googlemock/gtest && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /Users/zhuqian/Documents/code/TinyGraphicBox/external/googletest/googletest/src/gtest-all.cc -o CMakeFiles/gtest.dir/src/gtest-all.cc.s

googletest/googlemock/gtest/CMakeFiles/gtest.dir/src/gtest-all.cc.o.requires:
.PHONY : googletest/googlemock/gtest/CMakeFiles/gtest.dir/src/gtest-all.cc.o.requires

googletest/googlemock/gtest/CMakeFiles/gtest.dir/src/gtest-all.cc.o.provides: googletest/googlemock/gtest/CMakeFiles/gtest.dir/src/gtest-all.cc.o.requires
	$(MAKE) -f googletest/googlemock/gtest/CMakeFiles/gtest.dir/build.make googletest/googlemock/gtest/CMakeFiles/gtest.dir/src/gtest-all.cc.o.provides.build
.PHONY : googletest/googlemock/gtest/CMakeFiles/gtest.dir/src/gtest-all.cc.o.provides

googletest/googlemock/gtest/CMakeFiles/gtest.dir/src/gtest-all.cc.o.provides.build: googletest/googlemock/gtest/CMakeFiles/gtest.dir/src/gtest-all.cc.o

# Object files for target gtest
gtest_OBJECTS = \
"CMakeFiles/gtest.dir/src/gtest-all.cc.o"

# External object files for target gtest
gtest_EXTERNAL_OBJECTS =

googletest/googlemock/gtest/libgtest.a: googletest/googlemock/gtest/CMakeFiles/gtest.dir/src/gtest-all.cc.o
googletest/googlemock/gtest/libgtest.a: googletest/googlemock/gtest/CMakeFiles/gtest.dir/build.make
googletest/googlemock/gtest/libgtest.a: googletest/googlemock/gtest/CMakeFiles/gtest.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX static library libgtest.a"
	cd /Users/zhuqian/Documents/code/TinyGraphicBox/external/googletest/googletest/googlemock/gtest && $(CMAKE_COMMAND) -P CMakeFiles/gtest.dir/cmake_clean_target.cmake
	cd /Users/zhuqian/Documents/code/TinyGraphicBox/external/googletest/googletest/googlemock/gtest && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/gtest.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
googletest/googlemock/gtest/CMakeFiles/gtest.dir/build: googletest/googlemock/gtest/libgtest.a
.PHONY : googletest/googlemock/gtest/CMakeFiles/gtest.dir/build

googletest/googlemock/gtest/CMakeFiles/gtest.dir/requires: googletest/googlemock/gtest/CMakeFiles/gtest.dir/src/gtest-all.cc.o.requires
.PHONY : googletest/googlemock/gtest/CMakeFiles/gtest.dir/requires

googletest/googlemock/gtest/CMakeFiles/gtest.dir/clean:
	cd /Users/zhuqian/Documents/code/TinyGraphicBox/external/googletest/googletest/googlemock/gtest && $(CMAKE_COMMAND) -P CMakeFiles/gtest.dir/cmake_clean.cmake
.PHONY : googletest/googlemock/gtest/CMakeFiles/gtest.dir/clean

googletest/googlemock/gtest/CMakeFiles/gtest.dir/depend:
	cd /Users/zhuqian/Documents/code/TinyGraphicBox/external/googletest && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/zhuqian/Documents/code/TinyGraphicBox/external /Users/zhuqian/Documents/code/TinyGraphicBox/external/googletest/googletest /Users/zhuqian/Documents/code/TinyGraphicBox/external/googletest /Users/zhuqian/Documents/code/TinyGraphicBox/external/googletest/googletest/googlemock/gtest /Users/zhuqian/Documents/code/TinyGraphicBox/external/googletest/googletest/googlemock/gtest/CMakeFiles/gtest.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : googletest/googlemock/gtest/CMakeFiles/gtest.dir/depend

