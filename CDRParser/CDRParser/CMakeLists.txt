project(CDRParser)
cmake_minimum_required(VERSION 2.8)
include_directories(~/projects/PGW_aggregation/ASN2/)
include_directories(~/projects/PGW_aggregation/ASN2/skeletons)
include_directories(../../Common)
aux_source_directory(. SRC_LIST)
aux_source_directory(../../ASN2 SRC_LIST)
aux_source_directory(../../ASN2/skeletons SRC_LIST)
aux_source_directory(../../Common SRC_LIST)
#add_definitions(-DEMIT_ASN_DEBUG)


add_executable(${PROJECT_NAME} ${SRC_LIST})


