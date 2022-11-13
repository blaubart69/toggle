#include "ResourcePipeline.hpp"

//-----------------------------------------------------------------------------
void ResourcePipeline::camera_1(std::function<void(uint8_t)> process) {
//-----------------------------------------------------------------------------

    for ( ;; ) {

        // --- read ---

        const int32_t idx_cam_process = get_free();
        if ( idx_cam_process == -1) {
            break;
        }

        // --- process ---
        
        process(idx_cam_process);
        
        // --- write ---

        write(_CameraToDetect, idx_cam_process);

    } 

}

//-----------------------------------------------------------------------------
void ResourcePipeline::detect_2(std::function<void(uint8_t)> process) {
//-----------------------------------------------------------------------------
    
    for (;;) {

        int32_t idx;
        puts("detect read");
        if ( ! read(_CameraToDetect, &idx ) )
        {
            puts("detect exit");
            break;
        }
        else
        {
            printf("detect process %d\n", idx);
            process(idx);
            write(_DetectToEncode, idx);
        }
    } 
}

//-----------------------------------------------------------------------------
void ResourcePipeline::encode_3(std::function<bool(uint8_t)> process) {
//-----------------------------------------------------------------------------

    puts("start encode");

    for (;;)
    {
        int32_t idx;
        if ( ! read(_DetectToEncode, &idx ) )
        {
            break;
        }
        else
        {
            bool likeToExit = process(idx);
            set_free(idx);

            if ( likeToExit )
            {
                break;
            }
        }
    }

}
//-----------------------------------------------------------------------------
void ResourcePipeline::write(Postbox& postbox, int32_t write_idx) {
//-----------------------------------------------------------------------------

    const int32_t before = postbox.exchange(write_idx);

    if ( before == ACTION_FREE  ) 
    { 
        ; // reader thread was FREE, new work for him/her/it
    } 
    else if ( before == ACTION_SLEEP ) 
    { 
        postbox.signal(); 
    } 
    else if ( before == ACTION_EXIT  ) 
    {
        puts( "PIPE-WRITE: ERROR 3" );
    } 
    else if ( before  < ACTION_MIN || before > ACTION_MAX ) 
    { 
        puts( "PIPE-WRITE: <min >max" );
    } 
    else 
    {    
        // reader thread missed work-idx
        // mark this work-idx as FREE
        set_free(before);
    }
}
//-----------------------------------------------------------------------------
bool ResourcePipeline::read(Postbox& postbox, int32_t* new_idx) {
//-----------------------------------------------------------------------------

    int32_t idx;

    for (;;)
    {
        idx = postbox.exchange(ACTION_FREE);

        if ( idx == ACTION_FREE ) {
            // since our last read there is OUR FREE in the postbox.
            // so we go to SLEEP.
            if ( postbox.compare_exchange(idx, ACTION_SLEEP) ) { 
                // FREE --> SLEEP successfull. take a nap...
                postbox.wait_for_signal();         
                continue;   // goto begin of loop and get a new "idx"
            }
            else {
                // idx == new work IDX
                // there was work posted for us during the attempt going to SLEEP
                break;
            }
        }
        else
        {
            break;
        }
    }

    if ( idx == ACTION_SLEEP )                     
    { 
        puts( "PIPE-READ: idx == ACTION_SLEEP ... should not be possible" );
    } 
    else if ( idx == ACTION_EXIT  )                     
    { 
        ;
    } 
    else if ( idx < ACTION_MIN || idx > ACTION_MAX    ) 
    { 
        printf( "PIPE-READ: too much action: %d\n", idx );
    } 
    else 
    {
        *new_idx = idx;
        return true;
    }

    return false;

}
//-----------------------------------------------------------------------------
int8_t ResourcePipeline::get_free() {
//-----------------------------------------------------------------------------

    for (int8_t i=0; i < 5; ++i) {
        bool expected = true;
        if ( _Free[i].compare_exchange_strong(
                            expected
                            , false
                            , std::memory_order_acq_rel) ) {
            return i;
        }
    }

    puts( "GetFree ERROR -1" );
    return -1;

}
//-----------------------------------------------------------------------------
void ResourcePipeline::set_free(int8_t idx) {
//-----------------------------------------------------------------------------

    printf("set_free: %d\n", idx);
    _Free[ idx ].store(true, std::memory_order_release);

}
//-----------------------------------------------------------------------------
void ResourcePipeline::set_all_free() {
//-----------------------------------------------------------------------------

    for (int i=0; i < 5;++i) {
        _Free[i].store(true);    
    }

}
//-----------------------------------------------------------------------------
ResourcePipeline::ResourcePipeline()
//-----------------------------------------------------------------------------
{
    set_all_free();
}