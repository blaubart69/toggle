#include <functional>
#include <atomic>

#include "auto_reset_event.hpp"

class CDE {
public:

    void run_camera(std::function<void(uint8_t)> process);
    void run_detect(std::function<void(uint8_t)> process);
    
private:

    uint8_t get_free();
    void    set_all_free();

    #define ACTION_FREE  -1
    #define ACTION_SLEEP -2
    #define ACTION_EXIT  -3

    #define ACTION_MIN   -3
    #define ACTION_MAX    4

    std::atomic_bool    _Free[ 5 ];
    std::atomic_int32_t _Camera2Detect{ACTION_FREE};
    std::atomic_int32_t _Detect2Encode{ACTION_FREE};

    AutoResetEvent frame_ready_camera;
    AutoResetEvent frame_ready_detect;

    //volatile long _Data[ 5 ];
};