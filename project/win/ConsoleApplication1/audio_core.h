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

class audio_device_info{
public:
    audio_device_info(const std::wstring & inName):mName(inName){}

    std::wstring mName;
private:
    audio_device_info();
};

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
	audio_device(native_handle_type handle=nullptr):pAudioClient(nullptr),mActive(false),pFormat(nullptr),pDeviceHandle(handle){}
	audio_device(audio_device && d):pAudioClient(nullptr),pFormat(nullptr),mActive(false),pDeviceHandle(nullptr){
		std::swap(pDeviceHandle,d.pDeviceHandle);
		std::swap(pAudioClient,d.pAudioClient);
		std::swap(pFormat,d.pFormat);
		std::swap(mActive,d.mActive);
	}
	~audio_device();
	native_handle_type native_handle() const{return pDeviceHandle;}
	//! reurn the id of the audio device if initialized, or defaut if not
	id get_id();
	unsigned buffer_size();
	std::chrono::nanoseconds period();
	bool usable(){return id()!=get_id();}
	void initialize();
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