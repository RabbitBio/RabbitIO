cmake_minimum_required(VERSION 3.0)

project(RabbitIO)
#openmp
find_package(OpenMP REQUIRED)
if(OPENMP_FOUND)
message("OPENMP FOUND")
set(CMAKE_CXX_COMPILER "/usr/bin/g++")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVERB")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
set(EXECUTABLE_OUTPUT_PATH .)
endif()

#----------IGZIP---------------#
if(DEFINED IGZIP_PREFIX)
	include_directories(${IGZIP_PREFIX}/include) 
	link_directories(${IGZIP_PREFIX}/lib) 
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_IGZIP -lisal")
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_FLAGS "-O3 -g -ffast-math  -I/home/old_home/haoz/tools/isa-l/include -L/home/old_home/haoz/tools/isa-l/lib -lisal ${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS "-O3 -g -ffast-math ${CMAKE_CXX_FLAGS}")

add_subdirectory(io)

#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)

install(FILES "io/Buffer.h"  "io/DataPool.h"   "io/FastxChunk.h"   "io/Formater.h"  "io/Reference.h"  "io/utils.h" "io/Common.h"  "io/DataQueue.h"  "io/FastxStream.h"  "io/Globals.h"   "io/util.h" "io/FileReader.h"
  DESTINATION include
  )
install( FILES "build/io/librabbitio.a"
  DESTINATION lib
  )
#message(${SOURCE_LIST})
#include_directories(./src)
#add_library(libio io )

add_executable (testcount_pe TestCount.cpp) #se fastq 
add_executable (testcount_se TestCount_se.cpp) #se fastq 
#add_executable (rabbitstat RabbitStat.cpp) #se fastq 
#add_executable (test Examples.cpp)
#add_executable (RabbitIO ${SOURCE_LIST})

#target_link_libraries(test z)
#target_link_libraries(test rabbitio)
 
target_link_libraries(testcount_pe z)
target_link_libraries(testcount_pe rabbitio)
target_link_libraries(testcount_se z)
target_link_libraries(testcount_se rabbitio)

#target_link_libraries(rabbitstat z)
#target_link_libraries(rabbitstat rabbitio)
