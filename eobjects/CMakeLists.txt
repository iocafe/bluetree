# bluetree/eobjects/CmakeLists.txt - Cmake build for eobjects library.
cmake_minimum_required(VERSION 3.5)

# Set project name (= project root folder name).
set(E_REPO "bluetree")
set(E_PROJECT "eobjects")
set(E_UP "../../eosal/osbuild/cmakedefs")

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

  # Set path to where to keep libraries.
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY $ENV{E_LIBOUT})
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY $ENV{E_LIBOUT})

  # Set path to source files.
  set(BACKBONE "$ENV{E_ROOT}/${E_REPO}/${E_PROJECT}/code")
  set(EXTENSIONS "$ENV{E_ROOT}/${E_REPO}/${E_PROJECT}/extensions")

  # Add library include paths.
  include_directories("$ENV{E_ROOT}/iocom")
  include_directories("$ENV{E_ROOT}/iocom/extensions/lighthouse")
  include_directories("$ENV{E_ROOT}/${E_REPO}/${E_PROJECT}")

  # Add main headers.
  set(MAIN_HEADERS "$ENV{E_ROOT}/${E_REPO}/${E_PROJECT}/${E_PROJECT}.h")

  # Add source files common to all operating systems.
  file(GLOB BB_CM_SOURCES "${BACKBONE}/*/*.cpp")
  file(GLOB BB_CM_HEADERS "${BACKBONE}/*/*.h")
  file(GLOB EX_CM_SOURCES "${EXTENSIONS}/*/*.cpp")
  file(GLOB EX_CM_HEADERS "${EXTENSIONS}/*/*.h")

  # Generate the static library
  add_library(${E_PROJECT}${E_POSTFIX} STATIC ${MAIN_HEADERS} ${BB_CM_HEADERS} ${EX_CM_HEADERS} ${BB_CM_SOURCES} ${EX_CM_SOURCES})

endif()
