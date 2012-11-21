#include "audio_core.h"


void process(audio_buffer &buff){
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout<<" 1 sec \n";
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

        std::this_thread::sleep_for(std::chrono::seconds(30));
        it->stop();
    }
    return 0;
}


