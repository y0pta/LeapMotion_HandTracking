//
// Created by Liza on 6/4/2023.
//

#include "MouseEmulationApp.h"
#include "../sources/LeapConnection.h"
#define NOMINMAX
#include <windows.h>
#include <playsoundapi.h>
#include "../sources/Utils.h"
#include "spdlog/spdlog.h"
#include <opencv2/core.hpp>
#include <fstream>

std::unique_ptr<MouseEmulationApp> __app;

/** Match 3D points with 2d display*/
void frameCallback(LeapConnection& con, const LEAP_TRACKING_EVENT *frame){
    if (frame->nHands > 0) {
        for (int i = 0; i < frame->nHands; ++i) {
            auto &hand = frame->pHands[i];

            auto indexFinger = hand.index.distal.next_joint;
            auto middleFinger = hand.middle.distal.next_joint;
            __app->processFingersPosition(indexFinger, middleFinger, frame->info.timestamp);
        }
    }
}

MouseEmulationApp::MouseEmulationApp()
{
    __app = std::unique_ptr<MouseEmulationApp>(this);
    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);
    _recognizer = std::make_unique<GestureRecognizer>(screenW, screenH, "calib.txt");
    int monitorCount = GetSystemMetrics(SM_CMONITORS);
    spdlog::get("logger")->info("System screen W:" + std::to_string(screenW));
    spdlog::get("logger")->info("System screen H:" + std::to_string(screenH));
    spdlog::get("logger")->info("Number of monitors" + std::to_string(monitorCount));
}

MouseEmulationApp::~MouseEmulationApp()
{
    _recognizer.release();
    __app.release();
}

void MouseEmulationApp::run(){
    LeapConnection connection;
    connection.tracking_callback = frameCallback;
    connection.open();
    AppUtils::waitForKeyPress("Press ESC to exit");
}

void MouseEmulationApp::processFingersPosition(LEAP_VECTOR& indexCoords,
                                               LEAP_VECTOR& middleCoords,
                                               int64_t timestamp)
{
    cv::Point3f index(indexCoords.x, indexCoords.y, indexCoords.z);
    cv::Point3f middle(middleCoords.x, middleCoords.y, middleCoords.z);
    auto [type, screenPoint] = _recognizer->recognize(index,middle,timestamp);

    switch (type) {
        case eNone:
            break;
        case eMove:
            SetCursorPos(screenPoint.x , screenPoint.y);
            break;
        case eLeftDown:
            //spdlog::debug("Press ", screenPoint.x, " ", screenPoint.y);
            std::cout << "Press " << screenPoint.x <<  " " << screenPoint.y << std::endl;
            PlaySound(TEXT("lclick.wav"), NULL, SND_FILENAME | SND_ASYNC);
            mouse_event(MOUSEEVENTF_LEFTDOWN,screenPoint.x , screenPoint.y, 0, 0);
            break;
        case eLeftUp:
            //spdlog::debug("Release", screenPoint.x, " ", screenPoint.y);
            std::cout << "Release " << screenPoint.x <<  " " << screenPoint.y << std::endl;
            mouse_event(MOUSEEVENTF_LEFTUP, screenPoint.x , screenPoint.y, 0, 0);
            break;
        case eRightUp:
            //spdlog::debug("Right Release", screenPoint.x, " ", screenPoint.y);
            std::cout << "Right Release " << screenPoint.x <<  " " << screenPoint.y <<  std::endl;
            mouse_event(MOUSEEVENTF_RIGHTUP, screenPoint.x , screenPoint.y, 0, 0);
            break;
        case eRightDown:
            //spdlog::debug("Right Press", screenPoint.x, " ", screenPoint.y);
            std::cout << "Right Press " << screenPoint.x <<  " " << screenPoint.y << std::endl;
            PlaySound(TEXT("rclick.wav"), NULL, SND_FILENAME | SND_ASYNC);
            mouse_event(MOUSEEVENTF_RIGHTDOWN, screenPoint.x ,screenPoint.y, 0, 0);
            break;
        case eDoubleClick:
            //spdlog::debug("Double click", screenPoint.x, " ", screenPoint.y);
            std::cout << "Double click " << screenPoint.x <<  " " << screenPoint.y << std::endl;
            PlaySound(TEXT("lclick.wav"), NULL, SND_FILENAME | SND_ASYNC);
            mouse_event(MOUSEEVENTF_LEFTDOWN, screenPoint.x , screenPoint.y, 0, 0);
            mouse_event(MOUSEEVENTF_LEFTUP, screenPoint.x , screenPoint.y, 0, 0);
            mouse_event(MOUSEEVENTF_LEFTDOWN, screenPoint.x , screenPoint.y, 0, 0);
            mouse_event(MOUSEEVENTF_LEFTUP, screenPoint.x , screenPoint.y, 0, 0);
            break;
        default:
            break;
    }
}

void MouseEmulationApp::parseConfig(const std::string& fname){
    std::unordered_map<std::string, int> config;

    std::ifstream file(fname);
    if (!file.is_open()) {
        std::cout << "Error opening config file: " << fname << std::endl;
        spdlog::get("logger")->error("Error opening config file: " + fname);
    }

    std::string line;
    while (std::getline(file, line)) {
        size_t delimiterPos = line.find('=');
        if (delimiterPos == std::string::npos) {
            std::cout << "Error: Invalid format in config-file. Line: " << line << std::endl;
            spdlog::get("logger")->error("Error: Invalid format in config-file. Line: " + line);
            continue;
        }

        std::string key = line.substr(0, delimiterPos);
        std::string valueStr = line.substr(delimiterPos + 1);
        try {
            int value = std::stoi(valueStr);
            config[key] = value;
        } catch (const std::exception& e) {
            std::cout << "Error parsing value in line: " << line << std::endl;
            spdlog::get("logger")->error("Error parsing value in config-file. Line: " + line);
        }
    }
    file.close();

    // defaults
    // Default values
    std::unordered_map<std::string, int> defaultConfig;
    defaultConfig["interaction_threshold"] = 100;
    defaultConfig["press_threshold"] = 10;
    defaultConfig["interdigital_threshold"] = 20;
    defaultConfig["max_doubleclick_delay"] = 500;

    // Update with default values if not present in the file
    for (const auto& entry : defaultConfig) {
        if (config.find(entry.first) == config.end()) {
            config[entry.first] = entry.second;
        }
    }

    _recognizer->interaction_threshold = config["interaction_threshold"];
    _recognizer->press_threshold = config["press_threshold"];
    _recognizer->interdigital_threshold = config["interdigital_threshold"];
    _recognizer->max_doubleclick_delay = config["max_doubleclick_delay"];

}