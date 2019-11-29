cmake_minimum_required(VERSION 3.12)
project(simplegit)

add_subdirectory(${PROJECT_SOURCE_DIR}/libgit2)
include_directories(${PROJECT_SOURCE_DIR}/libgit2/include)
find_library(git2 libigt2)

include_directories(/usr/local/Cellar/openssl/1.0.2t/include)
link_directories(/usr/local/Cellar/openssl/1.0.2t/lib)
find_library(crypto libcrypto)

include_directories(
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/common
    ${PROJECT_SOURCE_DIR}/src/common/utils
    ${PROJECT_SOURCE_DIR}/src/common/commands
    ${PROJECT_SOURCE_DIR}/src/builtin
)
file(GLOB_RECURSE SOURCES src/*.*)

add_library(${PROJECT_NAME} ${SOURCES})

add_executable(${PROJECT_NAME}_exe main.cpp
    ${SOURCES})

target_link_libraries(${PROJECT_NAME}_exe git2 crypto)