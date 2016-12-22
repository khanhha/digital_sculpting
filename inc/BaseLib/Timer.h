#pragma once
/**
 * Timer class definition file.
 *
 * Provides basic timer functionality which calculates running time for certain
 * part of your program. Very useful for analyzing algorithm running time.
 * After a Timer object is created, calling the Start() member function starts 
 * the timer running. The Stop() member function stops the timer. Once the 
 * timer is stopped, Start() must be called again to reset the timer. 
 * GetElapsedSeconds() returns the number of seconds elapsed. Calling 
 * GetElapsedSeconds() while the timer is still running gives you the elapsed
 * time up to that point.
 *
 * Note: 
 * This Timer class implementation is platform independent. The accuracy of the 
 * timer might differ based on the platform/hardware. Both *NIX/Windows version 
 * provides high resolution up to one microseconds. For other platforms the timer 
 * from standard library is used instead. It is not as precise, but is 
 * supported across all platforms.
 */
#ifndef TIMER_H
#define TIMER_H

#if defined(linux) || defined(__linux) || defined(__linux__) || defined(__unix__) || defined(unix) || defined(__unix)
    #define __PLATFORM_UNIX__
    #include <sys/time.h>
    #define stopwatch timeval
#elif defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
    #define __PLATFORM_WINDOWS__
    
	#ifndef NOMINMAX
		#define NOMINMAX // required to stop windows.h messing up std::min
	#endif 

    #define WIN32_LEAN_AND_MEAN
    #include <algorithm>
    #include <windows.h>
    #define stopwatch LARGE_INTEGER
#else
    #define __PLATFORM_OTHER__
    #include <ctime>
    #define stopwatch clock_t
#endif


class Timer
{
public:
    Timer();
    void Start();
    void Stop();
    double GetElapsedSeconds();

	// Qt style functions
	void start() { Start(); } // Start timer
	void stop() { Stop(); } // End timer
	double elapsed() { return GetElapsedSeconds(); } // Get elapsed time in seconds

private:
    stopwatch begin;
    stopwatch end;
    bool isRunning;
    
#ifdef __PLATFORM_WINDOWS__
    DWORD mStartTick;
	LONGLONG mLastTime;
    LARGE_INTEGER mFrequency;
    DWORD mTimerMask;
#endif
    
    void SetBeginTime();
    void SetEndTime();
};

#endif