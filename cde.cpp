#include "cde.hpp"

//-----------------------------------------------------------------------------
void CDE::run_camera(std::function<void(uint8_t)> process) {
//-----------------------------------------------------------------------------

    set_all_free();

    for ( ;; ) {

        // --- read ---

        const int32_t idx_cam_process = get_free();

        // --- process ---
        
        process(idx_cam_process);
        
        // --- write ---

        const int32_t detect = _Camera2Detect.exchange(idx_cam_process, std::memory_order_release);

               if ( detect == ACTION_FREE ) {   // detect has finished BEFORE we posted a new frame
                                                // BUT was not able to go to sleep
        } else if ( detect == ACTION_SLEEP ) {  frame_ready_camera.Set(); 
        } else if ( detect == ACTION_EXIT )  {  printf( "CAMERA: exit\n" ); break;
        } else if ( detect  < ACTION_MIN 
                 || detect  > ACTION_MAX )   {  printf( "CAMERA: min max\n" );
        } else {
            // there was "work" in the slot
            // it got overwritten by a new frame
            _Free[ detect ].store(true, std::memory_order_release);
        } 
    } 

    //Message( L"CAMERA: %d (skipped %d) = %d\n", n, m, n - m );

    //Sleep( 200 );
    _Camera2Detect = ACTION_EXIT;
    frame_ready_camera.Set();

}
//-----------------------------------------------------------------------------
void CDE::run_detect(std::function<void(uint8_t)> process) {
//-----------------------------------------------------------------------------
    
    for(;;) {

        int32_t idx = _Camera2Detect.exchange(ACTION_FREE, std::memory_order_acquire );

        if ( idx == ACTION_FREE ) {
            // we are FREE and want to go to SLEEP

            if ( std::atomic_compare_exchange_strong_explicit(
                &_Camera2Detect,
                &idx,                       // expected == FREE
                ACTION_SLEEP,               // desired  == SLEEP
                std::memory_order_acq_rel,  // compare success: memory order for read-modify-write
                std::memory_order_acquire   // compare failure: memory order for load               
            )) {
                // FREE --> SLEEP successfull. take a nap...
                frame_ready_camera.WaitOne();         
                continue;   // goto begin of loop and get a new "idx"
            }
            else {
                // _Camera2Detect != FREE
                // idx == new work IDX
                // there was work posted for us during the attempt going to SLEEP
            }
        }

               if ( idx == ACTION_SLEEP ) { printf( "DETECT: idx == ACTION_SLEEP ... should not be possible\n" );
        } else if ( idx == ACTION_EXIT  ) { break;
        } else if ( idx < ACTION_MIN 
                 || idx > ACTION_MAX    ) { printf( "DETECT: too much action: %d\n", idx );
        } else {
           process(idx);
        }

        // --- write --- post idx to encode thread

        const int32_t encode = _Detect2Encode.exchange(idx, std::memory_order_release);

               if ( encode == ACTION_FREE  ) { ; // encode thread was FREE, new work for him
        } else if ( encode == ACTION_SLEEP ) { frame_ready_detect.Set(); 
        } else if ( encode == ACTION_EXIT  ) { printf( "DETECT: ERROR 3\n" );
        } else if ( encode  < ACTION_MIN || encode > ACTION_MAX ) { printf( "PANIC DETECT: <min >max\n" );
        } else {    
            // encode thread missed work-idx
            // mark this work-idx as FREE
            //m += _Data[ action ];
            _Free[ encode ].store(true, std::memory_order_release);
        }
    } 
}
//-----------------------------------------------------------------------------
void CDE::run_encode(std::function<void(uint8_t)> process) {
//-----------------------------------------------------------------------------
    
    for(;;) {

        int32_t idx = _Detect2Encode.exchange(ACTION_FREE, std::memory_order_acquire );

        if ( idx == ACTION_FREE ) {
            // we are FREE and want to go to SLEEP

            if ( std::atomic_compare_exchange_strong_explicit(
                &_Detect2Encode,
                &idx,                       // expected == FREE
                ACTION_SLEEP,               // desired  == SLEEP
                std::memory_order_acq_rel,  // compare success: memory order for read-modify-write
                std::memory_order_acquire   // compare failure: memory order for load               
            )) {
                // FREE --> SLEEP successfull. take a nap...
                frame_ready_detect.WaitOne();         
                continue;   // goto begin of loop and get a new "idx"
            }
            else {
                // != FREE
                // idx == new work IDX
                // there was work posted for us during the attempt going to SLEEP
            }
        }
        //
        // ---
        //
               if ( idx == ACTION_SLEEP ) { printf( "PANIC ENCODE: ACTION_SLEEP ... should not be possible\n" );
        } else if ( idx == ACTION_EXIT  ) { break;
        } else if ( idx  < ACTION_MIN 
                 || idx  > ACTION_MAX   ) { printf( "PANIC ENCODE: <min >max: %d\n", idx );
        } else {
            process(idx);
            _Free[ idx ].store(true, std::memory_order_release);
        }
    }
}
//-------------------------------------------------------------------------------------------------
u_int8_t CDE::get_free() {
//-------------------------------------------------------------------------------------------------

    bool expected = true;
    if ( _Free[0].compare_exchange_strong(expected, false, std::memory_order_acq_rel) ) return 0;
    if ( _Free[1].compare_exchange_strong(expected, false, std::memory_order_acq_rel) ) return 1;
    if ( _Free[2].compare_exchange_strong(expected, false, std::memory_order_acq_rel) ) return 2;
    if ( _Free[3].compare_exchange_strong(expected, false, std::memory_order_acq_rel) ) return 3;
    if ( _Free[4].compare_exchange_strong(expected, false, std::memory_order_acq_rel) ) return 4;

    printf( "GetFree ERROR 1\n" );

    /*
    for (;;) {
        if ( _Free[ 0 ] == 1 ) return 0;
        if ( _Free[ 1 ] == 1 ) return 1;
        if ( _Free[ 2 ] == 1 ) return 2;
        if ( _Free[ 3 ] == 1 ) return 3;
        if ( _Free[ 4 ] == 1 ) return 4; // we need 5!

        printf( "GetFree ERROR 1\n" );
        //Sleep( 10 );
    } */
}
//-------------------------------------------------------------------------------------------------
void CDE::set_all_free() {
//-------------------------------------------------------------------------------------------------

    for (int i=0; i < 5;++i) {
        _Free[i].store(true);    
    }
}