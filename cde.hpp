#include <functional>
#include <atomic>
#include <thread>

#include "auto_reset_event.hpp"

class CDE {
public:

    void start(
         std::function<void(uint8_t)> cbCamera
        ,std::function<void(uint8_t)> cbDetect
        ,std::function<void(uint8_t)> cbEncode );

    CDE() {
        set_all_free();
    }
    
private:

    int8_t  get_free();
    void    set_all_free();

    
    void thread_camera(std::function<void(uint8_t)> process);
    void thread_detect(std::function<void(uint8_t)> process);
    void thread_encode(std::function<void(uint8_t)> process);

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

    std::thread _threads[3];

    //volatile long _Data[ 5 ];
};