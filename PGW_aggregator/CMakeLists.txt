project(pgw-aggregator)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++14 -Wall")
cmake_minimum_required(VERSION 2.8)
include_directories(~/projects/PGW-aggregator/PGW_aggregator)
include_directories(~/projects/PGW-aggregator/Ericsson_ASN_R13)
#include_directories(~/projects/PGW-aggregator/ASN/skeletons)
include_directories(~/projects/PGW-aggregator/Common)
include_directories(~/projects/Common)
include_directories($ENV{ORACLE_HOME}/include)
include_directories(/usr/include/boost)

aux_source_directory(. SRC_LIST)
aux_source_directory(../Ericsson_ASN_R13 SRC_LIST)
#aux_source_directory(../ASN/skeletons SRC_LIST)
aux_source_directory(../Common SRC_LIST)
aux_source_directory(../../Common SRC_LIST)

# set the path to the library folder
link_directories($ENV{ORACLE_HOME}/lib)

add_executable(${PROJECT_NAME} ${SRC_LIST})

# link the libraries to the executable
target_link_libraries (${PROJECT_NAME} libclntsh.so)
target_link_libraries (${PROJECT_NAME} libboost_system.so)
target_link_libraries (${PROJECT_NAME} libboost_filesystem.so)


