#include "tobii_device.h"
#include "tobii_api.h"
#include "tobii_headers.h"
#include <thread>
#include <chrono>

using namespace std;
using namespace std::chrono;

namespace
{
    float latest_gaze_point[2] = {0.5f, 0.5f};
    float latest_eyes_point[6] = {0.5f, 0.5f,0.5f, 0.5f,0.5f, 0.5f};

    string device_url = "";

    void url_receiver(const char* url, void*)
    {
        device_url = string(url);
    }

    void gaze_callback(const tobii_gaze_point_t* gaze_point, void*)
    {
        latest_gaze_point[0] = gaze_point->position_xy[0];
        latest_gaze_point[1] = gaze_point->position_xy[1];
    }

    // This is new -------------------------------------------------------------------------
    void gaze_origin_callback( tobii_gaze_origin_t const* gaze_origin, void* user_data ){
        //left eye
        latest_eyes_point[0] = gaze_origin->left_xyz[0];
        latest_eyes_point[1] = gaze_origin->left_xyz[1];
        latest_eyes_point[2] = gaze_origin->left_xyz[2];
        //right eye
        latest_eyes_point[3] = gaze_origin->right_xyz[0];
        latest_eyes_point[4] = gaze_origin->right_xyz[1];
        latest_eyes_point[5] = gaze_origin->right_xyz[2];
    }
    // -------------------------------------------------------------------------------------
}

TobiiDevice::TobiiDevice(TobiiAPI& api) :
    device(nullptr)
{
    int error = tobii_enumerate_local_device_urls(api.get_api(), url_receiver, NULL);
    if (error != TOBII_ERROR_NO_ERROR)
    {
        throw -2;
    }

    error = tobii_device_create(api.get_api(), device_url.c_str(), &device);
    if (error != TOBII_ERROR_NO_ERROR)
    {
        device = nullptr;
        throw -2;
    }

    error = tobii_gaze_point_subscribe(device, gaze_callback, NULL);
    if (error != TOBII_ERROR_NO_ERROR)
    {
        throw -2;
    }

    // This is new-------------------------------------------------------------
    error = tobii_gaze_origin_subscribe(device, gaze_origin_callback, NULL );
    if (error != TOBII_ERROR_NO_ERROR)
    {
        throw -2;
    }
    //-------------------------------------------------------------------------


    thread([this]
    {
        while (true)
        {
            tobii_wait_for_callbacks(NULL, 1, &device);
            tobii_device_process_callbacks(device);
            this_thread::sleep_for(milliseconds(16));
        }
    }).detach();
}

TobiiDevice::~TobiiDevice()
{
    if (device != nullptr)
    {
        tobii_device_destroy(device);
    }
}

tobii_device_t* TobiiDevice::get_device()
{
    return device;
}

float* TobiiDevice::get_latest_gaze_point()
{
    return latest_gaze_point;
}

float* TobiiDevice::get_latest_eyes_point()
{
    return latest_eyes_point;
}