# Install script for directory: /coderoot/borromean/eobjects/examples/build/cmake

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/coderoot/borromean/eobjects/examples/build/build-cmake-Desktop_Qt_5_6_1_GCC_64bit-Default/eosal/cmake_install.cmake")
  include("/coderoot/borromean/eobjects/examples/build/build-cmake-Desktop_Qt_5_6_1_GCC_64bit-Default/eobjects/cmake_install.cmake")
  include("/coderoot/borromean/eobjects/examples/build/build-cmake-Desktop_Qt_5_6_1_GCC_64bit-Default/econtainer_example/cmake_install.cmake")
  include("/coderoot/borromean/eobjects/examples/build/build-cmake-Desktop_Qt_5_6_1_GCC_64bit-Default/evariables_example/cmake_install.cmake")
  include("/coderoot/borromean/eobjects/examples/build/build-cmake-Desktop_Qt_5_6_1_GCC_64bit-Default/enames_example/cmake_install.cmake")
  include("/coderoot/borromean/eobjects/examples/build/build-cmake-Desktop_Qt_5_6_1_GCC_64bit-Default/ethread_example/cmake_install.cmake")
  include("/coderoot/borromean/eobjects/examples/build/build-cmake-Desktop_Qt_5_6_1_GCC_64bit-Default/eproperty_example/cmake_install.cmake")
  include("/coderoot/borromean/eobjects/examples/build/build-cmake-Desktop_Qt_5_6_1_GCC_64bit-Default/econnection_example/cmake_install.cmake")
  include("/coderoot/borromean/eobjects/examples/build/build-cmake-Desktop_Qt_5_6_1_GCC_64bit-Default/eendpoint_example/cmake_install.cmake")

endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/coderoot/borromean/eobjects/examples/build/build-cmake-Desktop_Qt_5_6_1_GCC_64bit-Default/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
