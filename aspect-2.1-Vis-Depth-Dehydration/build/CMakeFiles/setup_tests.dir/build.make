# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.7

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
CMAKE_COMMAND = /usr/local/pylith-2.2.1-linux-x86_64/bin/cmake

# The command to remove a file.
RM = /usr/local/pylith-2.2.1-linux-x86_64/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/peach/Desktop/Yinuo/aspect-2.1-Vis-Depth

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/peach/Desktop/Yinuo/aspect-2.1-Vis-Depth/build

# Utility rule file for setup_tests.

# Include the progress variables for this target.
include CMakeFiles/setup_tests.dir/progress.make

CMakeFiles/setup_tests:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/peach/Desktop/Yinuo/aspect-2.1-Vis-Depth/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Enabling all tests ..."
	/usr/local/pylith-2.2.1-linux-x86_64/bin/cmake -D ASPECT_RUN_ALL_TESTS=ON -D PRINT_TEST_SUMMARY_AS_IMPORTANT=ON . >/dev/null

setup_tests: CMakeFiles/setup_tests
setup_tests: CMakeFiles/setup_tests.dir/build.make

.PHONY : setup_tests

# Rule to build all files generated by this target.
CMakeFiles/setup_tests.dir/build: setup_tests

.PHONY : CMakeFiles/setup_tests.dir/build

CMakeFiles/setup_tests.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/setup_tests.dir/cmake_clean.cmake
.PHONY : CMakeFiles/setup_tests.dir/clean

CMakeFiles/setup_tests.dir/depend:
	cd /home/peach/Desktop/Yinuo/aspect-2.1-Vis-Depth/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/peach/Desktop/Yinuo/aspect-2.1-Vis-Depth /home/peach/Desktop/Yinuo/aspect-2.1-Vis-Depth /home/peach/Desktop/Yinuo/aspect-2.1-Vis-Depth/build /home/peach/Desktop/Yinuo/aspect-2.1-Vis-Depth/build /home/peach/Desktop/Yinuo/aspect-2.1-Vis-Depth/build/CMakeFiles/setup_tests.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/setup_tests.dir/depend
