project(pgw-to-kafka)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++11 -Wall")
cmake_minimum_required(VERSION 2.8)
include_directories(~/projects/PGW_aggregation/ASN/)
include_directories(~/projects/PGW_aggregation/ASN/skeletons)
include_directories(~/projects/PGW_aggregation/Common)
include_directories(~/projects/Common)
include_directories(/usr/include/boost)
include_directories(/usr/local/include/librdkafka)
# include_directories(/home/oleg/librdkafka/librdkafka/src)
include_directories(/usr/local/include/avro)

aux_source_directory(. SRC_LIST)
aux_source_directory(../ASN SRC_LIST)
aux_source_directory(../ASN/skeletons SRC_LIST)
aux_source_directory(../Common SRC_LIST)
aux_source_directory(../../Common SRC_LIST)
#add_definitions(-DEMIT_ASN_DEBUG)

link_directories(/usr/local/lib)

add_executable(${PROJECT_NAME} ${SRC_LIST})

target_link_libraries (${PROJECT_NAME} libboost_system.so)
target_link_libraries (${PROJECT_NAME} libboost_filesystem.so)
target_link_libraries (${PROJECT_NAME} librdkafka++.so)
target_link_libraries (${PROJECT_NAME} libavrocpp.so)


