#include "audio_core.h"


int main(int argc, char* argv[])
{
    windows_helper::scanAudioEndpoints();

    audio_system as;
    for(int i=0;i<as.count();i++)
        std::cout<<as.getDeviceInfoAt(i).mName.data()<<std::endl;
	return 0;
}

