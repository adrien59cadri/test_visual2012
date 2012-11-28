#include "audio_core.h"


#pragma once

#ifdef _WIN32    
#include <windows.h>
#include <avrt.h>
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
        std::cout<<std::endl<<"error msg : "<<lpMsgBuf<<std::endl;
        LocalFree(lpMsgBuf);
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
			LPWSTR _id =nullptr;
			hr = pDevice->GetId(&_id);
			if(hr!=ERROR_SUCCESS)
			{
			}
            push_back(audio_device(_id));
        }


        //clean
        WIN_SAFE_RELEASE( pDeviceCollection);
        WIN_SAFE_RELEASE( pEnumerator);
        mInitialized = true;
    }

    audio_device::audio_device(const audio_device::id& id):pAudioClient(nullptr),mInitialized(false),hEvent(nullptr),mActive(false),pDeviceHandle(nullptr),mId(id){
        mRunProcess.exchange(false);
		
		
		//get the audio device really
		
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
		
		hr = pEnumerator->GetDevice(mId.data(),&	pDeviceHandle);
		if(hr != S_OK)
		{
			if(hr==E_POINTER)std::cout<<"Parameter pwstrId or ppDevice is NULL."<<std::endl;
			else if (hr==E_NOTFOUND)std::cout<<"The device ID does not identify an audio device that is in this system."<<std::endl;
			else if(hr==E_OUTOFMEMORY)std::cout<<"Out of memory."<<std::endl;
			
		}
		
        WIN_SAFE_RELEASE( pDeviceCollection);
        WIN_SAFE_RELEASE( pEnumerator);



    }
    audio_device::audio_device(audio_device && d){
        std::swap(pDeviceHandle,d.pDeviceHandle);
        std::swap(pAudioClient,d.pAudioClient);
        std::swap(mFormat,d.mFormat);
        std::swap(mActive,d.mActive);
		std::swap(mId,d.mId);
		std::swap(mInitialized,d.mInitialized);
		std::swap(mCallback,d.mCallback);
		std::swap(hEvent,d.hEvent);
		std::swap(mDeviceModeIsExclusive, d.mDeviceModeIsExclusive);
		//!note, moving can not be atomic, so this is a non sens if the future is set or is the runprocess flag is set we should not authorize the move
		mRunProcess = d.mRunProcess;
		std::swap(mFuture,d.mFuture);
    }
    
    bool audio_device::initialize(){
        audio_format format;
        format.mChannelCount = 2;
        format.mSampleRate = 44100.;
        format.mBitOrder = bit_order::little;
        format.mSampleDataType = audio_sample_data_type::eFloat32;
        return initialize(format);
    }
    bool audio_device::initialize(const audio_format& format){
		mInitialized = false;

        mFormat = format;   
        REFERENCE_TIME deviceperiod=0;

        AUDCLNT_SHAREMODE mode  = AUDCLNT_SHAREMODE_SHARED;//AUDCLNT_SHAREMODE_EXCLUSIVE;//AUDCLNT_SHAREMODE_SHARED;

        //fill format ?


        

        const IID IID_IAudioClient = __uuidof(IAudioClient);

        HRESULT hr = pDeviceHandle->Activate(IID_IAudioClient,CLSCTX_ALL,nullptr,(void**) &pAudioClient);
        if(hr!=S_OK || pAudioClient == nullptr)
        {
            switch(hr)
			{
				case E_NOINTERFACE: std::cout<<"The object does not support the requested interface type."<<std::endl;break;
				case E_POINTER : std::cout<<"Parameter ppInterface is NULL."<<std::endl;break;
				case E_INVALIDARG : std::cout<<"The pActivationParams parameter must be NULL for the specified interface; or pActivationParams points to invalid data."<<std::endl;break;
				case E_OUTOFMEMORY: std::cout<<"Out of memory."<<std::endl;break;
				case AUDCLNT_E_DEVICE_INVALIDATED : std::cout<<"The user has removed either the audio endpoint device or the adapter device that the endpoint device connects to."<<std::endl;break;
			}
        }

//what is this for?
        WAVEFORMATEX  *pmixerformat = nullptr;

        hr = pAudioClient->GetMixFormat(&pmixerformat);

        if(hr!=S_OK|| pmixerformat == nullptr)
        {
            switch(hr)
			{
				case AUDCLNT_E_DEVICE_INVALIDATED:std::cout<<"The audio endpoint device has been unplugged, or the audio hardware or associated hardware resources have been reconfigured, disabled, removed, or otherwise made unavailable for use."<<std::endl;break;
				case AUDCLNT_E_SERVICE_NOT_RUNNING:std::cout<<"The Windows audio service is not running."<<std::endl;break;
				case E_POINTER:std::cout<<"Parameter ppDeviceFormat is NULL."<<std::endl;break;
				case E_OUTOFMEMORY:std::cout<<"Out of memory."<<std::endl;break;
			}
        }


        CoTaskMemFree(pmixerformat);
        //end
        
        WAVEFORMATEXTENSIBLE requestedformat ;
        //memcpy(&requestedformat,pmixerformat,sizeof(requestedformat));
        
        requestedformat.Format.nChannels = format.mChannelCount;
        requestedformat.Format.nSamplesPerSec = (int)( format.mSampleRate);
        requestedformat.Format.wBitsPerSample = format.sample_size() * 8;
        requestedformat.Format.nAvgBytesPerSec = static_cast<int>(format.mChannelCount * format.sample_size() * format.mSampleRate);
        requestedformat.Format.wFormatTag=WAVE_FORMAT_IEEE_FLOAT;//  format.mSampleDataType==audio_sample_data_type::eFloat32? WAVE_FORMAT_IEEE_FLOAT:WAVE_FORMAT_PCM;
        requestedformat.SubFormat= format.mSampleDataType==audio_sample_data_type::eFloat32? KSDATAFORMAT_SUBTYPE_IEEE_FLOAT:KSDATAFORMAT_SUBTYPE_PCM; 
        requestedformat.Format.nBlockAlign=format.mChannelCount*format.sample_size();
        requestedformat.Format.cbSize=0;//ignored because WAVE_FORMAT_PCM
        requestedformat.dwChannelMask=SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT;
        
        WAVEFORMATEX * pacceptedformat = nullptr;
        WAVEFORMATEX * pnearestformat = nullptr;
        WAVEFORMATEX**ppnearestformat = nullptr;
        if(mode == AUDCLNT_SHAREMODE_EXCLUSIVE)
        {
            ppnearestformat = nullptr;

        }else{
            ppnearestformat = &pnearestformat;
        }
        hr = pAudioClient->IsFormatSupported(mode,&requestedformat.Format,ppnearestformat);


        WAVEFORMATEXTENSIBLE * pnearestextensible = (WAVEFORMATEXTENSIBLE*)(*ppnearestformat);


        if(hr == S_OK)
        {
            //format accepté
            pacceptedformat  = &requestedformat.Format;
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
            pacceptedformat = &requestedformat.Format;
        }else
        {
            switch(hr)
			{
			case E_POINTER:std::cout<<"Parameter pFormat is NULL, or ppClosestMatch is NULL and ShareMode is AUDCLNT_SHAREMODE_SHARED."<<std::endl;break;
			case E_INVALIDARG:std::cout<<"Parameter ShareMode is a value other than AUDCLNT_SHAREMODE_SHARED or AUDCLNT_SHAREMODE_EXCLUSIVE."<<std::endl;break;
			case AUDCLNT_E_DEVICE_INVALIDATED:std::cout<<"The audio endpoint device has been unplugged, or the audio hardware or associated hardware resources have been reconfigured, disabled, removed, or otherwise made unavailable for use."<<std::endl;break;
			case AUDCLNT_E_SERVICE_NOT_RUNNING:std::cout<<"The Windows audio service is not running."<<std::endl;break;
			}
        }
        
        LPCGUID audio_session_guid = nullptr;
        

        //ici pour appeler buffer_size il faut avoir initialisé
        

        hr=pAudioClient->Initialize(
            mode,//chose shared mode or exclusive to have exclusive access to the endpoint
            AUDCLNT_STREAMFLAGS_EVENTCALLBACK ,//stream flags : here we say we will use callback api
            deviceperiod,//buffer requested to the endpoint in exclusive mode, or to the audio engine in shared
            deviceperiod,//because in shared mode, in exclusive this sets the periodicity for the endpoint
            pacceptedformat,//pointer to the filled valid format
            audio_session_guid//set to null this ptr indicates that we don't want wasapi to use session info
            //sessions
            //http://msdn.microsoft.com/en-us/library/windows/desktop/dd370796(v=vs.85).aspx
            );

        
        if(hr!=S_OK){
            if(hr == AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED)
            {
                REFERENCE_TIME hnsRequestedDuration = (REFERENCE_TIME)
                    ((10000.0 * 1000 / mFormat.mSampleRate * buffer_size()) + 0.5);
                pAudioClient->Release();
                pAudioClient=nullptr;
                //reactivate:
                hr = pDeviceHandle->Activate(IID_IAudioClient,CLSCTX_ALL,nullptr,(void**) &pAudioClient);
                if(hr!=ERROR_SUCCESS || pAudioClient == nullptr)
                {
                    windows_helper::getLastErrorMessage();
                }
                hr=pAudioClient->Initialize(
                    mode,//chose shared mode or exclusive to have exclusive access to the endpoint
                    AUDCLNT_STREAMFLAGS_EVENTCALLBACK ,//stream flags : here we say we will use callback api
                    deviceperiod,//buffer requested to the endpoint in exclusive mode, or to the audio engine in shared
                    deviceperiod,//because in shared mode, in exclusive this sets the periodicity for the endpoint
                    pacceptedformat,//pointer to the filled valid format
                    audio_session_guid//set to null this ptr indicates that we don't want wasapi to use session info
                    //sessions
                    //http://msdn.microsoft.com/en-us/library/windows/desktop/dd370796(v=vs.85).aspx
                    );

            }
            if(hr!=S_OK){
                switch(hr)
                {
				case AUDCLNT_E_ALREADY_INITIALIZED:std::cout<<"The IAudioClient object is already initialized."<<std::endl;break;
				case AUDCLNT_E_WRONG_ENDPOINT_TYPE:std::cout<<"The  AUDCLNT_STREAMFLAGS_LOOPBACK flag is set but the endpoint device is a capture device, not a rendering device."<<std::endl;break;
				case AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED:std::cout<<"Note  Applies to Windows 7 and later.\
				The requested buffer size is not aligned. This code can be returned for a render or a capture device if the caller specified case AUDCLNT_SHAREMODE_EXCLUSIVE and the case AUDCLNT_STREAMFLAGS_EVENTCALLBACK flags. The caller must call Initialize again with the aligned buffer size. For more information, see Remarks."<<std::endl;break;
				case AUDCLNT_E_BUFFER_SIZE_ERROR:std::cout<<"Note  Applies to Windows 7 and later.\
				Indicates that the buffer duration value requested by an exclusive-mode client is out of range. The requested duration value for pull mode must not be greater than 500 milliseconds; for push mode the duration value must not be greater than 2 seconds."<<std::endl;break;
				case AUDCLNT_E_CPUUSAGE_EXCEEDED:std::cout<<"Indicates that the process-pass duration exceeded the maximum CPU usage. The audio engine keeps track of CPU usage by maintaining the number of times the process-pass duration exceeds the maximum CPU usage. The maximum CPU usage is calculated as a percent of the engine's periodicity. The percentage value is the system's CPU throttle value (within the range of 10% and 90%). If this value is not found, then the default value of 40% is used to calculate the maximum CPU usage."<<std::endl;break;
				case AUDCLNT_E_DEVICE_INVALIDATED:std::cout<<"The audio endpoint device has been unplugged, or the audio hardware or associated hardware resources have been reconfigured, disabled, removed, or otherwise made unavailable for use."<<std::endl;break;
				case AUDCLNT_E_DEVICE_IN_USE:std::cout<<"The endpoint device is already in use. Either the device is being used in exclusive mode, or the device is being used in shared mode and the caller asked to use the device in exclusive mode."<<std::endl;break;
				case AUDCLNT_E_ENDPOINT_CREATE_FAILED:std::cout<<"The method failed to create the audio endpoint for the render or the capture device. This can occur if the audio endpoint device has been unplugged, or the audio hardware or associated hardware resources have been reconfigured, disabled, removed, or otherwise made unavailable for use."<<std::endl;break;
				case AUDCLNT_E_INVALID_DEVICE_PERIOD:std::cout<<"Note  Applies to Windows 7 and later.\
				Indicates that the device period requested by an exclusive-mode client is greater than 500 milliseconds."<<std::endl;break;
				case AUDCLNT_E_UNSUPPORTED_FORMAT:std::cout<<"The audio engine (shared mode) or audio endpoint device (exclusive mode) does not support the specified format."<<std::endl;break;
				case AUDCLNT_E_EXCLUSIVE_MODE_NOT_ALLOWED:std::cout<<"The caller is requesting exclusive-mode use of the endpoint device, but the user has disabled exclusive-mode use of the device."<<std::endl;break;
				case AUDCLNT_E_BUFDURATION_PERIOD_NOT_EQUAL:std::cout<<"The AUDCLNT_STREAMFLAGS_EVENTCALLBACK flag is set but parameters hnsBufferDuration and hnsPeriodicity are not equal."<<std::endl;break;
				case AUDCLNT_E_SERVICE_NOT_RUNNING:std::cout<<"The Windows audio service is not running."<<std::endl;break;
				case E_POINTER:std::cout<<"Parameter pFormat is NULL."<<std::endl;break;
				case E_INVALIDARG:std::cout<<"\
				Parameter pFormat points to an invalid format description; or the AUDCLNT_STREAMFLAGS_LOOPBACK flag is set but ShareMode is not equal to AUDCLNT_SHAREMODE_SHARED; or the case AUDCLNT_STREAMFLAGS_CROSSPROCESS flag is set but ShareMode is equal to case AUDCLNT_SHAREMODE_EXCLUSIVE.\
				A prior call to SetClientProperties was made with an invalid category for audio/render streams."<<std::endl;break;
				case E_OUTOFMEMORY:std::cout<<"Out of memory."<<std::endl;break;
                default :
                    //unknown
                    break;
                }



                pAudioClient->Release();
                pAudioClient = nullptr;
            }

        }
        if(pnearestformat!=nullptr)
            CoTaskMemFree(pnearestformat);

        mDeviceModeIsExclusive = mode == AUDCLNT_SHAREMODE_EXCLUSIVE;
		mInitialized = true;
        return mInitialized;
    }
    unsigned audio_device::buffer_size(){
        if(!is_initialized())
            return 0;
        unsigned buffersize = 0;
        HRESULT hr = pAudioClient->GetBufferSize(&buffersize);
        if(hr!=S_OK)
        {

			switch(hr)
			{
				case AUDCLNT_E_NOT_INITIALIZED:std::cout<<"The audio stream has not been successfully initialized."<<std::endl;break;
				case AUDCLNT_E_DEVICE_INVALIDATED:std::cout<<"The audio endpoint device has been unplugged, or the audio hardware or associated hardware resources have been reconfigured, disabled, removed, or otherwise made unavailable for use."<<std::endl;break;
				case AUDCLNT_E_SERVICE_NOT_RUNNING:std::cout<<"The Windows audio service is not running."<<std::endl;break;
				case E_POINTER:std::cout<<"Parameter pNumBufferFrames is NULL."<<std::endl;break;
			}

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
        HRESULT hr=S_OK;
        const IID IID_IAudioClient = __uuidof(IAudioClient);



        // Create an event handle and register it for
    // buffer-event notifications.
        hEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (hEvent == nullptr)
        {
            hr = E_FAIL;
        }
        hr = pAudioClient->SetEventHandle(hEvent);
        if(hr!=S_OK){
            switch(hr){
            case E_INVALIDARG ://Parameter eventHandle is NULL or an invalid handle.
            case AUDCLNT_E_EVENTHANDLE_NOT_EXPECTED :std::cout<<"The audio stream was not initialized for event-driven buffering."<<std::endl;break;
            case AUDCLNT_E_NOT_INITIALIZED : std::cout<<"The audio stream has not been successfully initialized. call initialize first"<<std::endl;break;
            case AUDCLNT_E_DEVICE_INVALIDATED : std::cout<<"The audio endpoint device has been unplugged, or the audio hardware or associated hardware resources have been reconfigured, disabled, removed, or otherwise made unavailable for use."<<std::endl;break;
            case AUDCLNT_E_SERVICE_NOT_RUNNING : std::cout<<"The Windows audio service is not running."<<std::endl;break;
            }
        }

        return true;
    }
    void audio_device::start(){


        if(!is_initialized())
            return;




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
	hr=pAudioClient->GetBufferSize(&bufferFrameCount);
        if(hr!=S_OK)
        {

			switch(hr)
			{
				case AUDCLNT_E_NOT_INITIALIZED:std::cout<<"The audio stream has not been successfully initialized."<<std::endl;break;
				case AUDCLNT_E_DEVICE_INVALIDATED:std::cout<<"The audio endpoint device has been unplugged, or the audio hardware or associated hardware resources have been reconfigured, disabled, removed, or otherwise made unavailable for use."<<std::endl;break;
				case AUDCLNT_E_SERVICE_NOT_RUNNING:std::cout<<"The Windows audio service is not running."<<std::endl;break;
				case E_POINTER:std::cout<<"Parameter pNumBufferFrames is NULL."<<std::endl;break;
			}

        }
    

    DWORD flags = 0;    
    
    
    
    
    // Ask MMCSS to temporarily boost the thread priority
    // to reduce glitches while the low-latency stream plays.
    //the name proaudio must match one of this
    //HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Multimedia\SystemProfile\Tasks.
    DWORD taskIndex = 0;
    HANDLE WINAPI hTask = AvSetMmThreadCharacteristics(TEXT("Pro Audio"), &taskIndex);
    if (hTask == NULL)
    {
        hr = E_FAIL;
    }
    
    BOOL test=AvSetMmThreadPriority (hTask, AVRT_PRIORITY_CRITICAL);


    hr = pAudioClient->Start();
    if(hr!=S_OK)
    {
        int toto=0;
        switch(hr){
            case AUDCLNT_E_NOT_INITIALIZED : std::cout<<"The audio stream has not been successfully initialized."<<std::endl;break;
            case AUDCLNT_E_NOT_STOPPED  : std::cout<<"The audio stream was not stopped at the time of the Start call."<<std::endl;break;
            case AUDCLNT_E_EVENTHANDLE_NOT_SET : std::cout<<"The audio stream is configured to use event-driven buffering, but the caller has not called IAudioClient::SetEventHandle to set the event handle on the stream."<<std::endl;break;
            case AUDCLNT_E_DEVICE_INVALIDATED : std::cout<<"he audio endpoint device has been unplugged, or the audio hardware or associated hardware resources have been reconfigured, disabled, removed, or otherwise made unavailable for use."<<std::endl;break; 
            case AUDCLNT_E_SERVICE_NOT_RUNNING :std::cout<<"The Windows audio service is not running."<<std::endl;break;  
        }
            
    }
    mRunProcess.exchange(true);
    while (flags != AUDCLNT_BUFFERFLAGS_SILENT && mRunProcess)
    {
        UINT32 numFramesPadding=0;
        if(!mDeviceModeIsExclusive)
        {
            hr = pAudioClient->GetCurrentPadding(&numFramesPadding);
            if(hr!=S_OK)
            {
                switch(hr)
                {
                    case AUDCLNT_E_NOT_INITIALIZED : std::cout<<"The audio stream has not been successfully initialized."<<std::endl;break;
                    case AUDCLNT_E_DEVICE_INVALIDATED :std::cout<<"The audio endpoint device has been unplugged, or the audio hardware or associated hardware resources have been reconfigured, disabled, removed, or otherwise made unavailable for use."<<std::endl;break;
                    case AUDCLNT_E_SERVICE_NOT_RUNNING :std::cout<<"The Windows audio service is not running."<<std::endl;break;
                    case E_POINTER :std::cout<<"param is null"<<std::endl;break;
                    default : std::cout<<"unknown"<<std::endl;break;
                }
            }
        }
        UINT32 numFramesAvailable = bufferFrameCount - numFramesPadding;

        BYTE *pData=nullptr;

    // Wait for next buffer event to be signaled.
        DWORD retval = WaitForSingleObject(hEvent, 2000);//max timeout  
        if (retval != WAIT_OBJECT_0)
        {
            // Event handle timed out after a 2-second wait.
            pAudioClient->Stop();break;
        }
        int size = buffer_size();
        // Grab the next empty buffer from the audio device.
        hr = pRenderClient->GetBuffer(numFramesAvailable, &pData);
        if(hr!=S_OK)
        {
            windows_helper::getLastErrorMessage();
            
        }
        // Load the buffer with data from the audio source.
        //hr = pMySource->LoadData(bufferFrameCount, pData, &flags);
        audio_buffer buffer(this->mFormat,pData, numFramesAvailable);
        mCallback(buffer);

        hr = pRenderClient->ReleaseBuffer(numFramesAvailable, flags);
        if(hr!=S_OK)
        {
            switch(hr)
            {
                
            case AUDCLNT_E_INVALID_SIZE : std::cout<<"The NumFramesWritten value exceeds the NumFramesRequested value specified in the previous IAudioRenderClient::GetBuffer call."<<std::endl;break;
            case AUDCLNT_E_BUFFER_SIZE_ERROR : std::cout<<"The stream is exclusive mode and uses event-driven buffering, but the client attempted to release a packet that was not the size of the buffer."<<std::endl;break;
            case AUDCLNT_E_OUT_OF_ORDER : std::cout<<"This call was not preceded by a corresponding call to IAudioRenderClient::GetBuffer."<<std::endl;break;
            case AUDCLNT_E_DEVICE_INVALIDATED : std::cout<<"The audio endpoint device has been unplugged, or the audio hardware or associated hardware resources have been reconfigured, disabled, removed, or otherwise made unavailable for use."<<std::endl;break;
            case AUDCLNT_E_SERVICE_NOT_RUNNING : std::cout<<"The Windows audio service is not running."<<std::endl;break;
            case E_INVALIDARG : std::cout<<"Parameter dwFlags is not a valid value."<<std::endl;break;
            }
            
        }
    }


    hr = pAudioClient->Stop();
    test=AvRevertMmThreadCharacteristics(hTask);

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
    std::wstring audio_device::name(){
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

