#include <iostream>
#include "MouseEmulationApp.h"
#include "spdlog/sinks/rotating_file_sink.h" // support for rotating file logging

int main() {
    auto logger = spdlog::rotating_logger_mt("logger", "log.txt", 1024 * 1024 * 5, 2);
    logger->set_level(spdlog::level::debug);
    MouseEmulationApp app;
    app.parseConfig("config.txt");
    app.run();
    return 0;
}
