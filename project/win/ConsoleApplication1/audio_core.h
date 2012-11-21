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

/**
 * \brief class aims to describe the format used to initialize a strem, or check if the device supports it
 * it would be best to use user defined types, compatible with MKS system and user literals
 */
template<typename sample_data_type_t,bit_order bit_order_t >
class audio_format{
public:
    audio_format():mBitOrder(bit_order_t){}
    double mSampleRate;
    unsigned mChannelCount;
    typedef sample_data_type_t sample_data_type;
    bit_order mBitOrder;
};



/**
 * \brief describes an audio device
 * can be open in exclusive of shared mode (shared mode only accepted now)
 * - A device is not necessarly initializable. For that it requires to have a proper id 
 * which mean it is associated to a device of the system
 * - Once the device as a proper id it is "initializable"
 * - You can then initialize it by providing an audio format, or use the default init
 * - Once initialized you can try to open the stream
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
    template <typename audio_format_t>
    bool initialize(const audio_format_t & format);
    bool initialize();
    void start();
    void stop();
    bool is_active(){return mActive;}
    void activate();
    bool is_initialized()const {return pDeviceHandle!=nullptr && pAudioClient!=nullptr;}
private:
    audio_device(const audio_device&);
    native_handle_type pDeviceHandle;
    IAudioClient * pAudioClient;
    bool mActive;
    WAVEFORMATEX * pFormat;
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