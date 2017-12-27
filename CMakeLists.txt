cmake_minimum_required(VERSION 2.6)
project (nmeaparser)

#define Sources and headers
set (NMEAP_SRC   src/nmeaparser.cpp
                 src/nmeautils.cpp)
set (NMEAP_INC   inc/nmeaparser.h
                 inc/nmeautils.h)
				 
#include directory
include_directories("inc")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Wno-varargs")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg -O3")


#Check if Boost is available on system
find_package(Boost 1.40.0 COMPONENTS date_time filesystem system)
if (NOT ${Boost_FOUND})
    message( FATAL_ERROR "libboost-all-dev was not found : ${Boost_NOT_FOUND_MESSAGE}" )
else ()
	include_directories(${Boost_INCLUDE_DIRS})
endif(NOT ${Boost_FOUND})

#compiling step
add_executable(nmeaparser ${NMEAP_SRC} ${NMEAP_INC} ${Boost_INCLUDE_DIRS})

#link step
target_link_libraries(nmeaparser ${Boost_LIBRARIES} -lpthread)
