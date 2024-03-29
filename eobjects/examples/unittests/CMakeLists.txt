# bluetree/eobjects/examples/unittests/CmakeLists.txt - eobjects unit tests
cmake_minimum_required(VERSION 3.5)

# Set project name (= project root folder name).
set(E_REPO "bluetree")
set(E_PROJECT "unittests")
set(E_UP "../../../../eosal/osbuild/cmakedefs")

# Set build root environment variable E_ROOT
include("${E_UP}/eosal-root-path.txt")

if("${IDF_TARGET}" STREQUAL "esp32")
  # ESP-IFD only: Include IDF project setup and declare the project.
  include($ENV{IDF_PATH}/tools/cmake/project.cmake)
  project(${E_PROJECT})

  # include build information common to all projects.
  include("${E_UP}/eosal-defs-espidf.txt")

else()
  project(${E_PROJECT})

  # include build information common to all projects.
  include("${E_UP}/eosal-defs.txt")

  # Select libraries to link with application.
  set(E_APPLIBS "eobjects${E_POSTFIX};iocom${E_POSTFIX};$ENV{OSAL_TLS_APP_LIBS}")
  # if ($ENV{E_OSVER} MATCHES "pi")
  #   set(E_APPLIBS "${E_APPLIBS};rt")
  # endif()

  # Build individual library projects.
  add_subdirectory($ENV{E_ROOT}/${E_REPO}/eobjects "${CMAKE_CURRENT_BINARY_DIR}/eobjects")
  add_subdirectory($ENV{E_ROOT}/eosal "${CMAKE_CURRENT_BINARY_DIR}/eosal")
  add_subdirectory($ENV{E_ROOT}/iocom "${CMAKE_CURRENT_BINARY_DIR}/iocom")

  # Set path to where to keep libraries.
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $ENV{E_BIN})

  # Set path to source files.
  set(E_SOURCE_PATH "$ENV{E_ROOT}/${E_REPO}/eobjects/examples/${E_PROJECT}/code")

  # Set include path for the project.
  include_directories("${E_SOURCE_PATH}")
  include_directories("$ENV{E_ROOT}/iocom")
  include_directories("$ENV{E_ROOT}/${E_REPO}/eobjects")

  # Add header files, the file(GLOB_RECURSE...) allows for wildcards and recurses subdirs.
  file(GLOB_RECURSE HEADERS "${E_SOURCE_PATH}/*.h")

  # Add source files.
  file(GLOB_RECURSE SOURCES "${E_SOURCE_PATH}/*.cpp")

  # Build executable. Set library folder and libraries to link with.
  link_directories($ENV{E_LIB})
  add_executable(${E_PROJECT}${E_POSTFIX} ${HEADERS} ${SOURCES})
  target_link_libraries(${E_PROJECT}${E_POSTFIX} ${E_APPLIBS})

endif()
