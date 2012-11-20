#pragma once

#ifdef _WIN32    
#include <windows.h>
#include <strsafe.h>
#include <mmdeviceapi.h>
#include <Functiondiscoverykeys_devpkey.h>
#else
#error // not supported
#endif


#include <iostream>
#include <vector>


namespace windows_helper{
#define WIN_SAFE_RELEASE(punk){ \
        if((punk)==nullptr)(punk)->Release();(punk) = nullptr; }

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
    }
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


//depends on the API we use, for now only one API windows : MMDEVICE
class audio_system{
public:
    audio_system(){scan();}
    int count()const{return mDevicesInfo.size();}
    const audio_device_info& getDeviceInfoAt(int i) const{return mDevicesInfo.at(i);}

private:
    
    //see http://msdn.microsoft.com/en-us/library/windows/desktop/ms680582%28v=vs.85%29.aspx
    void scan(){
        mInitialized = false;
        mDevicesInfo.clear();
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
            return ;
        }

        EDataFlow audio_direction = eAll;//or eCapture, or eRender, or eAll
        DWORD device_state_mask = DEVICE_STATE_ACTIVE; // or DEVICE_STATE_ACTIVE or DEVICE_STATE_DISABLED or DEVICE_STATE_NOTPRESENT or DEVICE_STATE_UNPLUGGED or everything DEVICE_STATEMASK_ALL.
        IMMDeviceCollection * pDeviceCollection=nullptr;
        hr = pEnumerator->EnumAudioEndpoints(audio_direction,device_state_mask,&pDeviceCollection);
        if(hr!=ERROR_SUCCESS || pDeviceCollection==nullptr){
            return ;
        }

        unsigned count;
        hr=pDeviceCollection->GetCount(&count);
        if(hr!=ERROR_SUCCESS){
            return ;
        }

        for(unsigned i=0;i<count;i++){
            IMMDevice * pDevice=nullptr;
            // Get pointer to endpoint number i.
            hr = pDeviceCollection->Item(i, &pDevice);
            if(hr!=ERROR_SUCCESS || pDevice == nullptr){
                break ;
            }
                    // Get the endpoint ID string.
            LPWSTR  device_id=nullptr;
            hr = pDevice->GetId(&device_id);
            if(hr!=ERROR_SUCCESS || device_id == nullptr){
                break ;
            }
            IPropertyStore *pProps=nullptr;
            hr = pDevice->OpenPropertyStore(
                              STGM_READ, &pProps);
            if(hr!=ERROR_SUCCESS || pProps == nullptr){
                break ;
            }

            PROPVARIANT varName;
            // Initialize container for property value.
            PropVariantInit(&varName);

            // Get the endpoint's friendly-name property.
            hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
            if(hr!=ERROR_SUCCESS){
                break ;
            }


            audio_device_info info(std::wstring(varName.pwszVal));
            mDevicesInfo.push_back(info);

            CoTaskMemFree(device_id);
            PropVariantClear(&varName);
            WIN_SAFE_RELEASE(pProps)
            WIN_SAFE_RELEASE(pDevice);
        }


        //clean
        WIN_SAFE_RELEASE( pDeviceCollection);
        WIN_SAFE_RELEASE( pEnumerator);
        mInitialized = true;
    }

    bool mInitialized;
    std::vector<audio_device_info> mDevicesInfo;

};