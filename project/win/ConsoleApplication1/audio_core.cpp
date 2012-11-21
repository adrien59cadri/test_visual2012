#include "audio_core.h"


#pragma once

#ifdef _WIN32    
#include <windows.h>
#include <strsafe.h>
#include <mmdeviceapi.h>
#include <Functiondiscoverykeys_devpkey.h>
#else
#error // not supported
#endif



namespace windows_helper{
#define WIN_SAFE_RELEASE(punk){ \
    if((punk)!=nullptr){(punk)->Release();(punk) = nullptr; }}

    void getLastErrorMessage() 
    { 
        // Retrieve the system error message for the last-error code

        LPVOID lpMsgBuf;
        DWORD dw = GetLastError(); 

        FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | 
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            dw,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR) &lpMsgBuf,
            0, NULL );
        std::cout<<lpMsgBuf;
        LocalFree(lpMsgBuf);
    }
    bool scanAudioEndpoints(){
        HRESULT hr= CoInitialize(nullptr);
        if(hr!=ERROR_SUCCESS){
            return false;
        }
        //create instance
        IMMDeviceEnumerator * pEnumerator=nullptr;
        const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
        const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
        hr = CoCreateInstance(
         CLSID_MMDeviceEnumerator, NULL,
         CLSCTX_ALL, IID_IMMDeviceEnumerator,
         (void**)&pEnumerator);

        if(hr!=ERROR_SUCCESS|| pEnumerator==nullptr){
            return false;
        }

        EDataFlow audio_direction = eAll;//or eCapture, or eRender, or eAll
        DWORD device_state_mask = DEVICE_STATE_ACTIVE; // or DEVICE_STATE_ACTIVE or DEVICE_STATE_DISABLED or DEVICE_STATE_NOTPRESENT or DEVICE_STATE_UNPLUGGED or everything DEVICE_STATEMASK_ALL.
        IMMDeviceCollection * pDeviceCollection=nullptr;
        hr = pEnumerator->EnumAudioEndpoints(audio_direction,device_state_mask,&pDeviceCollection);
        if(hr!=ERROR_SUCCESS || pDeviceCollection==nullptr){
            return false;
        }

        unsigned count;
        hr=pDeviceCollection->GetCount(&count);
        if(hr!=ERROR_SUCCESS){
            return false;
        }

        for(unsigned i=0;i<count;i++){
            IMMDevice * pDevice=nullptr;
            // Get pointer to endpoint number i.
            hr = pDeviceCollection->Item(i, &pDevice);
            if(hr!=ERROR_SUCCESS || pDevice == nullptr){
                return false;
            }
                    // Get the endpoint ID string.
            LPWSTR  endpointidstring=nullptr;
            hr = pDevice->GetId(&endpointidstring);
            if(hr!=ERROR_SUCCESS || endpointidstring == nullptr){
                return false;
            }
            IPropertyStore *pProps=nullptr;
            hr = pDevice->OpenPropertyStore(
                              STGM_READ, &pProps);
            if(hr!=ERROR_SUCCESS || pProps == nullptr){
                return false;
            }

            PROPVARIANT varName;
            // Initialize container for property value.
            PropVariantInit(&varName);

            // Get the endpoint's friendly-name property.
            hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
            if(hr!=ERROR_SUCCESS){
                return false;
            }

            // Print endpoint friendly name and endpoint ID.
            printf("Endpoint %d: \"%S\" (%S)\n",
                   i, varName.pwszVal, endpointidstring);

            CoTaskMemFree(endpointidstring);
            PropVariantClear(&varName);
            WIN_SAFE_RELEASE(pProps)
            WIN_SAFE_RELEASE(pDevice);
        }


        //clean
        WIN_SAFE_RELEASE( pDeviceCollection);
        WIN_SAFE_RELEASE( pEnumerator);
        return true;
    }
}


    audio_device_collection::audio_device_collection(){scan();}

    
    //see http://msdn.microsoft.com/en-us/library/windows/desktop/ms680582%28v=vs.85%29.aspx
    void audio_device_collection::scan(){
        mInitialized = false;
        clear();
        HRESULT hr= CoInitialize(nullptr);
        //here hr could indicate that init is ok or that it was already initialized or a thread context changed ...

        //create instance
        IMMDeviceEnumerator * pEnumerator=nullptr;
        const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
        const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
        hr = CoCreateInstance(
         CLSID_MMDeviceEnumerator, NULL,
         CLSCTX_ALL, IID_IMMDeviceEnumerator,
         (void**)&pEnumerator);

        if(hr!=ERROR_SUCCESS|| pEnumerator==nullptr){
            windows_helper::getLastErrorMessage();
        }

        EDataFlow audio_direction = eAll;//or eCapture, or eRender, or eAll
        DWORD device_state_mask = DEVICE_STATE_ACTIVE; // or DEVICE_STATE_ACTIVE or DEVICE_STATE_DISABLED or DEVICE_STATE_NOTPRESENT or DEVICE_STATE_UNPLUGGED or everything DEVICE_STATEMASK_ALL.
        IMMDeviceCollection * pDeviceCollection=nullptr;
        hr = pEnumerator->EnumAudioEndpoints(audio_direction,device_state_mask,&pDeviceCollection);
        if(hr!=ERROR_SUCCESS || pDeviceCollection==nullptr){
            windows_helper::getLastErrorMessage();
        }

        unsigned count;
        hr=pDeviceCollection->GetCount(&count);
        if(hr!=ERROR_SUCCESS){
            windows_helper::getLastErrorMessage();
        }

        for(unsigned i=0;i<count;i++){
            IMMDevice * pDevice=nullptr;
            // Get pointer to endpoint number i.
            hr = pDeviceCollection->Item(i, &pDevice);
            if(hr!=ERROR_SUCCESS || pDevice == nullptr){
                windows_helper::getLastErrorMessage();
            }
            push_back(audio_device(pDevice));
        }


        //clean
        WIN_SAFE_RELEASE( pDeviceCollection);
        WIN_SAFE_RELEASE( pEnumerator);
        mInitialized = true;
    }

    audio_device::audio_device(native_handle_type handle):pAudioClient(nullptr),mActive(false),pFormat(nullptr),pDeviceHandle(handle){}
    audio_device::audio_device(audio_device && d):pAudioClient(nullptr),pFormat(nullptr),mActive(false),pDeviceHandle(nullptr){
        std::swap(pDeviceHandle,d.pDeviceHandle);
        std::swap(pAudioClient,d.pAudioClient);
        std::swap(pFormat,d.pFormat);
        std::swap(mActive,d.mActive);
    }
    
    void audio_device::initialize(){
        if(!initializable())
            return;

        const IID IID_IAudioClient = __uuidof(IAudioClient);

        HRESULT hr = pDeviceHandle->Activate(IID_IAudioClient,CLSCTX_ALL,nullptr,(void**) &pAudioClient);
        if(hr!=ERROR_SUCCESS || pAudioClient == nullptr)
        {
            windows_helper::getLastErrorMessage();
        }
        hr = pAudioClient->GetMixFormat(&pFormat);
        if(hr!=ERROR_SUCCESS|| pFormat == nullptr)
        {
            windows_helper::getLastErrorMessage();
        }

    }
    unsigned audio_device::buffer_size(){
        if(!is_active())
            return 0;
        unsigned buffersize = 0;
        HRESULT hr = pAudioClient->GetBufferSize(&buffersize);
        if(hr!=ERROR_SUCCESS)
        {
            windows_helper::getLastErrorMessage();
            

        }
        return buffersize;
    }
    std::chrono::nanoseconds audio_device::period(){
        if(!is_active())
            return std::chrono::nanoseconds(0);
        REFERENCE_TIME default_period = 0;
        REFERENCE_TIME min_period = 0;
        HRESULT hr = pAudioClient->GetDevicePeriod(&default_period, &min_period);
        if(hr!=ERROR_SUCCESS)
        {
            windows_helper::getLastErrorMessage();
            
        }
        return std::chrono::nanoseconds(default_period*100);
    }
    void audio_device::start(){
        if(!is_active())
            return ;
        HRESULT hr = pAudioClient->Start();
        if(hr!=ERROR_SUCCESS)
        {
            windows_helper::getLastErrorMessage();
            
        }
        return ;
    }
    void audio_device::stop(){
        if(!is_active())
            return ;
        HRESULT hr = pAudioClient->Stop();
        if(hr!=ERROR_SUCCESS)
        {
            windows_helper::getLastErrorMessage();
            
        }
        return ;
    }
    audio_device::id audio_device::get_id(){
        if(pDeviceHandle==nullptr)
            return id();
        // Get the endpoint ID string.
        LPWSTR  device_id=nullptr;
        HRESULT hr = pDeviceHandle->GetId(&device_id);
        if(hr!=ERROR_SUCCESS || device_id == nullptr){
            windows_helper::getLastErrorMessage();
        }
   //         IPropertyStore *pProps=nullptr;
   //         hr = pDevice->OpenPropertyStore(
   //                           STGM_READ, &pProps);
   //         if(hr!=ERROR_SUCCESS || pProps == nullptr){
            //	windows_helper::getLastErrorMessage();
   //         }

   //         PROPVARIANT varName;
   //         // Initialize container for property value.
   //         PropVariantInit(&varName);

   //         // Get the endpoint's friendly-name property.
   //         hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
   //         if(hr!=ERROR_SUCCESS){
            //	windows_helper::getLastErrorMessage();
            //}
        
//            PropVariantClear(&varName);
 //           WIN_SAFE_RELEASE(pProps)
        return id( device_id);
        CoTaskMemFree(device_id);

    }

    audio_device::~audio_device(){
        WIN_SAFE_RELEASE(pAudioClient);
        CoTaskMemFree(pFormat);
        WIN_SAFE_RELEASE(pDeviceHandle);
    }

    void audio_device::activate(){
        if(is_active())
            return;
        if(!is_initialized())
            return;
        LPCGUID audio_session_guid = nullptr;
        REFERENCE_TIME minimum_100_ns = 10;//1ms
        HRESULT hr=pAudioClient->Initialize(
            AUDCLNT_SHAREMODE_SHARED,//chose shared mode or exclusive to have exclusive access to the endpoint
            0,//stream flags
            minimum_100_ns,//buffer requested to the endpoint in exclusive mode, or to the audio engine in shared
            0,//because in shared mode, in exclusive this sets the periodicity for the endpoint
            pFormat,
            audio_session_guid//set to null this ptr indicates that we don't want wasapi to use session info
            //sessions
            //http://msdn.microsoft.com/en-us/library/windows/desktop/dd370796(v=vs.85).aspx
            );
        if(hr!=ERROR_SUCCESS){
            windows_helper::getLastErrorMessage();
        }
        mActive =true;
    }