cmake_minimum_required(VERSION 3.16.3)

project(leap_project)
set(CMAKE_CXX_STANDARD 17)

# SETUP LEAP SDK path
set(LeapSDK_DIR "$ENV{ProgramFiles}/Ultraleap/TouchFree/Tracking/LeapSDK/lib/cmake/LeapSDK" CACHE STRING "path for leap sdk")
set(LeapSDK_INCLUDE "$ENV{ProgramFiles}/Ultraleap/TouchFree/Tracking/LeapSDK/include" CACHE STRING "include path for leap sdk")

# LIBRARY SETUP: OPENCV
set(OpenCV_DIR "D:/LIBS/opencv-4.x/build" CACHE STRING "path for opencv")

add_subdirectory(app_calibration)
add_subdirectory(app_tracking)
add_subdirectory(app_drawing)
add_subdirectory(sources/connection)
