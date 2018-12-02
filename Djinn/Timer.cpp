//***************************************************************************************
// GameTimer.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include <windows.h>
#include "Timer.h"

Timer *Timer::GetTimer()
{
    return &s_timer;
}

Timer Timer::s_timer;

Timer::Timer() :
    secondsPerCount(0.0),
    _deltaTime(-1.0),
    _baseTime(0),
    _stopTime(0),
    _pausedTime(0),
    _prevTime(0),
    _currTime(0),
    stopped(false)
{
    __int64 countsPerSec;
    QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&countsPerSec));
    secondsPerCount = 1.0 / static_cast<double>(countsPerSec);
}

// Returns the total time elapsed since Reset() was called, NOT counting any
// time when the clock is stopped.
float Timer::TotalTime()const
{
    // If we are stopped, do not count the time that has passed since we stopped.
    // Moreover, if we previously already had a pause, the distance 
    // stopTime - baseTime includes paused time, which we do not want to count.
    // To correct this, we can subtract the paused time from stopTime:  
    //
    //                     |<--paused time-->|
    // ----*---------------*-----------------*------------*------------*------> time
    //  baseTime       stopTime        startTime     stopTime    currTime

    if (stopped) {
        return static_cast<float>(((_stopTime - _pausedTime) - _baseTime) * secondsPerCount);
    }

    // The distance currTime - baseTime includes paused time,
    // which we do not want to count.  To correct this, we can subtract 
    // the paused time from currTime:  
    //
    //  (currTime - pausedTime) - baseTime 
    //
    //                     |<--paused time-->|
    // ----*---------------*-----------------*------------*------> time
    //  baseTime       stopTime        startTime     currTime

    else {
        return static_cast<float>(((_currTime - _pausedTime) - _baseTime) * secondsPerCount);
    }
}

float Timer::DeltaTime()const
{
    return static_cast<float>(_deltaTime);
}

void Timer::Reset()
{
    __int64 currTime;
    QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&currTime));

    _baseTime = currTime;
    _prevTime = currTime;
    _stopTime = 0;
    stopped = false;
}

void Timer::Start()
{
    __int64 startTime;
    QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&startTime));


    // Accumulate the time elapsed between stop and start pairs.
    //
    //                     |<-------d------->|
    // ----*---------------*-----------------*------------> time
    //  baseTime       stopTime        startTime     

    if (stopped) {
        _pausedTime += (startTime - _stopTime);

        _prevTime = startTime;
        _stopTime = 0;
        stopped = false;
    }
}

void Timer::Stop()
{
    if (stopped) return;

    __int64 currTime;
    QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&currTime));

    _stopTime = currTime;
    stopped = true;
}

void Timer::Tick()
{
    if (stopped) {
        _deltaTime = 0.0;
        return;
    }

    QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&_currTime));

    // Time difference between this frame and the previous.
    _deltaTime = (_currTime - _prevTime)*secondsPerCount;

    // Prepare for next frame.
    _prevTime = _currTime;

    // Force nonnegative.  The DXSDK's CDXUTTimer mentions that if the 
    // processor goes into a power save mode or we get shuffled to another
    // processor, then deltaTime can be negative.
    if (_deltaTime < 0.0) {
        _deltaTime = 0.0;
    }
}


//*****************************************************************************
// Modification of Timer class by Aaron Hull.
// Original UpdateFrameStats function still attributed to Frank Luna.
//*****************************************************************************


void Timer::UpdateFrameStats()
{
    static int frameCount = 0;
    static float timeElapsed = 0.0f;

    frameCount++;

    if (TotalTime() - timeElapsed >= 1.0f) {
        fps = static_cast<float>(frameCount);
        mspf = 1000.0f / fps;

        frameCount = 0;
        timeElapsed += 1.0f;
    }
}


std::wstring Timer::FrameStatus()const
{
    return std::to_wstring(mspf) + L"ms (" + std::to_wstring(fps) + L"fps)";
}


float Timer::Mspf()const
{
    return mspf;
}

float Timer::Fps()const
{
    return fps;
}
