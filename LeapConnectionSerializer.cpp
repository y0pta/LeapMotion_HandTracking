//
// Created by Liza on 4/27/2023.
//

#include "LeapConnectionSerializer.h"
#include <sstream>
#include <fstream>
#include <iostream>
#include <filesystem>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "Utils.h"

void LeapConnectionSerializer::serialize(const std::string& filename, const LeapConnection& connection){
    std::ofstream file(filename);
    if (file.is_open())
    {
        file << "Total frames:" << connection._trackingData.size() << std::endl;
        file << "Total images:" << connection._imagesData.size() << std::endl;
        file << deviceInfo(connection) << std::endl;
        file << frameInfo(connection) << std::endl;
        file << imageInfo(connection) << std::endl;
        file.close();
    }
    else
        std::cout << "Unable to write in a file" << std::endl;
}

std::string LeapConnectionSerializer::deviceInfo(const LeapConnection& connection) {
    std::ostringstream out;

    auto& devInfo = connection._deviceInfo;

    out << "-------------------\n";
    out << "Found device: " << devInfo.serial << std::endl;
    out << "HFOV:" << devInfo.h_fov << std::endl;
    out << "VFOV:" << devInfo.v_fov << std::endl;
    out << "Max range(micrometers):" << devInfo.range << std::endl;
    out << "Status: " << LeapDeviceStatus::toString(devInfo.status) << std::endl;
    out << "-------------------\n";

    return out.str();
}

std::string LeapConnectionSerializer::imageInfo(const LeapConnection& connection) {
    std::ostringstream out;

    auto& images = connection._imagesData;
    for (auto& image: images) {
        out << "-------------------\n";
        out << "Received image pair for frame " << image.info.frame_id
            << " with size " << image.image[0].properties.width << "x"
            << image.image[0].properties.height << std::endl;
        out << "-------------------\n";
    }
    return out.str();
}

std::string LeapConnectionSerializer::frameInfo(const LeapConnection& connection) {
    std::ostringstream out;

    auto& frames = connection._trackingData;

    for (auto& frame : frames){
        out << "-------------------\n";
        out << "Frame " << frame.info.frame_id << ":" << frame.nHands << "hands. \n";
        for (uint32_t h = 0; h < frame.nHands; h++) {
            LEAP_HAND *hand = &frame.pHands[h];
            out << "Hand id: " << hand->id << " ";
            out << ((hand->type == eLeapHandType_Left) ? "left" : "right") << std::endl;
            out << "Center position from center ultraleap(cm):"
                      << hand->palm.position.x / 10.0 << " "
                      << hand->palm.position.y / 10.0 << " "
                      << hand->palm.position.z / 10.0 << std::endl << std::endl;
        }
        out << "-------------------\n";
    }

    return out.str();
}

void LeapConnectionSerializer::saveImages(const LeapConnection& connection){
    // create directory for storage
    std::filesystem::path folderPath = "images";
    if (!std::filesystem::exists(folderPath))
    {
        if (!std::filesystem::create_directory(folderPath))
            std::cerr << "Error: failed to create directory!" << std::endl;
    }

    // write image data
    int i = 0;
    for(auto& img : connection._imagesData) {
        std::string fname = "images/image" + std::to_string(img.info.frame_id) + ".png";
        stbi_write_png(fname.c_str(),
                       img.image[0].properties.width,
                       img.image[0].properties.height,
                       1,
                       static_cast<void *>(img.image->data),
                       img.image[0].properties.width);
        stbi_write_png(fname.c_str(),
                       img.image[1].properties.width,
                       img.image[1].properties.height,
                       1,
                       static_cast<void *>(img.image->data),
                       img.image[1].properties.width);
        ++i;
    }
}
