//
//  audio_core_mac.cpp
//  test_core_audio
//
//  Created by developer on 11/27/12.
//
//

#include "audio_core_mac.h"
#include <memory>

audio_device_collection::audio_device_collection():mInitialized(false){
    
    std::vector<std::string> deviceNames;
    std::vector<audio_device::id> deviceIds;
    bool test = mac_utilities::scan_devices(deviceNames,deviceIds);
    if(!test)
        return ;
    for (auto i=deviceIds.begin();deviceIds.end()!= i;i++) {
        push_back(audio_device(*i));
    }
    
}

 //==============================================================================

std::string mac_utilities::get_device_name(AudioDeviceID id)
{
    AudioObjectPropertyAddress pa;
    pa.mSelector = kAudioDevicePropertyDeviceName;
    pa.mScope = kAudioObjectPropertyScopeWildcard;
    pa.mElement = kAudioObjectPropertyElementMaster;
    UInt32 length = 0;
    OSStatus err=AudioObjectGetPropertyDataSize(id,&pa,0,nullptr,&length);
    if(err!=noErr)
    {
        mac_utilities::print_osstatus_error(err);
    }
    UInt32 size = length +1;//for null terminated
    std::unique_ptr<char> buffer(new char[size]);
    err=AudioObjectGetPropertyData (id, &pa, 0, nullptr, &length, buffer.get());
    if(err!=noErr)
    {
        mac_utilities::print_osstatus_error(err);
    }
    return std::string(buffer.get(),size);

}

bool mac_utilities::scan_devices( std::vector<std::string>& devices_names,std::vector<AudioDeviceID>& devices_ids)
{
    devices_names.clear();
    devices_ids.clear();

    UInt32 size = 0;
    AudioObjectPropertyAddress pa;
    pa.mSelector = kAudioHardwarePropertyDevices;
    pa.mScope = kAudioObjectPropertyScopeWildcard;
    pa.mElement = kAudioObjectPropertyElementMaster;

    OSStatus err=AudioObjectGetPropertyDataSize (kAudioObjectSystemObject, &pa, 0, 0, &size);
    if(err!=noErr)
    {
        mac_utilities::print_osstatus_error(err);
        return false;
    }
    
    int device_nb = size/sizeof(AudioDeviceID);
    std::unique_ptr<AudioDeviceID[]> devs (new AudioDeviceID[device_nb]);

    err=AudioObjectGetPropertyData (kAudioObjectSystemObject, &pa, 0, 0, &size, devs.get());
    if(err!=noErr)
    {
        mac_utilities::print_osstatus_error(err);
        return false;
    }
    

    for(int i= 0;i<device_nb;i++)
    {

        char * buffer = nullptr;
        int length=0;
        std::string dev_name( get_device_name(devs.get()[i]));
        if(!dev_name.empty())
        {
            //aie
        }
        devices_names.push_back(dev_name);
        devices_ids.push_back(devs[i]);
        //get numchannels
    }
    return true;
}


bool mac_utilities::get_device_bufferlist (AudioDeviceID deviceID, AudioObjectPropertyScope scope, AudioBufferList &bufflist)
{
    int total = 0;
    UInt32 size = sizeof(AudioBufferList);

    AudioObjectPropertyAddress pa;
    pa.mSelector = kAudioDevicePropertyStreamConfiguration;
    pa.mScope = scope;
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
    mName = mac_utilities::get_device_name(id);
    mac_utilities::fill_channels_nb_vector(id,mInputBuffersChannelsNb,kAudioObjectPropertyScopeInput);
    mac_utilities::fill_channels_nb_vector(id, mOutputBuffersChannelsNb,kAudioObjectPropertyScopeOutput);
}
bool mac_utilities::fill_channels_nb_vector(AudioDeviceID id,std::vector<UInt32> & bufferchannelsnb,AudioObjectPropertyScope scope)
{
    bufferchannelsnb.clear();
    AudioBufferList bufflist;
    if(!mac_utilities::get_device_bufferlist(id,scope,bufflist))
        return false;
    for (int i=0;i<bufflist.mNumberBuffers;i++)
    {
        AudioBuffer& buf=bufflist.mBuffers[i];
        UInt32 channelsnb=buf.mNumberChannels;
        bufferchannelsnb.push_back(channelsnb);
    }
    return true;
}

   
audio_device::audio_device(audio_device && d){
    std::swap(mId,d.mId);
    std::swap(mName,d.mName);
    std::swap(mOutputBuffersChannelsNb,d.mOutputBuffersChannelsNb);
    std::swap(mInputBuffersChannelsNb,d.mInputBuffersChannelsNb);
    d.unregister_listener_proc();
    register_listener_proc();
}
audio_device::~audio_device(){
    unregister_listener_proc();
    
}


void  audio_device::register_listener_proc()
{
    AudioObjectPropertyAddress pa;
    pa.mSelector = kAudioObjectPropertySelectorWildcard;
    pa.mScope = kAudioObjectPropertyScopeWildcard;
    pa.mElement = kAudioObjectPropertyElementWildcard;
    
    AudioObjectAddPropertyListener (mId, &pa, device_listener_proc, this);
}

