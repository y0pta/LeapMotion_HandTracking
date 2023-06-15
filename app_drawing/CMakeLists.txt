cmake_minimum_required(VERSION 3.16.3)

project(drawing_app)
set(CMAKE_CXX_STANDARD 17)

# SETUP LEAP SDK path
#set(LeapSDK_DIR D:/PROGRAMMING/CLion/leapMotion/LeapSDK/lib/cmake/LeapSDK)
# LIBRARY SETUP: OPENCV
#SET("OpenCV_DIR" "D:/LIBS/opencv-4.x/build") ##-4.x

# SOURCES
set(SOURCES main.cpp
        ../sources/LeapConnection.cpp
        ../sources/LeapConnection.h
        ../sources/LeapConnectionSerializer.cpp
        ../sources/LeapConnectionSerializer.h
        ../sources/Utils.cpp
        ../sources/Utils.h
        TrackingApp.cpp
        TrackingApp.h
        ../sources/CalibrationArea.cpp
        ../sources/CalibrationArea.h
        "../3rdparty/spdlog")

# ADD EXE
add_executable(${PROJECT_NAME} ${SOURCES})

# LIBRARY SETUP: LEAPC
set(ULTRALEAP_PATH_ROOT "$ENV{ProgramFiles}/Ultraleap")

find_package(LeapSDK
        5
        REQUIRED
        PATHS
        "${ULTRALEAP_PATH_ROOT}" )

get_target_property(LEAPC_IMPORTED_CONFIG
        LeapSDK::LeapC
        IMPORTED_CONFIGURATIONS)
get_target_property(LEAPC_SHARED_LIB_PATH
        LeapSDK::LeapC
        IMPORTED_LOCATION_${LEAPC_IMPORTED_CONFIG})

add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${LEAPC_SHARED_LIB_PATH}
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
)

find_package(OpenCV REQUIRED highgui imgcodecs core)

ADD_CUSTOM_COMMAND (
            TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            "${OpenCV_DIR}/bin/${CMAKE_BUILD_TYPE}/opencv_highgui470d.dll"
            "${OpenCV_DIR}/bin/${CMAKE_BUILD_TYPE}/opencv_core470d.dll"
            "${OpenCV_DIR}/bin/${CMAKE_BUILD_TYPE}/opencv_imgcodecs470d.dll"
            "${OpenCV_DIR}/bin/${CMAKE_BUILD_TYPE}/opencv_videoio470d.dll"
            "${OpenCV_DIR}/bin/${CMAKE_BUILD_TYPE}/opencv_imgproc470d.dll"
            "${OpenCV_DIR}/bin/${CMAKE_BUILD_TYPE}/opencv_calib3d470d.dll"
            "${OpenCV_DIR}/bin/${CMAKE_BUILD_TYPE}/opencv_features2d470d.dll"
            "${OpenCV_DIR}/bin/${CMAKE_BUILD_TYPE}/opencv_flann470d.dll"
            "resources/lclick.wav"
            "resources/rclick.wav"
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
)
#------------------------------------
target_include_directories(${PROJECT_NAME} PUBLIC 3rdparty ${OpenCV_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} libLeapConnection
                                        opencv_core
                                        opencv_highgui
                                        opencv_calib3d
                                        Winmm.lib)
