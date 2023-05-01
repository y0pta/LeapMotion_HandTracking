cmake_minimum_required(VERSION 3.16.3)

project(leapc_test)
set(CMAKE_CXX_STANDARD 17)

# SETUP PATHES
set(LeapSDK_DIR D:/PROGRAMMING/CLion/leapMotion/LeapSDK/lib/cmake/LeapSDK)
add_subdirectory(connection)

# SOURCES
set(SOURCES main.cpp
        LeapConnection.cpp
        LeapConnection.h
        LeapConnectionSerializer.cpp
        LeapConnectionSerializer.h Utils.cpp Utils.h CalibrationApp.cpp CalibrationApp.h CalibData.cpp CalibData.h TrackingApp.cpp TrackingApp.h)

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

# LIBRARY SETUP: OPENCV
SET("OpenCV_DIR" "D:/LIBS/opencv-4.x/build") ##-4.x
find_package(OpenCV REQUIRED highgui imgcodecs core)

ADD_CUSTOM_COMMAND (
            TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            "${OpenCV_DIR}/bin/Debug/opencv_highgui470d.dll"
            "${OpenCV_DIR}/bin/Debug/opencv_core470d.dll"
            "${OpenCV_DIR}/bin/Debug/opencv_imgcodecs470d.dll"
            "${OpenCV_DIR}/bin/Debug/opencv_videoio470d.dll"
            "${OpenCV_DIR}/bin/Debug/opencv_imgproc470d.dll"
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
)
#------------------------------------
target_include_directories(${PROJECT_NAME} PUBLIC 3rdparty ${OpenCV_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} libExampleConnection opencv_core opencv_highgui)
