//
//  audio_core_mac.cpp
//  test_core_audio
//
//  Created by developer on 11/27/12.
//
//

#include "audio_core_mac.h"

audio_device_collection::audio_device_collection(){
    
	OSStatus result = noErr;
	UInt32 thePropSize;
	AudioDeviceID *theDeviceList = NULL;
	UInt32 theNumDevices = 0;
    // get the device list
    AudioObjectPropertyAddress thePropertyAddress = { kAudioHardwarePropertyDevices, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
    result = AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &thePropertyAddress, 0, NULL, &thePropSize);
    if (result) {
        printf("Error in AudioObjectGetPropertyDataSize: %d\n", result);
    }

    // Find out how many devices are on the system
    theNumDevices = thePropSize / sizeof(AudioDeviceID);
    theDeviceList = (AudioDeviceID*)calloc(theNumDevices, sizeof(AudioDeviceID));
    
    result = AudioObjectGetPropertyData(kAudioObjectSystemObject, &thePropertyAddress, 0, NULL, &thePropSize, theDeviceList);
    if (result) { printf("Error in AudioObjectGetPropertyData: %d\n", result);
    }

    CFStringRef theDeviceName;
    for (UInt32 i=0; i < theNumDevices; i++)
    {
        // get the device name
        thePropSize = sizeof(CFStringRef);
        thePropertyAddress.mSelector = kAudioObjectPropertyName;
        thePropertyAddress.mScope = kAudioObjectPropertyScopeGlobal;
        thePropertyAddress.mElement = kAudioObjectPropertyElementMaster;
        
        result = AudioObjectGetPropertyData(theDeviceList[i], &thePropertyAddress, 0, NULL, &thePropSize, &theDeviceName);
        if (result) { printf("Error in AudioObjectGetPropertyData: %d\n", result);
        }
        
        this->push_back(audio_device(theDeviceList[i]));
        
        CFRelease(theDeviceName);
    }
}

audio_device::audio_device(AudioDeviceID id):mAudioDeviceId(id){
    
}
audio_device::audio_device(audio_device && d){
    std::swap(mAudioDeviceId,d.mAudioDeviceId);
}
audio_device::~audio_device(){
    
}
//native_handle_type native_handle() const{return pDeviceHandle;}
//! reurn the id of the audio device if initialized, or defaut if not
audio_device::id audio_device::get_id(){
    return mAudioDeviceId;
}
//unsigned buffer_size();
//std::chrono::nanoseconds period();

std::string audio_device::name(){
    OSStatus result = noErr;

    CFStringRef theDeviceName;
	UInt32 thePropSize;
    AudioObjectPropertyAddress thePropertyAddress = { kAudioHardwarePropertyDevices, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
    // get the device name
    thePropSize = sizeof(CFStringRef);
    thePropertyAddress.mSelector = kAudioObjectPropertyName;
    thePropertyAddress.mScope = kAudioObjectPropertyScopeGlobal;
    thePropertyAddress.mElement = kAudioObjectPropertyElementMaster;
    
    result =AudioObjectGetPropertyData(mAudioDeviceId, &thePropertyAddress, 0, NULL, &thePropSize, &theDeviceName);
    if (result) { printf("Error in AudioObjectGetPropertyData: %d\n", result);
    }
    AudioObjectGetPropertyData(mAudioDeviceId, &thePropertyAddress, 0, NULL, &thePropSize, &theDeviceName);
    auto charbuffersize = CFStringGetLength(theDeviceName)+1;
    auto charbuffer = new char[charbuffersize];
    auto test= CFStringGetCString(theDeviceName,charbuffer,charbuffersize, kCFStringEncodingASCII);
    if(test==false)
    {
        std::cout<<"aie";
        
    }
    std::string name(charbuffer);
    CFRelease(theDeviceName);
    return name;
}

