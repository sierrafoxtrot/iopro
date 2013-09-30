/*
  Timer.cpp - Library for timers
  Copyright 2013, Scott Finneran.
*/

#include "Arduino.h"
#include "Timer.h"

Timer::Timer(unsigned long duration)
{
    timerSet(duration);
}

void Timer::timerSet(unsigned long duration)
{
    interval = duration;
    start = millis();
}

void Timer::timerReset()
{
    start += interval;
}

void Timer::timerRestart()
{
    start = millis();
}

bool Timer::timerExpired()
{
    bool result;

    /* Needs to be updated to handle wrap-around. */
    result = ((millis() - start) >= interval);

    return result;
}
