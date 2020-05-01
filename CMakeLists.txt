cmake_minimum_required(VERSION 3.2.2)
project(s3draw VERSION 1.0.0)

set(CMAKE_C_FLAGS "-Wall")
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}/bin")

find_package(s3map REQUIRED)
find_package(s3dat REQUIRED)
find_package(s3util REQUIRED)
find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)

include_directories(${s3util_INCLUDE_DIRS} ${s3map_INCLUDE_DIRS} ${s3dat_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIRS} ${glfw3_INCLUDE_DIRS})


#s3draw
file(GLOB_RECURSE s3draw_SOURCES "src/*.[ch]")

add_executable(s3draw ${s3draw_SOURCES})
target_link_libraries(s3draw s3dat s3map ${OPENGL_LIBRARIES} glfw m)

