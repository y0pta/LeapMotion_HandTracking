#include <iostream>
#include "CalibrationConsoleApp.h"
#include "../3rdparty/spdlog/sinks/rotating_file_sink.h" // support for rotating file logging

int main() {
    auto logger = spdlog::rotating_logger_mt("logger", "log.txt", 1024 * 1024 * 5, 2);
    logger->set_level(spdlog::level::debug);
    CalibrationConsoleApp app;
    app.run();
    system("pause");
    return 0;
}
