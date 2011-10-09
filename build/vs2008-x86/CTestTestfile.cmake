# CMake generated Testfile for 
# Source directory: D:/p4/users/jbyrd/logog
# Build directory: D:/p4/users/jbyrd/logog/build/vs2008-x86
# 
# This file includes the relevent testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
IF("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  ADD_TEST(test-harness "D:/p4/users/jbyrd/logog/build/vs2008-x86/Debug/test.exe")
ENDIF("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
IF("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  ADD_TEST(test-harness "D:/p4/users/jbyrd/logog/build/vs2008-x86/Release/test.exe")
ENDIF("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
IF("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  ADD_TEST(test-harness "D:/p4/users/jbyrd/logog/build/vs2008-x86/MinSizeRel/test.exe")
ENDIF("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
IF("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  ADD_TEST(test-harness "D:/p4/users/jbyrd/logog/build/vs2008-x86/RelWithDebInfo/test.exe")
ENDIF("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
