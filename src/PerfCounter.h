#pragma once
#include <stdint.h>
#include <cassert>



#ifdef _WIN32
#include <Windows.h>
#include <WinBase.h>
#else
#include <AvailabilityMacros.h>
#include <CoreAudio/CoreAudioTypes.h>
#include <CoreAudioTypes.h>
#endif

class PerfCounter{
public:
    PerfCounter():mStartTime(0){Init();}
    typedef uint64_t HighResolutionCount;
    typedef double HighResolutionTime;
    //! Reset the start counter
    HighResolutionCount Start(){
        mStartTime = GetHighResolutionTimeCounter();
        return mStartTime;
    }
    //! Get the current time in sec since start
    HighResolutionTime GetCurrentTime(){
        return GetHighResolutionTimeSec(GetCurrentCount());
    }
    //! Get the current cpu tick since start
    HighResolutionCount GetCurrentCount(){
        return GetHighResolutionTimeCounter() - mStartTime;
    }
private:
    HighResolutionCount mStartTime;

#if _WIN32
    HighResolutionCount GetHighResolutionTimeCounter(){
        LARGE_INTEGER t;
        BOOL test = QueryPerformanceCounter(&t);
        assert(test);
        return t.QuadPart;
    }
    HighResolutionTime GetHighResolutionTimeSec(HighResolutionCount time){
        auto dtime = static_cast<double>(time);
        return dtime/mEstimatedFrequency;
    }
    void Init(){
        LARGE_INTEGER f;
        BOOL test = QueryPerformanceFrequency(&f);
        assert(test);
        mEstimatedFrequency = static_cast<double>(f.QuadPart);
    }
#else
    HighResolutionCount GetHighResolutionTimeCounter(){
        return AudioGetCurrentHostTime();
    }
    HighResolutionTime GetHighResolutionTimeSec(HighResolutionCount time){
        auto dtime = static_cast<double>(time);
        return dtime/mEstimatedFrequency;
    }

    void Init(){
        mEstimatedFrequency= AudioGetHostClockFrequency()
    }
#endif
    HighResolutionTime mEstimatedFrequency;
};
