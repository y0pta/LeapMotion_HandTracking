#include <iostream>
#include "LeapC.h"
#include "connection/ExampleConnection.h"


/** Callback for when the connection opens. */
static void OnConnect(void){
    std::cout << "Connected.\n";
}

/** Callback for when a device is found. */
static void OnDevice(const LEAP_DEVICE_INFO *props){
    std::cout << "-------------------\n";
    std::cout << "Found device " << props->serial << std::endl;
    std::cout << "HFOV:" << props->h_fov << std::endl;
    std::cout << "VFOV:" << props->v_fov << std::endl;
    std::cout << "range(micrometers):" << props->range << std::endl;
    std::cout << "-------------------\n";
}

/** Callback for when a frame of tracking data is available. */
static void OnFrame(const LEAP_TRACKING_EVENT *frame) {
    std::cout << "Frame " << frame->info.frame_id << ":" << frame->nHands << "hands. \n";
    for (uint32_t h = 0; h < frame->nHands; h++) {
        LEAP_HAND *hand = &frame->pHands[h];
        std::cout << "Hand id: " << hand->id << std::endl;
        std::cout << ((hand->type == eLeapHandType_Left) ? "left" : "right") << std::endl;
        std::cout << "Center position from center ultraleap(cm):"
                  << hand->palm.position.x / 10.0 << " "
                  << hand->palm.position.y / 10.0 << " "
                  << hand->palm.position.z / 10.0 << std::endl;
    }
}

/** Callback for when an image is available. */
static void OnImage(const LEAP_IMAGE_EVENT *imageEvent){
    std::cout << "Received image set for frame " << imageEvent->info.frame_id
                << " with size " << imageEvent->image[0].properties.width << "x"
                << imageEvent->image[0].properties.height << std::endl;
}

int main() {
    //Set callback function pointers
    ConnectionCallbacks.on_connection          = &OnConnect;
    ConnectionCallbacks.on_device_found        = &OnDevice;
    ConnectionCallbacks.on_frame               = &OnFrame;
    ConnectionCallbacks.on_image               = &OnImage;

    LEAP_CONNECTION *connection = OpenConnection();
    LeapSetPolicyFlags(*connection, eLeapPolicyFlag_Images, 0);

    printf("Press Enter to exit program.\n");
    getchar();
    CloseConnection();
    DestroyConnection();
    return 0;
}
