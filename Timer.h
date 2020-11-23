//
// Created by jon on 11/22/20.
//

#ifndef SUPERMARIOBROS_TIMER_H
#define SUPERMARIOBROS_TIMER_H

#include <cstddef>
#include <functional>
#include <vector>

class ScheduledEvent
{
public:
    size_t mTime;
    std::function<void()> mCallback;

    ScheduledEvent(size_t time, std::function<void()> callback);
};

class Timer
{
public:
    void scheduleSeconds(size_t numSeconds, std::function<void()> callback);
    void incrementNumFrames();

    size_t numFrames;
    std::vector<ScheduledEvent> scheduledTimes;

    const size_t FRAMES_PER_SECOND = 30;
};

Timer& getTimer();


#endif  // SUPERMARIOBROS_TIMER_H
