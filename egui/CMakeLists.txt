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

# Add iocom and pins library include paths.
include_directories("$ENV{E_ROOT}/iocom")
include_directories("$ENV{E_ROOT}/${E_REPO}/eobjects")
include_directories("$ENV{E_ROOT}/${E_REPO}/${E_PROJECT}")

# Add main headers.
set(MAIN_HEADERS "$ENV{E_ROOT}/${E_REPO}/${E_PROJECT}/${E_PROJECT}.h")

message(${MAIN_HEADERS})

# Add source files common to all operating systems.
file(GLOB BB_CM_SOURCES "${BACKBONE}/*/*.cpp")
file(GLOB BB_CM_HEADERS "${BACKBONE}/*/*.h")
# file(GLOB EX_CM_SOURCES "${EXTENSIONS}/*/*.cpp")
# file(GLOB EX_CM_HEADERS "${EXTENSIONS}/*/*.h")

# Generate the static library
add_library(${E_PROJECT}${E_POSTFIX} STATIC ${MAIN_HEADERS} ${BB_CM_HEADERS} ${BB_CM_SOURCES})
# add_library(${E_PROJECT}${E_POSTFIX} STATIC ${MAIN_HEADERS} ${BB_CM_HEADERS} ${EX_CM_HEADERS} ${BB_CM_SOURCES} ${EX_CM_SOURCES})
