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

 //==============================================================================

    bool get_device_name(AudioDeviceID id,char *& buffer, int & size)
    {
        AudioObjectPropertyAddress pa;
        pa.mSelector = kAudioDevicePropertyDeviceName;
        pa.mScope = kAudioObjectPropertyScopeWildcard;
        pa.mElement = kAudioObjectPropertyElementMaster;
        int length = 0;
        OSStatus err=AudioObjectGetPropertyDataSize (id, &pa, 0, 0, &length);
        if(err!=noErr)
        {
            mac_utilities::print_osstatus_error(err);
            return false;
        }
        size = length +1;//for null terminated
        buffer = malloc(sizeof(char)*size);
        err=AudioObjectGetPropertyData (id, &pa, 0, 0, &length, buffer);
        if(err!=noErr)
        {
            mac_utilities::print_osstatus_error(err);
            return false;
        }
        

        return true;
    }

    void mac_scan_devices()
    {

        std::vector<std::string> output_devices_names;
        std::vector<std::string> input_devices_names;

        std::vector<AudioDeviceID> output_devices_ids;
        std::vector<AudioDeviceID> input_devices_ids;

        AudioObjectPropertyAddress pa;
        pa.mSelector = kAudioHardwarePropertyDevices;
        pa.mScope = kAudioObjectPropertyScopeWildcard;
        pa.mElement = kAudioObjectPropertyElementMaster;

        OSStatus err=AudioObjectGetPropertyDataSize (kAudioObjectSystemObject, &pa, 0, 0, &size);
        if(err!=noErr)
        {
            mac_utilities::print_osstatus_error(err);
        }
        
        int device_nb = size/sizeof(AudioDeviceID);
        AudioDeviceID * devs = new AudioDeviceID[device_nb];

        err=AudioObjectGetPropertyData (kAudioObjectSystemObject, &pa, 0, 0, &size, devs);
        if(err!=noErr)
        {
            mac_utilities::print_osstatus_error(err);
        }
        

        for(int i= 0;i<dev;i++)
        {

            char * buffer = nullptr;
            int length=0;
            bool test = get_device_name(devs[i],buffer, length);
            if(!test)
            {
                //aie
            }
            std::string dev_name(buffer, length);
            free(buffer);

            //get numchannels
        }
    }


    true get_device_bufferlist (AudioDeviceID deviceID, bool input, AudioBufferList &bufflist)
    {
        int total = 0;
        UInt32 size = sizeof(AudioBufferList);

        AudioObjectPropertyAddress pa;
        pa.mSelector = kAudioDevicePropertyStreamConfiguration;
        pa.mScope = input ? kAudioDevicePropertyScopeInput : kAudioDevicePropertyScopeOutput;
        pa.mElement = kAudioObjectPropertyElementMaster;


        OSStatus err= AudioObjectGetPropertyData(deviceID, &pa, 0, 0, &size,&bufflist) ;
        if(err != noErr)
        {
            mac_utilities::print_osstatus_error(err);
            return false;
        }
        return true;
    }
void mac_utilities::print_osstatus_error(OSStatus error){
    std::cout<<osstatus_error_msg(error)<<std::endl;
}
const char * mac_utilities::osstatus_error_msg(OSStatus error)
{
    
    const char * errorText = nullptr;
    
    switch (error) {
        case kAudioHardwareNoError:
            return nullptr;
        case kAudioHardwareNotRunningError:
            errorText = "Audio Hardware Not Running";
            break;
        case kAudioHardwareUnspecifiedError:
            errorText = "Unspecified Audio Hardware Error";
            break;
        case kAudioHardwareUnknownPropertyError:
            errorText = "Audio Hardware: Unknown Property";
            break;
        case kAudioHardwareBadPropertySizeError:
            errorText = "Audio Hardware: Bad Property Size";
            break;
        case kAudioHardwareIllegalOperationError:
            errorText = "Audio Hardware: Illegal Operation";
            break;
        case kAudioHardwareBadDeviceError:
            errorText = "Audio Hardware: Bad Device";
            break;
        case kAudioHardwareBadStreamError:
            errorText = "Audio Hardware: BadStream";
            break;
        case kAudioHardwareUnsupportedOperationError:
            errorText = "Audio Hardware: Unsupported Operation";
            break;
        case kAudioDeviceUnsupportedFormatError:
            errorText = "Audio Device: Unsupported Format";
            break;
        case kAudioDevicePermissionsError:
            errorText = "Audio Device: Permissions Error";
            break;
        default:
            errorText = "Unknown Error";
    }
    return errorText;
}




audio_device::audio_device(AudioDeviceID id):mId(id){
    register_listener_proc();


        
}
audio_device::audio_device(audio_device && d){
    std::swap(mId,d.mId);
}
audio_device::~audio_device(){
    unregister_listener_proc();
    
}
//native_handle_type native_handle() const{return pDeviceHandle;}
//! reurn the id of the audio device if initialized, or defaut if not

//unsigned buffer_size();
//std::chrono::nanoseconds period();

