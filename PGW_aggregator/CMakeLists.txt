project(PGW_aggregator)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
cmake_minimum_required(VERSION 2.8)
aux_source_directory(~/projects/PGW_aggregation SRC_LIST)
include_directories(~/projects/PGW_aggregation/ASN/)
include_directories(~/projects/PGW_aggregation/ASN/skeletons)
include_directories(/usr/lib/oracle/12.1/client64/include)
include_directories(/usr/include/boost)


aux_source_directory(. SRC_LIST)
aux_source_directory(../ASN SRC_LIST)
aux_source_directory(../ASN/skeletons SRC_LIST)
#add_definitions(-DEMIT_ASN_DEBUG)

# set the path to the library folder
link_directories(/usr/lib/oracle/11.2/client64/lib)

add_executable(${PROJECT_NAME} ${SRC_LIST})

# link the libraries to the executable
target_link_libraries (${PROJECT_NAME} libclntsh.so)
target_link_libraries (${PROJECT_NAME} libboost_system.so)
target_link_libraries (${PROJECT_NAME} libboost_filesystem.so)


