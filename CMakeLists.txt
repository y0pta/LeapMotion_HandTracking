cmake_minimum_required(VERSION 3.16.3)

project(leapc_test)
set(CMAKE_CXX_STANDARD 17)

set(LeapSDK_DIR D:/PROGRAMMING/CLion/leapMotion/LeapSDK/lib/cmake/LeapSDK)
message(STATUS ${LeapSDK_DIR} )

add_subdirectory(connection)

set(ULTRALEAP_PATH_ROOT "$ENV{ProgramFiles}/Ultraleap")

find_package(LeapSDK
        5
        REQUIRED
        PATHS
        "${ULTRALEAP_PATH_ROOT}" )

# EXE
add_executable(${PROJECT_NAME} main.cpp
        LeapConnection.cpp
        LeapConnection.h
        LeapConnectionSerializer.cpp
        LeapConnectionSerializer.h Utils.cpp Utils.h)

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
#------------------------------------
target_include_directories(${PROJECT_NAME} PUBLIC 3rdparty)
target_link_libraries(${PROJECT_NAME} PUBLIC libExampleConnection )
