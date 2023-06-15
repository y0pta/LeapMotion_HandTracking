cmake_minimum_required(VERSION 3.16.3)

project(libLeapConnection VERSION "1.0.0" LANGUAGES C)

set(ULTRALEAP_PATH_ROOT "$ENV{ProgramFiles}/Ultraleap")

find_package(LeapSDK 5 REQUIRED)

add_library(${PROJECT_NAME} STATIC "ExampleConnection.c")

target_include_directories(${PROJECT_NAME}
        PUBLIC
        .)
#target_include_directories(${PROJECT_NAME} PUBLIC ${LeapSDK_INCLUDE})

target_link_libraries(${PROJECT_NAME} PUBLIC LeapSDK::LeapC)

