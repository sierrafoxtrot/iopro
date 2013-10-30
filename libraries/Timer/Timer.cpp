//
// Timer library for Arduino
// Copyright (C) 2013 Scott Finneran
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

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
