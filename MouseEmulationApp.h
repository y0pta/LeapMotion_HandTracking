//
// Created by Liza on 6/4/2023.
//

#ifndef LEAPC_TEST_MOUSEEMULATIONAPP_H
#define LEAPC_TEST_MOUSEEMULATIONAPP_H
#include <memory>
#include "LeapC.h"
#include <thread>
#include "GestureRecognizer.h"

class MouseEmulationApp {
    public:
        MouseEmulationApp();
        ~MouseEmulationApp();
        void run();
        void processFingersPosition(LEAP_VECTOR& indexCoords,
                                    LEAP_VECTOR& middleCoords,
                                    int64_t timestamp);
    private:
        std::unique_ptr<GestureRecognizer> _recognizer;
};

#endif //LEAPC_TEST_MOUSEEMULATIONAPP_H
