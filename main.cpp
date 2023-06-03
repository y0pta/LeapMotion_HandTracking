#include <iostream>

#include <chrono>
#include <thread>
#include "TrackingApp.h"
#include "CalibrationConsoleApp.h"
#include <mutex>
#include <stdlib.h>
#include <filesystem>
#include <opencv2/highgui.hpp>
#include "LeapConnectionSerializer.h"
#include "fstream"

int main() {
    TrackingApp app;
    app.run();
    return 0;
}
