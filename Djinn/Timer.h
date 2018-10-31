#pragma once

#include <string>

class Timer
{
public:
    static Timer *GetTimer();
private:
    static Timer s_timer;
public:
    float TotalTime()const;
    float DeltaTime()const;
    void Reset();
    void Start();
    void Stop();
    void Tick();
    void UpdateFrameStats();
    std::wstring FrameStatus() const;
    float Fps() const;
    float Mspf() const;
private:
    Timer();
    double secondsPerCount;
    double deltaTime;
    __int64 baseTime;
    __int64 stopTime;
    __int64 pausedTime;
    __int64 prevTime;
    __int64 currTime;
    float fps;
    float mspf;
    bool stopped;
};