std::string audio_device::name(){
    OSStatus result = noErr;

    CFStringRef theDeviceName;
	UInt32 thePropSize;
    AudioObjectPropertyAddress thePropertyAddress;
    // get the device name
    thePropSize = sizeof(CFStringRef);
    thePropertyAddress.mSelector = kAudioObjectPropertyName;
    thePropertyAddress.mScope = kAudioObjectPropertyScopeGlobal;
    thePropertyAddress.mElement = kAudioObjectPropertyElementMaster;
    
    result =AudioObjectGetPropertyData(get_id(), &thePropertyAddress, 0, NULL, &thePropSize, &theDeviceName);
    if (result) { printf("Error in AudioObjectGetPropertyData: %d\n", result);
    }
    AudioObjectGetPropertyData(get_id(), &thePropertyAddress, 0, NULL, &thePropSize, &theDeviceName);
    auto charbuffersize = CFStringGetLength(theDeviceName)+1;
    auto charbuffer = new char[charbuffersize];
    auto test= CFStringGetCString(theDeviceName,charbuffer,charbuffersize, kCFStringEncodingASCII);
    if(test==false)
    {
        std::cout<<"aie";
        
    }
    std::string name(charbuffer);
    delete [] charbuffer;
    CFRelease(theDeviceName);
    return name;
}

bool audio_device::initialize(){
    int bytesPerSample = 4;
    AudioStreamBasicDescription stereoStreamFormat = {0};
    //see Core Audio Format Specification
    stereoStreamFormat.mFormatID          = kAudioFormatLinearPCM;
    stereoStreamFormat.mFormatFlags       = kAudioFormatFlagsAudioUnitCanonical;
    stereoStreamFormat.mChannelsPerFrame  = 2;           // 2 indicates stereo
    stereoStreamFormat.mBitsPerChannel    = 8*bytesPerSample;
    stereoStreamFormat.mBytesPerFrame     = stereoStreamFormat.mChannelsPerFrame * stereoStreamFormat.mBitsPerChannel / 8;
    stereoStreamFormat.mFramesPerPacket   = 1;//because l;inear pcm this must be 1
    stereoStreamFormat.mBytesPerPacket    = stereoStreamFormat.mFramesPerPacket*stereoStreamFormat.mBytesPerFrame ;
    stereoStreamFormat.mSampleRate        = 48000.;
    
    
    AudioObjectPropertyAddress theAddress = { kAudioDevicePropertyStreams,
        kAudioDevicePropertyScopeInput,
        kAudioObjectPropertyElementMaster };
    
    // StreamListenerBlock is called whenever the sample rate changes (as well as other format characteristics of the device)
	UInt32 propSize;
	OSStatus err = AudioObjectGetPropertyDataSize(get_id(), &theAddress, 0, NULL, &propSize);
    if (err) fprintf(stderr, "Error %ld returned from AudioObjectGetPropertyDataSize\n", (long)err);
    
	if(!err) {
        
		AudioStreamID *streams = (AudioStreamID*)malloc(propSize);
		err = AudioObjectGetPropertyData(get_id(), &theAddress, 0, NULL, &propSize, streams);
        if (err) fprintf(stderr, "Error %ld returned from AudioObjectGetPropertyData\n", (long)err);
        
		if(!err) {
			UInt32 numStreams = propSize / sizeof(AudioStreamID);
			
            for(UInt32 i=0; i < numStreams; i++) {
				UInt32 isInput;
				propSize = sizeof(UInt32);
                theAddress.mSelector = kAudioStreamPropertyDirection;
                theAddress.mScope = kAudioObjectPropertyScopeGlobal;
				
                err = AudioObjectGetPropertyData(streams[i], &theAddress, 0, NULL, &propSize, &isInput);
                if (err) fprintf(stderr, "Error %ld returned from AudioObjectGetPropertyData\n", (long)err);
                
                std::cout<<"is input "<<isInput<<std::endl;
                
                
                AudioStreamRangedDescription * formats = nullptr;
                
                theAddress.mSelector= kAudioStreamPropertyAvailablePhysicalFormats;
                err = AudioObjectGetPropertyDataSize(streams[i],&theAddress,0,nullptr,&propSize);
                if (err) fprintf(stderr, "Error %ld returned from AudioObjectGetPropertyDataSize\n", (long)err);
                
                int nbformats = propSize/sizeof(AudioStreamRangedDescription);
                formats = new AudioStreamRangedDescription[nbformats];
                err = AudioObjectGetPropertyData(streams[i], &theAddress, 0, NULL, &propSize, streams);
                if (err) fprintf(stderr, "Error %ld returned from AudioObjectGetPropertyData\n", (long)err);
                
                for(int j=0;j<nbformats;j++){
                    formats[j].mFormat;
                }
                
                delete [] formats;
            }
        }
        
        if (NULL != streams) free(streams);
    }
}

bool audio_device::set_callback(const audio_callback & inCallback){
    mCallback = inCallback;
    
    OSStatus theError = AudioDeviceCreateIOProcID(get_id(), audio_device::ioproc, this, &mIOProcID);
}

OSStatus audio_device::ioproc(AudioDeviceID inDevice, const AudioTimeStamp * inNow
                              ,const AudioBufferList* inInputData, const AudioTimeStamp* inInputTime
                              ,AudioBufferList * outOutData, const AudioTimeStamp * inOutputTime, void * inClientData){
    auto device = reinterpret_cast<audio_device*>(inClientData);
    auto buffernb = outOutData->mNumberBuffers;
    auto channelsnb =outOutData->mBuffers[0].mNumberChannels;
    void * data =outOutData->mBuffers[0].mData;
    auto datasize =outOutData->mBuffers[0].mDataByteSize;
    audio_buffer buffer(device->mFormat,data,datasize);
    device->mCallback(buffer);
}
void audio_device::start(){
    auto    theError = AudioDeviceStart(get_id(), mIOProcID);
    
}
void audio_device::stop(){
    auto    theError = AudioDeviceStop(get_id(), mIOProcID);
}

