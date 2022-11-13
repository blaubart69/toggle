#include <cstdio>
#include <thread>
#include <curses.h>

//#include "cde.hpp"
#include "ResourcePipeline.hpp"

void camera(uint8_t idx)
{
    printf(">> C: %d\n", idx);
    std::this_thread::sleep_for( std::chrono::milliseconds(300) );
    printf("<< C: %d\n", idx);
}
void detect(uint8_t idx)
{
    printf(">> D: %d\n", idx);
    std::this_thread::sleep_for( std::chrono::milliseconds(600) );
    printf("<< D: %d\n", idx);
}
bool encode(uint8_t idx)
{
    printf(">> E: %d\n", idx);
    std::this_thread::sleep_for( std::chrono::milliseconds(100) );
    printf("<< E: %d\n", idx);
    return true;
}
/*
void run_cde()
{
    CDE cde;

    cde.start(camera, detect, encode);
    std::this_thread::sleep_for( std::chrono::seconds(15) );
}
*/

void run_resource_pipeline()
{
    ResourcePipeline rp;

    std::thread _threads[3];

    //_threads[2] = std::thread( [&] { rp.encode_3(encode); } );
    _threads[1] = std::thread( [&] { rp.detect_2(detect); } );
    _threads[0] = std::thread( [&] { rp.camera_1(camera); } );

    std::this_thread::sleep_for( std::chrono::seconds(60) );

}

int main(int, char**) {
    run_resource_pipeline();
    
}
