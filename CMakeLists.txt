cmake_minimum_required(VERSION 3.16.3)

project(leapc_test)
set(CMAKE_CXX_STANDARD 17)

set(LeapSDK_DIR D:/PROGRAMMING/CLion/leapMotion/LeapSDK/lib/cmake/LeapSDK)
set(LibEC_INCLUDE D:/PROGRAMMING/CLion/leapMotion/LeapSDK/samples)
set(LibEC_SOURCE D:/PROGRAMMING/CLion/leapMotion/LeapSDK/samples/ExampleConnection.c)

message(STATUS ${LeapSDK_DIR} )
set(ULTRALEAP_PATH_ROOT "$ENV{ProgramFiles}/Ultraleap")

find_package(LeapSDK
        5
        REQUIRED
        PATHS
        "${ULTRALEAP_PATH_ROOT}" )

# EXE
add_executable(${PROJECT_NAME} main.cpp)

get_target_property(LEAPC_IMPORTED_CONFIG
        LeapSDK::LeapC
        IMPORTED_CONFIGURATIONS)
get_target_property(LEAPC_SHARED_LIB_PATH
        LeapSDK::LeapC
        IMPORTED_LOCATION_${LEAPC_IMPORTED_CONFIG})

add_custom_command(
        TARGET
            ${PROJECT_NAME}
        POST_BUILD
        COMMAND
            ${CMAKE_COMMAND} -E copy
            ${LEAPC_SHARED_LIB_PATH}
            $<TARGET_FILE_DIR:${PROJECT_NAME}>)

add_library(
        libExampleConnection
        #OBJECT
        connection/ExampleConnection.c)
target_include_directories(
        libExampleConnection
        PUBLIC
       ${CMAKE_CURRENT_SOURCE_DIR}/connection)
target_link_libraries(
        libExampleConnection
        PUBLIC
        LeapSDK::LeapC)


#set_property(TARGET libExampleConnection PROPERTY
#        IMPORTED_OBJECTS ${CMAKE_SOURCE_DIR}/connections/ExampleConnection.lib)

#add_library(
#        libExampleConnection
#        SHARED
#        IMPORTED
#)
#set_target_properties(
#        libExampleConnection
#        PROPERTIES
#        IMPORTED_LOCATION connection/libExampleConnection.lib
#)
#------------------------------------
#target_include_directories(${PROJECT_NAME} PUBLIC connection)
target_link_libraries(${PROJECT_NAME} PRIVATE libExampleConnection )
