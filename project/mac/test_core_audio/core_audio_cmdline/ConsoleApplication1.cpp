#include "audio_core_mac.h"
#define _USE_MATH_DEFINES
#include <math.h>
static std::atomic_int count;
static float phase=0.;

void process(audio_buffer &buff){

    std::cout<<std::this_thread::get_id()<<" : "<<count++<<std::endl;
    auto type=buff.mFormat.mSampleDataType;
    float * t=((float*)buff.mData);
    const float f = 220./buff.mFormat.mSampleRate;
    const float tierce = 5.f/4.f;
    const float quinte = 3./2.;
    for(int i=0;i<buff.mSize;i++){
        const float res = sinf(phase);
        t[i*2] = t[i*2+1] = res*.2;//interleaved
        phase+=M_2_PI*f;
        while(phase > M_2_PI)
            phase-=M_2_PI;
    }
}


int main(int argc, char* argv[])
{
//    windows_helper::scanAudioEndpoints();

    audio_device_collection collection;
    int pos=0;
    for(auto it=collection.begin();it!=collection.end();it++){
        std::cout<<"device "<<pos++<<" : ";
        std::cout<<"name "<<it->name().data()<<std::endl;
        std::wcout<<" id "<<it->get_id()<<std::endl;
    }
    std::cin>>pos;
    {
        auto&& device = collection.at(pos);
        std::cout<<device.name().data()<<std::endl;
        std::wcout<<device.get_id()<<std::endl;
//        if(!device.initialize())
//            std::cout<<"echec initialize()"<<std::endl;
//
//        std::wcout<<"buffer size : "<<device.buffer_size()<<" period : "<<device.period().count()<<" ns"<<std::endl;
       device.set_callback(process);
        device.start();
//
//        
        std::cout<<"\nthread id "<<std::this_thread::get_id()<<std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(30));
        device.stop();
    }
    return 0;
}


