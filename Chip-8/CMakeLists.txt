cmake_minimum_required(VERSION 3.10)
project(VChip8 VERSION 1.0)
include(CMakePrintHelpers)
# Use pkg-config to get SDL2 flags
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)

include_directories(${SDL2_INCLUDE_DIRS})
link_directories(${SDL2_LIBRARY_DIRS})
add_definitions(${SDL2_CFLAGS_OTHER})



set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include the include directory for headers
include_directories(${PROJECT_SOURCE_DIR}/include)

cmake_print_variables(SDL2_INCLUDE_DIRS)

# Add source files from the src directory
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Add executable
add_executable(VChip8 ${SOURCES})

# Link SDL2 library
target_link_libraries(VChip8 ${SDL2_LIBRARIES})

install(TARGETS VChip8 DESTINATION bin)
