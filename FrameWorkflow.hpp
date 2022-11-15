#pragma once

#include <atomic>
#include <functional>

#include "auto_reset_event.hpp"
#include "Postbox.hpp"

class FrameWorkflow
{
public:

    void camera_1(std::function<void(uint8_t)> process);
    void detect_2(std::function<void(uint8_t)> process);
    void encode_3(std::function<bool(uint8_t)> process);

    FrameWorkflow();

private:

    #define ACTION_FREE  -1
    #define ACTION_SLEEP -2
    #define ACTION_EXIT  -3

    #define ACTION_MIN   -3
    #define ACTION_MAX    4

    Postbox _CameraToDetect{ ACTION_FREE };
    Postbox _DetectToEncode{ ACTION_FREE };

    std::atomic_bool    _Free[ 5 ];

    bool read (Postbox& postbox, int32_t* idx);
    void write(Postbox& postbox, int32_t write_idx);

    int8_t get_free();
    void   set_free(int8_t idx);
    void   set_all_free();
};