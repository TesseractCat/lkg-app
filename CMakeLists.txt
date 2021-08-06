cmake_minimum_required(VERSION 3.11) # FetchContent is available in 3.11+
project(lkg_app)

# Set this to the minimal version you want to support
# find_package(raylib 3.0 QUIET) # Let CMake search for a raylib-config.cmake
# find_library(raylib 3.0 QUIET) # Let CMake search for a raylib-config.cmake

set(CMAKE_C_STANDARD 11) # Requires C11 standard
set(CMAKE_CXX_STANDARD 17)

# if (NOT raylib_FOUND)
#   include(FetchContent)
# 
#   FetchContent_Declare(
#     raylib
#     #URL https://github.com/raysan5/raylib/archive/d6708de7b0c640b3ca2e667ba8aaec1d70f14845.tar.gz
#     URL https://github.com/raysan5/raylib/archive/refs/heads/master.tar.gz
#   )
# 
#   FetchContent_GetProperties(raylib)
#   if (NOT raylib_POPULATED) # Have we downloaded raylib yet?
#     set(FETCHCONTENT_QUIET NO)
#     FetchContent_Populate(raylib)
# 
#     set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
# 
#     # build raylib
#     add_subdirectory(${raylib_SOURCE_DIR} ${raylib_BINARY_DIR})
# 
#   endif()
# 
# endif()

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} raylib)
target_link_libraries(${PROJECT_NAME} drm)
target_link_libraries(${PROJECT_NAME} EGL)
target_link_libraries(${PROJECT_NAME} GLESv2)
target_link_libraries(${PROJECT_NAME} gbm)
target_link_libraries(${PROJECT_NAME} pthread)
target_link_libraries(${PROJECT_NAME} rt)
target_link_libraries(${PROJECT_NAME} m)
target_link_libraries(${PROJECT_NAME} dl)

# Disable console on windows
# if(MSVC)
#     set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
# endif()
