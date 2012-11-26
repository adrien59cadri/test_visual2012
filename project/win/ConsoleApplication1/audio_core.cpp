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
        
        AUDCLNT_SHAREMODE mode  = AUDCLNT_SHAREMODE_SHARED;

        //fill format ?


        

        const IID IID_IAudioClient = __uuidof(IAudioClient);

        HRESULT hr = pDeviceHandle->Activate(IID_IAudioClient,CLSCTX_ALL,nullptr,(void**) &pAudioClient);
        if(hr!=ERROR_SUCCESS || pAudioClient == nullptr)
        {
            windows_helper::getLastErrorMessage();
        }

//what is this for?
        WAVEFORMATEX  *pmixerformat = nullptr;

        hr = pAudioClient->GetMixFormat(&pmixerformat);

        if(hr!=S_OK|| pmixerformat == nullptr)
        {
            windows_helper::getLastErrorMessage();
        }


        CoTaskMemFree(pmixerformat);
        //end

        WAVEFORMATEX requestedformat ;
        //memcpy(&requestedformat,pmixerformat,sizeof(requestedformat));
        
        requestedformat.nChannels = format.mChannelCount;
        requestedformat.nSamplesPerSec = (int)( format.mSampleRate);
        requestedformat.wBitsPerSample = format.sample_size() * 8;
        requestedformat.nAvgBytesPerSec = format.mChannelCount * format.sample_size() * format.mSampleRate;
        requestedformat.wFormatTag=WAVE_FORMAT_PCM;
        requestedformat.nBlockAlign=format.mChannelCount*format.sample_size();
        requestedformat.cbSize=0;//ignored because WAVE_FORMAT_PCM
        
        WAVEFORMATEX * pacceptedformat = nullptr;
        WAVEFORMATEX * pnearestformat = nullptr;//should perhaps be freed
        WAVEFORMATEX**ppnearestformat = nullptr;
        if(mode == AUDCLNT_SHAREMODE_EXCLUSIVE)
        {
            ppnearestformat = nullptr;

        }else{
            ppnearestformat = &pnearestformat;
        }
        hr = pAudioClient->IsFormatSupported(mode,&requestedformat,ppnearestformat);
        if(hr == S_OK)
        {
            //format accepté
            pacceptedformat  = &requestedformat;
        }
        else if(hr == S_FALSE)
        {
            //may happen only in shared mode as pnearest is null in exclusive
            //ok, use pnearest
            pacceptedformat = pnearestformat;
        }
        else if(hr==AUDCLNT_E_UNSUPPORTED_FORMAT)
        {
            //ok but requested format is unsupported in exclusive mode
            if(mode == AUDCLNT_SHAREMODE_EXCLUSIVE)
            {
                //ouch, mode rejected ... no closest solution ...
            }
            mode = AUDCLNT_SHAREMODE_SHARED;
            pacceptedformat = &requestedformat;
        }else
        {
            //ouch here we failed
            if(hr == AUDCLNT_E_DEVICE_INVALIDATED)
            {
                //device not valid anymore
            }else if(hr==AUDCLNT_E_SERVICE_NOT_RUNNING)
            {
                //?
            }else if(hr=E_INVALIDARG)
            {
                //mode is not shared and not exclusive
            }else{
                //invalid call
                //pointers to struct are 0
            }
        }
        
        LPCGUID audio_session_guid = nullptr;
        REFERENCE_TIME minimum_100_ns = 10;//1ms
        hr=pAudioClient->Initialize(
            mode,//chose shared mode or exclusive to have exclusive access to the endpoint
            0,//stream flags
            minimum_100_ns,//buffer requested to the endpoint in exclusive mode, or to the audio engine in shared
            0,//because in shared mode, in exclusive this sets the periodicity for the endpoint
            pacceptedformat,//pointer to the filled valid format
            audio_session_guid//set to null this ptr indicates that we don't want wasapi to use session info
            //sessions
            //http://msdn.microsoft.com/en-us/library/windows/desktop/dd370796(v=vs.85).aspx
            );
        if(hr!=S_OK){

            switch(hr)
            {
            case AUDCLNT_E_ALREADY_INITIALIZED : break;
            case AUDCLNT_E_WRONG_ENDPOINT_TYPE : break;
            case AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED : break;
            case AUDCLNT_E_BUFFER_SIZE_ERROR : break;
            case AUDCLNT_E_CPUUSAGE_EXCEEDED : break;
            case AUDCLNT_E_DEVICE_INVALIDATED : break;
            case AUDCLNT_E_DEVICE_IN_USE : break;
            case AUDCLNT_E_ENDPOINT_CREATE_FAILED : break;
            case AUDCLNT_E_INVALID_DEVICE_PERIOD : break;
            case AUDCLNT_E_UNSUPPORTED_FORMAT : break;
            case AUDCLNT_E_EXCLUSIVE_MODE_NOT_ALLOWED : break;
            case AUDCLNT_E_BUFDURATION_PERIOD_NOT_EQUAL : break;//AUDCLNT_STREAMFLAGS_EVENTCALLBACK flag is set but parameters hnsBufferDuration and hnsPeriodicity are not equal.
            case AUDCLNT_E_SERVICE_NOT_RUNNING : break;
            case E_POINTER : break; //pformatisnull
            case E_INVALIDARG : break;
            case E_OUTOFMEMORY : break;
            default :
                //unknown
                break;
            }




        }
        if(pnearestformat!=nullptr)
            CoTaskMemFree(pnearestformat);

        return true;
    }
    unsigned audio_device::buffer_size(){
        if(!is_initialized())
            return 0;
        unsigned buffersize = 0;
        HRESULT hr = pAudioClient->GetBufferSize(&buffersize);
        if(hr!=S_OK)
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
        if(hr!=S_OK)
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
    if(hr!=S_OK || pRenderClient==nullptr)
    {
        windows_helper::getLastErrorMessage();
            return;
    }
    UINT32 bufferFrameCount=0;

    // Get the actual size of the allocated buffer.
    hr = pAudioClient->GetBufferSize(&bufferFrameCount);
    if(hr!=S_OK)
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
        if(hr!=S_OK)
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
    if(hr!=S_OK)
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
        if(hr!=S_OK || device_id == nullptr){
            windows_helper::getLastErrorMessage();
        }
        return id( device_id);
        CoTaskMemFree(device_id);

    }
    std::wstring audio_device::name(){
        if(!initializable())
            return std::wstring();
        IPropertyStore *pProps=nullptr;
        HRESULT hr = this->pDeviceHandle->OpenPropertyStore(
                          STGM_READ, &pProps);
        if(hr!=S_OK || pProps == nullptr)
        {
            windows_helper::getLastErrorMessage();
        }

        PROPVARIANT varName;
        // Initialize container for property value.
        PropVariantInit(&varName);

        // Get the endpoint's friendly-name property.
        hr = pProps->GetValue(
                       PKEY_Device_FriendlyName, &varName);
        if(hr!=S_OK && hr != INPLACE_S_TRUNCATED)
        {
            windows_helper::getLastErrorMessage();
        }

        return std::wstring(varName.pwszVal);
    }
    audio_device::~audio_device(){
        WIN_SAFE_RELEASE(pAudioClient);
        WIN_SAFE_RELEASE(pDeviceHandle);
    }

