#include <iostream>

#include <chrono>
#include <thread>
#include "LeapConnection.h"
#include "LeapConnectionSerializer.h"

int main() {
    LeapConnection connection;
    connection.setDefaultCallbacks();
    connection.open();

    std::this_thread::sleep_for(std::chrono::seconds(2));

    connection.close();
    LeapConnectionSerializer::serialize("info.txt", connection);
    LeapConnectionSerializer::saveImages(connection);
    return 0;
}
