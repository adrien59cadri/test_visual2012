#pragma once

#ifdef _WIN32    
#include <windows.h>
#include <strsafe.h>
#include <Audioclient.h>
#include <mmdeviceapi.h>
#include <Functiondiscoverykeys_devpkey.h>
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

namespace windows_helper{
    bool scanAudioEndpoints();
}
enum class audio_direction{
    eInput,
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

private:
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
    typedef IMMDevice * native_handle_type;
    class id:std::wstring{
    public:
        id():std::wstring(){}
        id(wchar_t*pwstr):std::wstring(pwstr){}
        inline bool operator==(const id& rhs){ return compare(rhs)==0;}
        inline bool operator!=(const id& rhs){ return !operator==(rhs);}
        friend std::wostream& operator<<(std::wostream& out, const audio_device::id& id){
            return out<<id.data();	
        } 
    };
    audio_device(native_handle_type handle=nullptr);
    audio_device(audio_device && d);
    ~audio_device();
    native_handle_type native_handle() const{return pDeviceHandle;}
    //! reurn the id of the audio device if initialized, or defaut if not
    id get_id();
    unsigned buffer_size();
    std::chrono::nanoseconds period();
    bool initializable(){return id()!=get_id();}
	std::wstring name();


    
    bool initialize(const audio_format & format);
    bool initialize();

    bool set_callback(const audio_callback & inCallback);
    
    void start();
    void stop();
    void close();

    bool is_initialized()const {return pDeviceHandle!=nullptr && pAudioClient!=nullptr;}
private:
    void internal_process();
    audio_device(const audio_device&);
    native_handle_type pDeviceHandle;
    IAudioClient * pAudioClient;
    bool mActive;
    audio_format mFormat;
    audio_callback mCallback;
    std::future<void> mFuture;
    std::atomic_bool mRunProcess;
};


//depends on the API we use, for now only one API windows : MMDEVICE
class audio_device_collection : std::vector<audio_device>{
public:
    typedef std::vector<audio_device>::iterator iterator;
    iterator begin(){return std::vector<audio_device>::begin();}
    iterator end(){return std::vector<audio_device>::end();}

    audio_device_collection();
private:
    //see http://msdn.microsoft.com/en-us/library/windows/desktop/ms680582%28v=vs.85%29.aspx
    void scan();
    bool mInitialized;

};