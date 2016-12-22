//--------------------------------------------------------------------------
// 
//  Copyright (c) Microsoft Corporation.  All rights reserved. 
// 
//  File: barrier.h
//
//  Implements a ConcRT aware barrier.
//
//  NOTE: Using barrier in Concrt constructs such as parallel_for should be done only
//  after careful consideration of the required semantics. Parallel_for could chunk 
//  work and as a result the barrier wouldn't know the number of tasks that are
//  actually executing in parallel and could waste cycles spinning resulting in
//  worse performance (when compared to openMP where the number of threads in a 
//  thread team is known).
//
//--------------------------------------------------------------------------

#pragma once

#include <concrt.h>
#include <functional>

namespace Concurrency
{
namespace samples
{

/// <summary>
/// A wrapper around cooperative event to enable spinwait.
/// </summary>
class EventWrapper
{
public:

    EventWrapper() : m_blockedState(None)
    {
    }

    void set()
    {
        if (_InterlockedExchange(&m_blockedState, Unblocked) == Blocked)
        {
            m_event.set();
        }
    }

    void wait(int spinCount = 8192)
    {
        // Attempt to spin wait first
        spinWait(spinCount);

        if ((m_blockedState != Unblocked) && (_InterlockedCompareExchange(&m_blockedState, Blocked, None) != Unblocked))
        {
            m_event.wait(COOPERATIVE_TIMEOUT_INFINITE);
        }
    }

    void reset()
    {
        m_event.reset();
        m_blockedState = None;
    }

    void spinWait(int spinCount)
    {
        // Empirical evidence indicate that it is almost always beneficial to spin
        // wait than block.

        const int repeat = 128;
        int retries = spinCount/repeat;

        while ((m_blockedState != Unblocked) && (--retries > 0))
        {
            // Let other tasks run. When we are oversubscribed
            // this would yield to other virtual processors once
            // all the runnables are exhausted. Note that using
            // SwitchToThread() here would be counter productive
            // on UMS as it will only switch tasks within the 
            // particular virtual processor on the scheduler.

            Context::Yield();

            int spin = 0;

            while ((m_blockedState != Unblocked) && (++spin < repeat))
            {
                _YieldProcessor();
            }
        }

        return;
    }

private:

    // Enumeration of blocked states
    enum
    {
        None,
        Blocked,
        Unblocked
    };

    // blocking state
    volatile long m_blockedState;

    // Underlying cooperative event
    Concurrency::event m_event;
};

/// <summary>
/// A ConcRT aware barrrier
/// </summary>
class barrier
{
public:

    /// <summary>
    /// Signature of the callback method when a barrier is reached.
    /// </summary>
    typedef std::tr1::function<void (const size_t)> barrier_method;

    /// <summary>
    /// Constructs a new barrier.
    /// </summary>
    /// <param name="count">Indicates the number of tasks.</param>
    barrier(size_t count) 
        : m_numTasks(count), m_numWaiters(0), m_version(0), m_eventIndex(0), m_pBarrierMethod(NULL)
    {
    }

    /// <summary>
    /// Constructs a new barrier.
    /// </summary>
    /// <param name="count">Indicates the number of tasks.</param>
    /// <param name="barrierMethod"> Callback method to be invoked on reaching a barrier </param>
    barrier(size_t count, barrier_method const& barrierMethod)
        : m_numTasks(count), m_numWaiters(0), m_version(0), m_eventIndex(0), m_pBarrierMethod(NULL)
    {
        if (barrierMethod != NULL)
        {
            m_pBarrierMethod = new barrier_method(barrierMethod);
        }
    }

    void wait()
    {
        // Capture the current event before incrementing the number of 
        // waiters as the current event could be switched out as soon as the barrier
        // is satisfied
        EventWrapper * pEvent = &m_events[m_eventIndex];

        size_t numWaiters = _InterlockedIncrement(&m_numWaiters);

        if (numWaiters == m_numTasks)
        {
            // Update the barrier version number
            ++m_version;

            // Invoke the callback
            if (m_pBarrierMethod != NULL)
            {
                (*m_pBarrierMethod)(m_version);
            }

            // prev = (curr + 1) % 2
            int previousIndex = (m_eventIndex + 1) & 0x1;
            EventWrapper * previousEvent = &m_events[previousIndex];

            // Make previous the event for the next barrier version
            previousEvent->reset();
            m_eventIndex = previousIndex;
            m_numWaiters = 0;

            // Signal the event for the current barrier version
            pEvent->set();
        }
        else
        {
            // Heuristic to determine the amount of spin. Currently, it only accounts
            // for the cases where the number of tasks is greater than the number of 
            // hardware threads

            size_t remainingTasks = m_numTasks - numWaiters;
            int spinCount = (remainingTasks >= CurrentScheduler::Get()->GetNumberOfVirtualProcessors()) ? 0 : 8192;

            pEvent->wait(spinCount);
        }
    }

private:

    // We use 2 events to synchronize the barrier across
    // multiple iterations
    EventWrapper m_events[2];

    // Index of the event for the current barrier version
    int m_eventIndex;

    // Number of tasks that have reached and are waiting on the barrier
    volatile long m_numWaiters;

    // The number of tasks participating in the barrier
    size_t m_numTasks;

    // Barrier version
    size_t m_version;

    // Barrier callback method to be invoked on reaching a barrier
    barrier_method * m_pBarrierMethod;
};

} // namespace samples
} // namespace Concurrency