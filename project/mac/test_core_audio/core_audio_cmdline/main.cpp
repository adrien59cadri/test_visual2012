//
//  main.cpp
//  core_audio_cmdline
//
//  Created by developer on 11/22/12.
//
//

#include <iostream>

AudioDeviceID NewGetDefaultInputDevice()
{
    AudioDeviceID theAnswer = 0;
    UInt32 theSize = sizeof(AudioDeviceID);
    AudioObjectPropertyAddress theAddress = { kAudioHardwarePropertyDefaultInputDevice,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMaster };
    
    OSStatus theError = AudioObjectGetPropertyData(kAudioObjectSystemObject,
                                                   &theAddress,
                                                   0,
                                                   NULL,
                                                   &theSize,
                                                   &theAnswer);
    // handle errors
    
    return theAnswer;
}

int main(int argc, const char * argv[])
{

    // insert code here...
    std::cout << "Hello, World!\n";
    return 0;
}

