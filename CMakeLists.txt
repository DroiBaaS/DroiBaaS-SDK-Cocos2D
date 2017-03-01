cmake_minimum_required(VERSION 2.6)

project(coresdk)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
# Enable C++11 under OSX
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

# Coresdk
file(GLOB_RECURSE CPPSOURCES "src/*.cpp")
file(GLOB_RECURSE CSOURCES "src/*.c")
file(GLOB_RECURSE HEADERS "inc/*.h")

# 

include_directories("inc")

add_library(coresdk ${CPPSOURCES} ${CSOURCES} ${HEADERS})
