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
    double _deltaTime;
    __int64 _baseTime;
    __int64 _stopTime;
    __int64 _pausedTime;
    __int64 _prevTime;
    __int64 _currTime;
    float fps;
    float mspf;
    bool stopped;
};
