# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8 FATAL_ERROR)

project ("PBRLookDev")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

# Enable C++20 for host code
set(CMAKE_CXX_STANDARD 20)

set(EXTERNAL "external")

set(GLFW_ROOT_DIR ${EXTERNAL})
set(GLFW_USE_STATIC_LIBS ON)
find_package(GLFW REQUIRED)

set(GLEW_ROOT_DIR ${EXTERNAL})
set(GLEW_USE_STATIC_LIBS ON)
find_package(GLEW REQUIRED)

find_package(OpenGL REQUIRED)

set(GLM_ROOT_DIR ${EXTERNAL})
find_package(GLM REQUIRED)

# set SRC_ROOT_DIR
set(SRC_ROOT_DIR "src")
# set library
set(LIBRARIES ${GLEW_LIBRARY} ${GLFW_LIBRARY} ${OPENGL_LIBRARY})

# set library
set(INCLUDE_DIR
    ${SRC_ROOT_DIR}
    ${GLM_INCLUDE_DIRS} 
    ${GLEW_INCLUDE_DIR} 
    ${GLFW_INCLUDE_DIR}
)

file(GLOB_RECURSE PBRLookDevHeaders 
    "${SRC_ROOT_DIR}/*.h"
)
file(GLOB_RECURSE PBRLookDevSources 
    "${SRC_ROOT_DIR}/*.cpp"
)

list(SORT PBRLookDevHeaders)
list(SORT PBRLookDevSources)

source_group(Headers FILES ${PBRLookDevHeaders})
source_group(Sources FILES ${PBRLookDevSources})

add_executable(PBRLookDev ${PBRLookDevHeaders} ${PBRLookDevSources})

target_include_directories(PBRLookDev 
    PUBLIC
        ${INCLUDE_DIR}
)

target_link_libraries(PBRLookDev
    PRIVATE
        ${LIBRARIES}
)