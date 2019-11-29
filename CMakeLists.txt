cmake_minimum_required(VERSION 3.12)
project(simplegit)

find_package(OpenSSL REQUIRED)

add_subdirectory(${PROJECT_SOURCE_DIR}/libgit2)
include_directories(${PROJECT_SOURCE_DIR}/libgit2/include)
find_library(git2 libigt2)

file(GLOB_RECURSE SOURCES src/*.*)
add_library(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} git2 OpenSSL::Crypto)

# include simplegit headers
include_directories(src src/common src/common/utils src/builtin)
add_executable(${PROJECT_NAME}_exe main.cpp)
target_link_libraries(${PROJECT_NAME}_exe ${PROJECT_NAME})