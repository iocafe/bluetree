# bluetree/egui/CmakeLists.txt - Cmake build for egui library.
cmake_minimum_required(VERSION 3.5)

# Set project name (= project root folder name).
set(E_REPO "bluetree")
set(E_PROJECT "egui")
project(${E_PROJECT})

# include build information common to all projects.
include(../../eosal/osbuild/cmakedefs/eosal-defs.txt)

# Set path to where to keep libraries.
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY $ENV{E_LIBOUT})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY $ENV{E_LIBOUT})

# Set path to source files.
set(BACKBONE "$ENV{E_ROOT}/${E_REPO}/${E_PROJECT}/code")
set(GRAPHICS "$ENV{E_ROOT}/${E_REPO}/${E_PROJECT}/glfw_opengl3")

# Add iocom and pins library include paths.
include_directories("$ENV{E_ROOT}/iocom")
include_directories("$ENV{E_ROOT}/${E_REPO}/eobjects")
include_directories("$ENV{E_ROOT}/${E_REPO}/econnect")
include_directories("$ENV{E_ROOT}/${E_REPO}/${E_PROJECT}")
include_directories("$ENV{E_ROOT}/${E_REPO}/${E_PROJECT}/code/imgui")
include_directories("$ENV{E_ROOT}/${E_REPO}/${E_PROJECT}/code/imgui_libs/styleserializer")

# Add main headers.
set(MAIN_HEADERS "$ENV{E_ROOT}/${E_REPO}/${E_PROJECT}/egui.h")

# List source and header files to compile into library.
file(GLOB_RECURSE BB_SOURCES "${BACKBONE}/*/*.cpp")
file(GLOB_RECURSE BB_HEADERS "${BACKBONE}/*/*.h")
file(GLOB_RECURSE IM_SOURCES "${GRAPHICS}/*/*.cpp" "${GRAPHICS}/*/*.c")
file(GLOB_RECURSE IM_HEADERS "${GRAPHICS}/*/*.h")

# Generate the static library
add_library(${E_PROJECT}${E_POSTFIX} STATIC ${MAIN_HEADERS} ${BB_HEADERS} ${IM_HEADERS} ${BB_SOURCES} ${IM_SOURCES})
