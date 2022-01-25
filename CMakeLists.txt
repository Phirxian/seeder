cmake_minimum_required(VERSION 3.0)
project(Seeder)

SET(CMAKE_CXX_FLAGS_RELEASE "-O6 -ffast-math -Os")
SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -g2 -pg -fno-inline-functions -fno-inline ")
SET(CMAKE_C_FLAGS_DEBUG "-O0 -g2 -pg -fno-inline-functions -fno-inline")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(CMAKE_USE_PTHREADS_INIT true)
set(CMAKE_USE_PTHREADS_INIT ON)

###############################################################################
################################ dependencies #################################
###############################################################################

if( NOT cmake_build_type_tolower STREQUAL "release" )
  add_definitions(-DDEBUG)
endif()

include(CheckCXXCompilerFlag)
if (NOT MSVC)
  CHECK_CXX_COMPILER_FLAG(-std=c++11 COMPILER_SUPPORTS_CXX11)
  CHECK_CXX_COMPILER_FLAG(-std=c++0x COMPILER_SUPPORTS_CXX0X)
  if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
  else()
	message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
  endif()
endif()

string(LENGTH "${CMAKE_SOURCE_DIR}/" SOURCE_PATH_SIZE)
add_definitions("-DSOURCE_PATH_SIZE=${SOURCE_PATH_SIZE}")

find_package(Threads REQUIRED)

include_directories("src/")

###############################################################################

file(GLOB_RECURSE SRC src/*.cpp src/*.c src/*.h)
add_library(seeder STATIC ${SRC})

add_executable(chat-client examples/chat-client.cpp)
add_executable(chat-server examples/chat-server.cpp)
add_executable(test-http examples/test-http.cpp)

target_link_libraries(chat-client -lstdc++fs -lpthread -lcurl -lncurses seeder)
target_link_libraries(chat-server -lstdc++fs -lpthread -lcurl seeder)
target_link_libraries(test-http -lstdc++fs -lpthread -lcurl seeder)