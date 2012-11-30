#pragma once

#ifdef __APPLE__
#include <CoreAudio/CoreAudio.h>


#else
#error // not supported
#endif


#include <iostream>
#include <vector>
#include <chrono>
#include <functional>
#include <array>
#include <future>
#include <atomic>


namespace mac_utilities{
    void print_osstatus_error(OSStatus error);
    const char * osstatus_error_msg(OSStatus error);
    std::string get_device_name(AudioDeviceID id);
    bool scan_devices(std::vector<std::string>& devices_names,
                      std::vector<AudioDeviceID>& devices_ids);
    bool get_device_bufferlist (AudioDeviceID deviceID, AudioObjectPropertyScope scope, AudioBufferList &bufflist);
    bool fill_channels_nb_vector(AudioDeviceID deviceID,std::vector<UInt32> & bufferchannelsnb,AudioObjectPropertyScope scope);
    bool fill_streams_vector(AudioDeviceID id,std::vector<AudioStreamID> & streams_,AudioObjectPropertyScope scope);

}

enum class audio_direction{
    eInput,
    eAll,
    eOutput
};


enum class bit_order{
    big,
    little
};

enum class audio_sample_data_type{
    eInt16,
    eInt32,
    eUInt16,
    eUInt32,
    eFloat32
};

/**
 * \brief class aims to describe the format used to initialize a strem, or check if the device supports it
 * it would be best to use user defined types, compatible with MKS system and user literals
 */
class audio_format {
public:
    size_t sample_size()const{
        switch(mSampleDataType){
            case audio_sample_data_type::eFloat32:
            case audio_sample_data_type::eInt32:
            case audio_sample_data_type::eUInt32:
                return sizeof(uint32_t);
            case audio_sample_data_type::eInt16:
            case audio_sample_data_type::eUInt16:
                return sizeof(uint16_t);
            default:
                assert(false);
        }
    }
    double mSampleRate;
    unsigned mChannelCount;
    audio_sample_data_type mSampleDataType;
    bit_order mBitOrder;
};

/**
 * \brief buffer passed to the callback
 * i don't like the void * we loose the strong type safety here ...
 * an idea could be to have a template <typename data_type>, but then we have to check the consistency of format and data_type...
 */

class audio_buffer{
public:
    audio_buffer(const audio_format& format,void *data,size_t length)
    :mFormat(format),mData(data),mSize(length){}
    size_t size()const{return mSize;}
    
    
    size_t mSize;
    void * mData;
    audio_format mFormat;
};

/**
 * the callback is a function object that will be called by the device to fill the audio_buffer
 */
typedef std::function<void(audio_buffer&)> audio_callback;
/**
 * \brief describes an audio device
 * can be open in exclusive of shared mode (shared mode only accepted now)
 * - A device is not necessarly initializable. For that it requires to have a proper id
 * which mean it is associated to a device of the system
 * - Once the device as a proper id it is "initializable"
 * - You can then initialize it by providing an audio format, or use the default init
 * - Once initialized you can try to open a stream and provide a callback to call
 */
class audio_device{
public:
    
    typedef AudioDeviceID id;


    audio_device(AudioDeviceID id);
    audio_device(audio_device && d);
    ~audio_device();
    //native_handle_type native_handle() const{return pDeviceHandle;}
    //! reurn the id of the audio device if initialized, or defaut if not
    id get_id() const{return mId;}
    //unsigned buffer_size();
    //std::chrono::nanoseconds period();
	bool	valid() const { return mId != kAudioDeviceUnknown; }
    std::string name()const {return mName;}
    
    

    //bool initialize(const audio_format & format);
    bool initialize();
    
    bool set_callback(const audio_callback & inCallback);
    
    bool start();
    bool stop();
    
    //bool is_initialized()const {return pDeviceHandle!=nullptr ;}
private:
    static OSStatus ioproc(AudioDeviceID inDevice, const AudioTimeStamp * inNow
                    , const AudioBufferList* inInputData, const AudioTimeStamp* inInputTime, AudioBufferList * outOutData, const AudioTimeStamp * inOutputTime, void * inClientData);
    audio_device(const audio_device&);
    //native_handle_type pDeviceHandle;
    //retrives the name during ctor
    void init_name();
    bool mActive;
    std::string mName;
    audio_format mFormat;
    audio_callback mCallback;
    std::future<void> mFuture;
    //std::atomic_bool mRunProcess;
    bool mDeviceModeIsExclusive;
    AudioDeviceIOProcID mIOProcID;
	AudioDeviceID					mId;
    
    //!listener to get changes on the devices
    
    void register_listener_proc();
    bool set_sample_rate(Float64 samplerate);
    bool set_buffer_size(UInt32 buffer_size);
    bool get_sample_rate(Float64& samplerate);
    bool get_buffer_size(UInt32 &buffer_size);
    void unregister_listener_proc();
    void update_infos();
    static OSStatus device_listener_proc (AudioDeviceID /*inDevice*/, UInt32 /*inLine*/, const AudioObjectPropertyAddress* pa, void* inClientData);
    static OSStatus system_listener_proc (AudioDeviceID /*inDevice*/, UInt32 /*inLine*/, const AudioObjectPropertyAddress* pa, void* inClientData);
    std::vector<UInt32> mInputBuffersChannelsNb,mOutputBuffersChannelsNb;
    bool set_stream_format(AudioStreamID sid, AudioStreamBasicDescription & descr, AudioObjectPropertyScope scope);

    Float64 mCurrentSampleRate;
    UInt32 mCurrentBufferSize;
    std::vector<AudioStreamID> mOutputStreams,mInputStreams;

};


//depends on the API we use, for now only one API windows : MMDEVICE
class audio_device_collection : std::vector<audio_device>{
public:
    typedef std::vector<audio_device>::iterator iterator;
    iterator begin(){return std::vector<audio_device>::begin();}
    iterator end(){return std::vector<audio_device>::end();}
    audio_device & at(size_type pos){return std::vector<audio_device>::at(pos);}
    audio_device_collection();
private:
    //see http://msdn.microsoft.com/en-us/library/windows/desktop/ms680582%28v=vs.85%29.aspx
    bool mInitialized;
    
    
    
};