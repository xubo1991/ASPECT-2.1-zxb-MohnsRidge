# CMake generated Testfile for 
# Source directory: /home/peach/Desktop/Yinuo/aspect-2.1-Vis-Depth/tests
# Build directory: /home/peach/Desktop/Yinuo/aspect-2.1-Vis-Depth/build/tests
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(quick_mpi "/usr/local/pylith-2.2.1-linux-x86_64/bin/cmake" "-DBINARY_DIR=/home/peach/Desktop/Yinuo/aspect-2.1-Vis-Depth/build/tests" "-DTESTNAME=tests.quick_mpi" "-DERROR=\"Test quick_mpi failed\"" "-P" "/home/peach/Desktop/Yinuo/aspect-2.1-Vis-Depth/tests/run_test.cmake")
set_tests_properties(quick_mpi PROPERTIES  TIMEOUT "600" WORKING_DIRECTORY "/home/peach/Desktop/Yinuo/aspect-2.1-Vis-Depth/build/tests")
