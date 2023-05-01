#include <iostream>

#include <chrono>
#include <thread>
#include "TrackingApp.h"
#include <mutex>
#include <stdlib.h>

int main() {
    TrackingApp app;
    app.run();
    return 0;
}
