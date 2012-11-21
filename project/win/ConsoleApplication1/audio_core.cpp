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

    audio_device::audio_device(native_handle_type handle):pAudioClient(nullptr),mActive(false),pDeviceHandle(handle){
        mRunProcess.exchange(false);
    }
    audio_device::audio_device(audio_device && d):pAudioClient(nullptr),mActive(false),pDeviceHandle(nullptr){
        std::swap(pDeviceHandle,d.pDeviceHandle);
        std::swap(pAudioClient,d.pAudioClient);
        std::swap(mFormat,d.mFormat);
        std::swap(mActive,d.mActive);
    }
    
    bool audio_device::initialize(){
        audio_format format;
        format.mChannelCount = 2;
        format.mSampleRate = 44100.;
        format.mBitOrder = bit_order::little;
        format.mSampleDataType = audio_sample_data_type::eInt16;
        return initialize(format);
    }
    bool audio_device::initialize(const audio_format& format){
        if(!initializable())
            return false;
        mFormat = format;

        //fill format ?


        WAVEFORMATEX * pFormat;


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
        
        LPCGUID audio_session_guid = nullptr;
        REFERENCE_TIME minimum_100_ns = 10;//1ms
        hr=pAudioClient->Initialize(
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
        
        CoTaskMemFree(pFormat);
        return true;
    }
    unsigned audio_device::buffer_size(){
        if(!is_initialized())
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
        if(!is_initialized())
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
    
    bool audio_device::set_callback(const audio_callback& callback){
        if(!is_initialized())
            return false;
        mCallback = callback;
        return true;
    }
    void audio_device::start(){


        if(!is_initialized())
            return;
        HRESULT hr = pAudioClient->Start();
        if(hr!=ERROR_SUCCESS)
        {
            windows_helper::getLastErrorMessage();
            
        }



        mFuture = std::async([this](){internal_process();});
    }
void audio_device::internal_process(){
            
    const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);
    IAudioRenderClient *pRenderClient = nullptr;
    HRESULT hr = pAudioClient->GetService(
                        IID_IAudioRenderClient,
                        (void**)&pRenderClient);
    if(hr!=ERROR_SUCCESS)
    {
        windows_helper::getLastErrorMessage();
            
    }
    UINT32 bufferFrameCount=0;

    // Get the actual size of the allocated buffer.
    hr = pAudioClient->GetBufferSize(&bufferFrameCount);
    if(hr!=ERROR_SUCCESS)
    {
        windows_helper::getLastErrorMessage();
            
    }
    auto period_ = period();

    DWORD flags = 0;
    BYTE * pData=nullptr;
    mRunProcess.exchange(true);
    while (flags != AUDCLNT_BUFFERFLAGS_SILENT && mRunProcess)
    {
        UINT32 numFramesPadding=0;
        // Sleep for half the buffer duration.
        //Sleep((DWORD)(hnsActualDuration/REFTIMES_PER_MILLISEC/2));
        std::this_thread::sleep_for(period_/2);
        // See how much buffer space is available.
        hr = pAudioClient->GetCurrentPadding(&numFramesPadding);
        if(hr!=ERROR_SUCCESS)
        {
            windows_helper::getLastErrorMessage();
            
        }

        int numFramesAvailable = bufferFrameCount - numFramesPadding;

        // Grab all the available space in the shared buffer.
        hr = pRenderClient->GetBuffer(numFramesAvailable, &pData);
        if(hr!=ERROR_SUCCESS)
        {
            windows_helper::getLastErrorMessage();
            
        }
        audio_buffer buffer(mFormat,pData,numFramesAvailable);
        mCallback(buffer);

                

        hr = pRenderClient->ReleaseBuffer(numFramesAvailable, flags);
        if(hr!=ERROR_SUCCESS)
        {
            windows_helper::getLastErrorMessage();
            
        }
    }


    hr = pAudioClient->Stop();
    if(hr!=ERROR_SUCCESS)
    {
        windows_helper::getLastErrorMessage();
            
    }
    return ;
}
            
    void audio_device::stop(){
        mRunProcess.exchange(false);
        mFuture.wait();
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
        WIN_SAFE_RELEASE(pDeviceHandle);
    }

