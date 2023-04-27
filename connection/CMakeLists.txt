cmake_minimum_required(VERSION 3.16.3)

project(libExampleConnection VERSION "1.0.0" LANGUAGES C)
set(LeapSDK_DIR D:/PROGRAMMING/CLion/leapMotion/LeapSDK/lib/cmake/LeapSDK)
set(ULTRALEAP_PATH_ROOT "$ENV{ProgramFiles}/Ultraleap")

find_package(LeapSDK 5 REQUIRED)


add_library(libExampleConnection STATIC "ExampleConnection.c")

target_include_directories(libExampleConnection
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(libExampleConnection PUBLIC LeapSDK::LeapC)