void  audio_device::unregister_listener_proc()
{
    AudioObjectPropertyAddress pa;
    pa.mSelector = kAudioObjectPropertySelectorWildcard;
    pa.mScope = kAudioObjectPropertyScopeWildcard;
    pa.mElement = kAudioObjectPropertyElementWildcard;
    
    AudioObjectRemovePropertyListener (mId, &pa, device_listener_proc, this);
    
}
void audio_device::update_infos(){
    
    AudioObjectPropertyAddress pa={0};
    pa.mSelector = kAudioDevicePropertyDeviceIsAlive;
    pa.mScope = kAudioObjectPropertyScopeWildcard;
    pa.mElement = kAudioObjectPropertyElementMaster;
    
    UInt32 isAlive;
    UInt32 size = sizeof (isAlive);
    OSStatus err = AudioObjectGetPropertyData (get_id(), &pa, 0, 0, &size, &isAlive);
    if(err != noErr)
    {
        mac_utilities::print_osstatus_error(err);
    }
    Float64 sr;
    size = sizeof (sr);
    pa.mSelector = kAudioDevicePropertyNominalSampleRate;
    err = AudioObjectGetPropertyData (get_id(), &pa, 0, 0, &size, &sr);
    if(err != noErr)
    {
        mac_utilities::print_osstatus_error(err);
    }
        
    UInt32 framesPerBuf;
    size = sizeof (framesPerBuf);
    pa.mSelector = kAudioDevicePropertyBufferFrameSize;
    err = AudioObjectGetPropertyData (get_id(), &pa, 0, 0, &size, &framesPerBuf);
    if(err != noErr)
    {
        mac_utilities::print_osstatus_error(err);
    }
    
        
    pa.mSelector = kAudioDevicePropertyBufferFrameSizeRange;
    err = AudioObjectGetPropertyDataSize (get_id(), &pa, 0, 0, &size);
    if(err != noErr)
    {
        mac_utilities::print_osstatus_error(err);
    }
    int framesizes_nb = size/sizeof(AudioValueRange);
    std::unique_ptr<AudioValueRange> framesizes( new AudioValueRange[framesizes_nb]);

    err = AudioObjectGetPropertyData (get_id(), &pa, 0, 0, &size, framesizes.get());
    if(err != noErr)
    {
        mac_utilities::print_osstatus_error(err);
    }

    pa.mSelector = kAudioDevicePropertyAvailableNominalSampleRates;
    err = AudioObjectGetPropertyDataSize (get_id(), &pa, 0, 0, &size);
    if(err != noErr)
    {
        mac_utilities::print_osstatus_error(err);
    }
    int samplerates_nb = size/sizeof(AudioValueRange);
    std::unique_ptr<AudioValueRange> samplerates( new AudioValueRange[samplerates_nb]);

    
    err = AudioObjectGetPropertyData (get_id(), &pa, 0, 0, &size, samplerates.get());
    if(err != noErr)
    {
        mac_utilities::print_osstatus_error(err);
    }
    UInt32 inputLatency = 0;
    size = sizeof (inputLatency);
    pa.mSelector = kAudioDevicePropertyLatency;
    pa.mScope = kAudioDevicePropertyScopeInput;
    err = AudioObjectGetPropertyData (get_id(), &pa, 0, 0, &size, &inputLatency);
    if(err != noErr)
    {
        mac_utilities::print_osstatus_error(err);
    }
        
    pa.mScope = kAudioDevicePropertyScopeOutput;
    UInt32 outputLatency = 0;
    size = sizeof (outputLatency);
    pa.mSelector = kAudioDevicePropertyLatency;
    pa.mScope = kAudioDevicePropertyScopeInput;
    err = AudioObjectGetPropertyData (get_id(), &pa, 0, 0, &size, &outputLatency);
    if(err != noErr)
    {
        mac_utilities::print_osstatus_error(err);
    }


    //channels info
    }
OSStatus audio_device::device_listener_proc (AudioDeviceID /*inDevice*/, UInt32 /*inLine*/, const AudioObjectPropertyAddress* pa, void* inClientData)
{
    auto audio_device_ptr = static_cast <audio_device*> (inClientData);
    
    switch (pa->mSelector)
    {
        case kAudioDevicePropertyBufferSize:
        case kAudioDevicePropertyBufferFrameSize:
        case kAudioDevicePropertyNominalSampleRate:
        case kAudioDevicePropertyStreamFormat:
        case kAudioDevicePropertyDeviceIsAlive:
        case kAudioStreamPropertyPhysicalFormat:
            audio_device_ptr->update_infos();
            break;
        case kAudioDevicePropertyBufferSizeRange:
            std::cout<<"kAudioDevicePropertyBufferSizeRange ignored callback"<<std::endl;
            break;
        case kAudioDevicePropertyVolumeScalar:
            std::cout<<"kAudioDevicePropertyVolumeScalar ignored callback"<<std::endl;
            break;
        case kAudioDevicePropertyMute:
            std::cout<<"kAudioDevicePropertyMute ignored callback"<<std::endl;
            break;
        case kAudioDevicePropertyPlayThru:
            std::cout<<"kAudioDevicePropertyPlayThru ignored callback"<<std::endl;
            break;
        case kAudioDevicePropertyDataSource:
            std::cout<<"kAudioDevicePropertyDataSource ignored callback"<<std::endl;
            break;
        case kAudioDevicePropertyDeviceIsRunning:
            std::cout<<"kAudioDevicePropertyDeviceIsRunning ignored callback"<<std::endl;
            break;
        default:
            std::cout<<"ignored callback"<<std::endl;
            break;
    }
    
    return noErr;
}

void  audio_device::init_name(){
    
    assert(valid());
        
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
    mName = std::string(charbuffer);
    delete [] charbuffer;
    CFRelease(theDeviceName);
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

