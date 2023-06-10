#include <iostream>

#include "CalibrationConsoleApp.h"
#include <stdlib.h>
#include "MouseEmulationApp.h"
#include "spdlog/sinks/rotating_file_sink.h" // support for rotating file logging

int main() {
    auto logger = spdlog::rotating_logger_mt("logger", "log.txt", 1024 * 1024 * 5, 2);
    logger->set_level(spdlog::level::debug);
    //CalibrationConsoleApp app;
    MouseEmulationApp app;
    app.run();
    return 0;
}
