//
// Created by Liza on 4/27/2023.
//

#ifndef LEAPC_TEST_LEAPCONNECTIONSERIALIZER_H
#define LEAPC_TEST_LEAPCONNECTIONSERIALIZER_H
#include "LeapConnection.h"
#include <string>

class LeapConnectionSerializer {
    public:
        LeapConnectionSerializer() {}
        static void serialize(const std::string& filename, const LeapConnection& connection);
        static std::string deviceInfo(const LeapConnection& connection);
        static std::string imageInfo(const LeapConnection& connection);
        static std::string frameInfo(const LeapConnection& connection);
        static void saveImages(const LeapConnection& connection);

};

#endif //LEAPC_TEST_LEAPCONNECTIONSERIALIZER_H
