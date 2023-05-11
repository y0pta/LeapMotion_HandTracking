#include <iostream>

#include <chrono>
#include <thread>
#include "TrackingApp.h"
#include "CalibrationApp.h"
#include <mutex>
#include <stdlib.h>

int main() {
    CalibrationApp app;
    app.run();
    return 0;
}
