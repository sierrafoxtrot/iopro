/*
  Timer.h - Library for timers
  Copyright 2013, Scott Finneran.
*/
#ifndef Timer_h
#define Timer_h

class Timer
{
public:
    Timer(unsigned long duration);

    /**
     * Set a timer.
     *
     * This function is used to set a timer for a time at some point in the
     * future. The function timer_expired() will evaluate to true after
     * the timer has expired.
     *
     * \param interval The interval before the timer expires.
     *
     */
    void timerSet(unsigned long duration);

    /**
     * Reset the timer with the same interval.
     *
     * This function resets the timer with the same interval that was
     * given to the timerSet() function. The start point of the interval
     * is the exact time that the timer last expired. Therefore, this
     * function will cause the timer to be stable over time, unlike the
     * timerRestart() function.
     *
     * \param t A pointer to the timer.
     *
     * \sa timerRestart()
     */
    void timerReset();

    /**
     * Restart the timer from the current point in time.
     *
     * This function restarts a timer with the same interval that was
     * given to the timerSet() function. The timer will start at the
     * current time.
     *
     * \note A periodic timer will drift if this function is used to reset
     * it. For periodic timers, use the timerReset() function instead.
     *
     * \sa timerReset()
     */
    void timerRestart();

    /**
     * Check if a timer has expired.
     *
     * This function tests if a timer has expired and returns true or
     * false depending on its status.
     *
     * \return Non-zero if the timer has expired, zero otherwise.
     *
     */
    bool timerExpired();

private:

    /** When the timer started. */
    unsigned long start;

    /** Timer duration. */
    unsigned long interval;

    /** Default Constructor. DO NOT USE. */
    Timer();

    /** Copy Constructor. DO NOT USE. */
    Timer(const Timer &);

    /** Assignment Operator. DO NOT USE. */
    Timer &operator=(const Timer &);
};

#endif // Timer_h
