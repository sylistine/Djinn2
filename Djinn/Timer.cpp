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
    deltaTime(-1.0),
    baseTime(0),
    stopTime(0),
    pausedTime(0),
    prevTime(0),
    currTime(0),
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
        return static_cast<float>(((stopTime - pausedTime) - baseTime) * secondsPerCount);
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
        return static_cast<float>(((currTime - pausedTime) - baseTime) * secondsPerCount);
    }
}

float Timer::DeltaTime()const
{
    return static_cast<float>(deltaTime);
}

void Timer::Reset()
{
    __int64 currTime;
    QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&currTime));

    baseTime = currTime;
    prevTime = currTime;
    stopTime = 0;
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
        pausedTime += (startTime - stopTime);

        prevTime = startTime;
        stopTime = 0;
        stopped = false;
    }
}

void Timer::Stop()
{
    if (!stopped) {
        __int64 currTime;
        QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&currTime));

        stopTime = currTime;
        stopped = true;
    }
}

void Timer::Tick()
{
    if (stopped) {
        deltaTime = 0.0;
        return;
    }

    __int64 currTime;
    QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&currTime));
    currTime = currTime;

    // Time difference between this frame and the previous.
    deltaTime = (currTime - prevTime)*secondsPerCount;

    // Prepare for next frame.
    prevTime = currTime;

    // Force nonnegative.  The DXSDK's CDXUTTimer mentions that if the 
    // processor goes into a power save mode or we get shuffled to another
    // processor, then deltaTime can be negative.
    if (deltaTime < 0.0) {
        deltaTime = 0.0;
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
