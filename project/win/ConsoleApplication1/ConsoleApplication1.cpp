#include "audio_core.h"
static std::atomic_int count;

void process(audio_buffer &buff){

    std::cout<<std::this_thread::get_id()<<" : "<<count++<<std::endl;
}


int main(int argc, char* argv[])
{
//    windows_helper::scanAudioEndpoints();

    audio_device_collection collection;
    for(auto it=collection.begin();it!=collection.end();it++){
        std::wcout<<it->get_id()<<std::endl;
    }
    for(auto it=collection.begin();it!=collection.end();it++){
        std::wcout<<it->get_id()<<std::endl;
        if(!it->initialize())
            std::cout<<"echec initialize()"<<std::endl;

        std::wcout<<"buffer size : "<<it->buffer_size()<<" period : "<<it->period().count()<<" ns"<<std::endl;
        it->set_callback(process);
        it->start();


    }
    std::cout<<"\nthread id "<<std::this_thread::get_id()<<std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(30));
    for(auto it=collection.begin();it!=collection.end();it++)
        it->stop();
    return 0;
}


