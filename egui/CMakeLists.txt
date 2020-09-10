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
# set(EXTENSIONS "$ENV{E_ROOT}/${E_REPO}/${E_PROJECT}/extensions")
set(GRAPHICS "$ENV{E_ROOT}/${E_REPO}/${E_PROJECT}/glfw_opengl3")


# Add iocom and pins library include paths.
include_directories("$ENV{E_ROOT}/iocom")
include_directories("$ENV{E_ROOT}/${E_REPO}/eobjects")
include_directories("$ENV{E_ROOT}/${E_REPO}/${E_PROJECT}")

# Add main headers.
set(MAIN_HEADERS "$ENV{E_ROOT}/${E_REPO}/${E_PROJECT}/eguilib.h")

message(${MAIN_HEADERS})

# List source and header files to compile into library.
file(GLOB BB_SOURCES "${BACKBONE}/*/*.cpp")
file(GLOB BB_HEADERS "${BACKBONE}/*/*.h")
file(GLOB_RECURSE IM_SOURCES "${GRAPHICS}/*/*.cpp" "${GRAPHICS}/*/*.c")
file(GLOB_RECURSE IM_HEADERS "${GRAPHICS}/*/*.h")
# file(GLOB EX_SOURCES "${EXTENSIONS}/*/*.cpp")
# file(GLOB EX_HEADERS "${EXTENSIONS}/*/*.h")

# Generate the static library
add_library(${E_PROJECT}${E_POSTFIX} STATIC ${MAIN_HEADERS} ${BB_HEADERS} ${IM_HEADERS} ${BB_SOURCES} ${IM_SOURCES})
# add_library(${E_PROJECT}${E_POSTFIX} STATIC ${MAIN_HEADERS} ${BB_HEADERS} ${EX_HEADERS} ${BB_SOURCES} ${EX_SOURCES})
