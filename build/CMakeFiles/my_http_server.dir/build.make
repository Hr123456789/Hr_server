# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.25

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Produce verbose output by default.
VERBOSE = 1

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /home/hr/cmake-3.25.0-linux-x86_64/bin/cmake

# The command to remove a file.
RM = /home/hr/cmake-3.25.0-linux-x86_64/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/hr/sylar_v3.0

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/hr/sylar_v3.0/build

# Include any dependencies generated for this target.
include CMakeFiles/my_http_server.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/my_http_server.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/my_http_server.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/my_http_server.dir/flags.make

CMakeFiles/my_http_server.dir/samples/my_http_server.cc.o: CMakeFiles/my_http_server.dir/flags.make
CMakeFiles/my_http_server.dir/samples/my_http_server.cc.o: /home/hr/sylar_v3.0/samples/my_http_server.cc
CMakeFiles/my_http_server.dir/samples/my_http_server.cc.o: CMakeFiles/my_http_server.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/hr/sylar_v3.0/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/my_http_server.dir/samples/my_http_server.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/my_http_server.dir/samples/my_http_server.cc.o -MF CMakeFiles/my_http_server.dir/samples/my_http_server.cc.o.d -o CMakeFiles/my_http_server.dir/samples/my_http_server.cc.o -c /home/hr/sylar_v3.0/samples/my_http_server.cc

CMakeFiles/my_http_server.dir/samples/my_http_server.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/my_http_server.dir/samples/my_http_server.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/hr/sylar_v3.0/samples/my_http_server.cc > CMakeFiles/my_http_server.dir/samples/my_http_server.cc.i

CMakeFiles/my_http_server.dir/samples/my_http_server.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/my_http_server.dir/samples/my_http_server.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/hr/sylar_v3.0/samples/my_http_server.cc -o CMakeFiles/my_http_server.dir/samples/my_http_server.cc.s

# Object files for target my_http_server
my_http_server_OBJECTS = \
"CMakeFiles/my_http_server.dir/samples/my_http_server.cc.o"

# External object files for target my_http_server
my_http_server_EXTERNAL_OBJECTS =

/home/hr/sylar_v3.0/bin/my_http_server: CMakeFiles/my_http_server.dir/samples/my_http_server.cc.o
/home/hr/sylar_v3.0/bin/my_http_server: CMakeFiles/my_http_server.dir/build.make
/home/hr/sylar_v3.0/bin/my_http_server: /home/hr/sylar_v3.0/lib/libsylar.so
/home/hr/sylar_v3.0/bin/my_http_server: CMakeFiles/my_http_server.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/hr/sylar_v3.0/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable /home/hr/sylar_v3.0/bin/my_http_server"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/my_http_server.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/my_http_server.dir/build: /home/hr/sylar_v3.0/bin/my_http_server
.PHONY : CMakeFiles/my_http_server.dir/build

CMakeFiles/my_http_server.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/my_http_server.dir/cmake_clean.cmake
.PHONY : CMakeFiles/my_http_server.dir/clean

CMakeFiles/my_http_server.dir/depend:
	cd /home/hr/sylar_v3.0/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/hr/sylar_v3.0 /home/hr/sylar_v3.0 /home/hr/sylar_v3.0/build /home/hr/sylar_v3.0/build /home/hr/sylar_v3.0/build/CMakeFiles/my_http_server.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/my_http_server.dir/depend

