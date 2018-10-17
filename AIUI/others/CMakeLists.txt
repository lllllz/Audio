cmake_minimum_required(VERSION 2.8.7)
if(POLICY CMP0046)
  cmake_policy(SET CMP0046 NEW)
endif()
if(POLICY CMP0054)
  cmake_policy(SET CMP0054 NEW)
endif()

# ---[ Aduio project
project(Audio C CXX)

# ---[ Flags
if(UNIX OR APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Wall")
endif()

if(USE_libstdcpp)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libstdc++")
  message("-- Warning: forcing libstdc++ (controlled by USE_libstdcpp option in cmake)")
endif()

add_definitions(-g)

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-O3")        ## Optimize
endif() 

# ---[ Includes
set(CURRENT_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(CURRENT_SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(INCLUDE_DIR /usr/local/include)
set(LIBRARY_DIR /usr/local/lib)
include_directories(${CURRENT_INCLUDE_DIR} ${INCLUDE_DIR})

# ---[ Srcs
set(SRC_LIST ${CURRENT_SRC_DIR}/cJSON.c ${CURRENT_SRC_DIR}/uart.c ${CURRENT_SRC_DIR}/audio.cpp)

# ---[ Declare a cpp executable
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
add_executable(Audio ${SRC_LIST}) 

# ---[ Linter target
target_link_libraries(Audio -lm -lzmq -lz -lpthread -lglog)