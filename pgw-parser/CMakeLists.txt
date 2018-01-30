project(pgw-parser)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++11 -Wall")
#add_definitions(-DEMIT_ASN_DEBUG=1)
cmake_minimum_required(VERSION 2.8)
include_directories(~/projects/PGW-aggregator/Ericsson_ASN_R13/)
#include_directories(~/projects/PGW-aggregator/ASN/skeletons)
include_directories(~/projects/PGW-aggregator/Common)
include_directories(/usr/include/boost)

aux_source_directory(. SRC_LIST)
aux_source_directory(../Ericsson_ASN_R13 SRC_LIST)
#aux_source_directory(../ASN/skeletons SRC_LIST)
aux_source_directory(../Common SRC_LIST)

add_executable(${PROJECT_NAME} ${SRC_LIST})

target_link_libraries (${PROJECT_NAME} libboost_system.so)
target_link_libraries (${PROJECT_NAME} libboost_filesystem.so)
