#include "audio_core.h"


int main(int argc, char* argv[])
{
//    windows_helper::scanAudioEndpoints();

    audio_device_collection collection;
	for(auto it=collection.begin();it!=collection.end();it++)
		std::wcout<<it->get_id()<<std::endl;
	return 0;
}

