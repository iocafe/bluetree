# bluetree/glass/CmakeLists.txt - Cmake build for glass user interface.
cmake_minimum_required(VERSION 3.5)

# Set project name (= project root folder name).
set(E_REPO "bluetree")
set(E_PROJECT "glass")
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

  # Select libraries to link with application.
  set(E_APPLIBS "egui${E_POSTFIX};eobjects${E_POSTFIX};lighthouse${E_POSTFIX};iocom${E_POSTFIX};$ENV{OSAL_TLS_APP_LIBS}")
  # if ($ENV{E_OSVER} STREQUAL "pi")
  #   set(E_APPLIBS "${E_APPLIBS};rt")
  # endif()

  # Add defines to indicate target operating system and target OS version (at level needed for build).
  add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_GLEW)

  find_package(PkgConfig REQUIRED)
  pkg_search_module(GLFW REQUIRED glfw3)
  pkg_search_module(GLEW REQUIRED glew)
  include_directories(${GLFW_INCLUDE_DIRS})
  include_directories(${GLEW_INCLUDE_DIRS})
  set(E_APPLIBS "${E_APPLIBS};GL")

  # Build individual library projects.
  add_subdirectory($ENV{E_ROOT}/${E_REPO}/egui "${CMAKE_CURRENT_BINARY_DIR}/egui")
  add_subdirectory($ENV{E_ROOT}/${E_REPO}/eobjects "${CMAKE_CURRENT_BINARY_DIR}/eobjects")
  add_subdirectory($ENV{E_ROOT}/eosal "${CMAKE_CURRENT_BINARY_DIR}/eosal")
  add_subdirectory($ENV{E_ROOT}/iocom "${CMAKE_CURRENT_BINARY_DIR}/iocom")
  add_subdirectory($ENV{E_ROOT}/iocom/extensions/lighthouse "${CMAKE_CURRENT_BINARY_DIR}/iocom/extensions/lighthouse")

  # Set path to where to keep libraries.
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $ENV{E_BIN})

  # Set path to source files.
  set(E_SOURCE_PATH "$ENV{E_ROOT}/${E_REPO}/${E_PROJECT}/code")

  # Set include path for the project.
  include_directories("$ENV{E_ROOT}/${E_REPO}/${E_PROJECT}")

  # Add libraries.
  include_directories("$ENV{E_ROOT}/${E_REPO}/egui")
  include_directories("$ENV{E_ROOT}/${E_REPO}/eobjects")
  include_directories("$ENV{E_ROOT}/iocom")
  include_directories("$ENV{E_ROOT}/iocom/extensions/lighthouse")

  # Add header files, the file(GLOB_RECURSE...) allows for wildcards and recurses subdirs.
  file(GLOB_RECURSE HEADERS "${E_SOURCE_PATH}/*.h")

  # Add source files.
  file(GLOB_RECURSE SOURCES "${E_SOURCE_PATH}/*.c" "${E_SOURCE_PATH}/*.cpp")

  # Build executable. Set library folder and libraries to link with.
  link_directories($ENV{E_LIB})
  add_executable(${E_PROJECT}${E_POSTFIX} ${HEADERS} ${SOURCES})
  target_link_libraries(${E_PROJECT}${E_POSTFIX} ${E_APPLIBS} ${GLFW_STATIC_LIBRARIES} ${GLEW_STATIC_LIBRARIES})

endif()
